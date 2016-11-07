#include <iostream>
#include <limits>
#include <vector>
#include "big_integer.h"


big_integer::big_integer() {
    this->data.push_back(0);
    this->sign = true;
}

big_integer::big_integer(big_integer const &other) {
    this->data = other.data;
    this->sign = other.sign;
}

big_integer::big_integer(std::string const &str) {
    this->data.resize(1, 0);
    this->sign = true;
    for (size_t i = ((str.at(0) == '-') ? 1 : 0); i < str.length(); i++) {
        *this = this->mul_long_short(10);
        *this += (str[i] - '0');
    }
    this->sign = (str.at(0) != '-');
}

big_integer::big_integer(int a) {
    sign = a >= 0;
    data.push_back(uint32_t(sign ? a : (-int64_t(a))));
}

big_integer::~big_integer() { }

bool big_integer::is_zero() const {
    return (this->data.size() == 1 && this->data[0] == 0);
}

int big_integer::compare(big_integer const &other) const {
    if (this->is_zero() && other.is_zero()) return 0;
    if (this->sign && !other.sign) return 1;
    if (!this->sign && other.sign) return -1;
    if (this->sign && other.sign) {
        if (this->data.size() > other.data.size()) return 1;
        if (this->data.size() < other.data.size()) return -1;
        for (size_t i = this->data.size(); i > 0; i--) {
            if (this->data[i - 1] > other.data[i - 1]) return 1;
            if (this->data[i - 1] < other.data[i - 1]) return -1;
        }
    }
    else //both negative
    {
        if (this->data.size() > other.data.size()) return -1;
        if (this->data.size() < other.data.size()) return 1;
        for (size_t i = this->data.size(); i > 0; i--) {
            if (this->data[i - 1] > other.data[i - 1]) return -1;
            if (this->data[i - 1] < other.data[i - 1]) return 1;
        }
    }
    return 0;
}

big_integer &big_integer::convert() {
    if (!this->sign) {
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = ~(data[i]);
        }
        this->unsigned_add(1);
    }
    return *this;
}

big_integer &big_integer::unsigned_add(big_integer const &rhs) {
    uint32_t carry = 0;
    for (size_t i = 0; (i < std::max(this->data.size(), rhs.data.size())) || carry; i++) {
        if (i == this->data.size())
            this->data.push_back(0);
        uint64_t tmp = carry + uint64_t(this->data[i]) + (i < rhs.data.size() ? rhs.data[i] : 0);
        carry = uint32_t(tmp >> (BASE_LEN));
        this->data[i] = uint32_t(tmp);
    }
    this->remove_zeroes();
    return *this;
}

big_integer &big_integer::unsigned_sub(big_integer const &rhs) {
    big_integer r(rhs);
    r.sign = true;
    this->sign = true;
    bool sign = true;
    int comp = this->compare(r);
    if (comp == -1) {
        big_integer tmp(r);
        r = *this;
        *this = tmp;
        sign = false;
    }
    int carry = 0;
    this->data.push_back(0);
    for (size_t i = 0; i < r.data.size() || carry; i++) {
        uint32_t cur = this->data[i];
        this->data[i] -= (i < r.data.size() ? r.data[i] : 0) + carry;
        if (cur < this->data[i]) {
            carry = 1;
        } else {
            carry = 0;
        }
    }
    this->sign = sign;
    this->remove_zeroes();
    return *this;
}

big_integer big_integer::div_long_short(uint32_t d) const {
    big_integer res = *this;
    uint32_t carry = 0;
    for (size_t i = res.data.size(); i > 0; i--) {
        uint64_t cur = (uint64_t(uint64_t(carry) << BASE_LEN)) + uint64_t(res.data[i - 1]);
        res.data[i - 1] = uint32_t(cur / uint64_t(d));
        carry = uint32_t(cur % uint64_t(d));
    }
    res.remove_zeroes();
    return res;
}

big_integer big_integer::mul_long_short(uint32_t d) const {
    big_integer res = *this;
    uint32_t carry = 0;
    for (size_t i = 0; (i < res.data.size()) || carry; i++) {
        if (i == res.data.size()) {
            res.data.push_back(0);
        }
        uint64_t cur = uint64_t(res.data[i]) * d;
        cur += carry;
        res.data[i] = uint32_t(cur);
        carry = uint32_t(cur >> (BASE_LEN));
    }
    res.remove_zeroes();
    return res;
}

big_integer &big_integer::operator=(big_integer const &other) {
    this->data = other.data;
    this->sign = other.sign;
    return *this;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (this->sign == 0) {
        if (!rhs.sign)
            this->unsigned_add(-rhs);
        else
            *this = rhs - (-*this);
    }
    else if (this->sign == 1 && rhs.sign == 0)
        this->unsigned_sub(-rhs);
    else
        this->unsigned_add(rhs);
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (this->sign == 0) {
        if (!rhs.sign)
            *this += -rhs;
        else
            this->unsigned_add(rhs);
    }
    else if (this->sign == 1) {
        if (!rhs.sign)
            this->unsigned_add(-rhs);
        else
            this->unsigned_sub(rhs);
    }
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    bool sign = this->sign == rhs.sign;
    this->sign = true;
    big_integer res(this->mul_long_short(rhs.data[0]));
    for (size_t j = 1; j < rhs.data.size(); j++)
        res += this->mul_long_short(rhs.data[j]) << (BASE_LEN * j);
    this->remove_zeroes();
    *this = res;
    this->sign = sign;
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool sign = this->sign == rhs.sign;
    big_integer res;
    big_integer r = rhs;
    this->sign = true;
    r.sign = true;
    int comp = this->compare(rhs);
    if (comp != 1) {
        this->sign = sign && (comp == -1);
        *this = (comp == 0);
        return *this;
    }
    if (rhs.data.size() == 1) {
        *this = this->div_long_short(rhs.data.back());
        this->sign = sign;
        return *this;
    }
    uint32_t normalize = uint32_t((BASE) / uint64_t(r.data.back() + 1));
    *this = this->mul_long_short(normalize);
    r = r.mul_long_short(normalize);
    size_t m = this->data.size() - r.data.size();
    res.data.resize(m + 1);
    if (*this >= r << (BASE_LEN * m)) {
        *this -= r << (BASE_LEN * m);
        res.data.back() = 1;
    }
    else
        res.data.back() = 0;
    for (size_t i = m; i > 0; i--) {
        uint64_t cur = (uint64_t(r.data.size() + i - 1 < this->data.size() ? this->data[r.data.size() + i - 1] : 0) << BASE_LEN)
                       + (r.data.size() + i - 2 < this->data.size() ? this->data[r.data.size() + i - 2] : 0);
        cur = cur / uint64_t(r.data.back());
        res.data[i - 1] = uint32_t(std::min(BASE - 1, cur));
        *this -= r.mul_long_short(res.data[i - 1]) << (BASE_LEN * (i - 1));
        while (*this < 0) {
            res.data[i - 1]--;
            *this += r << (BASE_LEN * (i - 1));
        }
    }
    res.sign = sign;
    res.remove_zeroes();
    *this = res;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    return *this -= ((*this / rhs) * rhs);
}

big_integer &big_integer::bit_operation(big_integer const &rhs, int type) {
    big_integer right(rhs);
    this->convert();
    right.convert();
    for (size_t i = 0, s = std::max(this->data.size(), right.data.size()); i < s; i++) {
        uint32_t l, r;
        l = (i < this->data.size() ? this->data[i] : (this->sign ? 0 : uint32_t(BASE - 1)));
        r = (i < right.data.size() ? right.data[i] : (right.sign ? 0 : uint32_t(BASE - 1)));
        if (i == this->data.size()) this->data.push_back(0);
        switch (type) {
            case 0: this->data[i] = l & r;
                break;
            case 1: this->data[i] = l | r;
                break;
            case 2: this->data[i] = l ^ r;
                break;
            default: std::cerr << "unknown bit operation" << std::endl;
                break;
        }
    }
    switch (type) {
        case 0: this->sign = this->sign || right.sign;
            break;
        case 1: this->sign = (this->sign && right.sign);
            break;
        case 2: this->sign = !(this->sign ^ right.sign);
            break;
        default: std::cerr << "unknown bit operation" << std::endl;
            break;
    }
    this->convert();
    this->remove_zeroes();
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    this->bit_operation(rhs, 0);
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    this->bit_operation(rhs, 1);
    return *this;
}


big_integer &big_integer::operator^=(big_integer const &rhs) {
    this->bit_operation(rhs, 2);
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs == 0 || is_zero()) {
        return *this;
    }
    if (rhs < 0) {
        *this >>= -rhs;
        return *this;
    }
    uint32_t shift = uint32_t(rhs) / BASE_LEN;
    uint32_t bytes = uint32_t(rhs) % BASE_LEN;
    this->data.push_back(0);
    if (bytes != 0) {
        uint32_t mask = ~((1U << (32U - bytes)) - 1U);
        for (size_t i = this->data.size() - 1; i > 0; i--) {
            uint32_t next, cur;
            next = mask;
            next &= this->data[i - 1];
            next >>= (32 - bytes);
            cur = this->data[i - 1];
            cur <<= bytes;
            this->data[i - 1] = cur;
            this->data[i] |= next;
        }
    }
    size_t n = this->data.size();
    this->data.resize(this->data.size() + shift);
    for (int i = n + shift - 1; i >= int(shift); i--) {
        this->data[i] = this->data[i - shift];
    }
    for (size_t i = 0; i < shift; ++i) {
        this->data[i] = 0;
    }
    this->remove_zeroes();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs == 0 || is_zero()) {
        return *this;
    }
    if (rhs < 0) {
        *this >>= -rhs;
        return *this;
    }
    uint32_t shift = uint32_t(rhs) / BASE_LEN;
    uint32_t bytes = uint32_t(rhs) % BASE_LEN;
    uint32_t mask = ~((1U << (32U - bytes)) - 1U);
    this->convert();
    if (bytes != 0) {
        for (size_t i = shift; i < this->data.size(); i++) {
            uint32_t cur, prev;
            cur = this->data[i];
            cur >>= bytes;
            if (i == this->data.size() - 1 && !this->sign) {
                cur |= mask;
            }
            this->data[i] = cur;
            if (i < this->data.size() - 1) {
                prev = (1U << bytes) - 1;
                prev &= this->data[i + 1];
                prev <<= (32 - bytes);
                this->data[i] |= prev;
            }
        }
    }
    for (size_t i = shift; i < this->data.size(); i++) {
        this->data[i - shift] = this->data[i];
    }
    mask = (this->sign ? 0 : uint32_t(BASE - 1));
    for (size_t i = 0; i < shift; i++) {
        this->data[this->data.size() - 1 - i] = mask;
    }
    this->convert();
    this->remove_zeroes();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    r.sign = !r.sign;
    return r;
}

big_integer big_integer::operator~() const {
    big_integer r(*this);
    r += 1;
    r.sign = !r.sign;
    return r;
}

big_integer &big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer &big_integer::operator--() {
    *this -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

bool operator==(big_integer const &a, big_integer const &b) {
    return a.compare(b) == 0;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    return a.compare(b) == -1;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return a.compare(b) == 1;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

std::string to_string(big_integer const &a) {
    std::string s;
    if (a == 0) return "0";
    big_integer t(a);
    t.sign = true;
    while (t.data.size() != 1 || t.data[0] != 0) {
        big_integer r = t % 10;
        s = char(r.data[0] + '0') + s;
        t = t.div_long_short(10);
    }
    s = (!a.sign ? "-" : "") + s;
    return s;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
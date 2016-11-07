#include <assert.h>
#include "optvector.h"

shared_data::shared_data() : value(new std::vector<uint32_t>()) {}

shared_data::shared_data(shared_data const &other) : value(other.value) {}


void shared_data::make_own() {
    if (!this->value.unique())
        this->value = std::make_shared<std::vector<uint32_t>>(*this->value);
}

optvector::optvector() {
    this->contains_small = 1;
}

optvector::optvector(optvector const &other) {
    this->contains_small = other.contains_small;
    if (this->is_small())
        this->small_value = other.small_value;
    else {
        this->data = new shared_data(*other.data);
    }
}


optvector::~optvector() {
    if (!this->is_small())
        delete (data);
}

uint32_t &optvector::back() {
    if (this->is_small())
        return this->small_value;
    else {
        this->data->make_own();
        return this->data->value.get()->back();
    }
}

uint32_t optvector::back() const {
    if (this->is_small())
        return this->small_value;
    else
        return this->data->value.get()->back();
}

size_t optvector::size() const {
    if (this->is_small())
        return (this->small_size() ? 1 : 0);
    else
        return this->data->value.get()->size();
}

uint32_t optvector::operator[](size_t i) const {
    assert(i < size());
    if (this->is_small())
        return this->small_value;
    else
        return this->data->value.get()->at(i);
}

uint32_t &optvector::operator[](size_t i) {
    assert(i < size());
    if (this->is_small()) {
        this->contains_small = 3;
        return this->small_value;
    } else {
        this->data->make_own();
        return this->data->value.get()->at(i);
    }
}

void optvector::swap(optvector &b) {
    std::swap(this->data, b.data);
    std::swap(this->contains_small, b.contains_small);
}

optvector &optvector::operator=(optvector const &other) {
    optvector tmp(other);
    this->swap(tmp);
    return *this;
}

void optvector::resize(size_t i) {
    if (this->is_small()) {
        if (i != 1) {
            data = new shared_data;
            data->value.get()->resize(i);
            this->contains_small = 0;
        } else if (this->small_size()) {
            this->contains_small = 1;
        }
    } else {
        this->data->make_own();
        this->data->value.get()->resize(i);
    }
}

void optvector::resize(size_t i, uint32_t val) {
    if (this->is_small()) {
        if (i != 1) {
            data = new shared_data;
            data->value.get()->resize(i, val);
            this->contains_small = 0;
        } else if (!this->small_size()) {
            this->small_value = val;
            this->contains_small = 3;
        }
    } else {
        if (i == 1) {
            delete (this->data);
            this->small_value = val;
            this->contains_small = 3;
        } else {
            this->data->make_own();
            this->data->value.get()->resize(i, val);
        }
    }
}

uint32_t optvector::pop_back() {
    if (this->data->value.get()->size() > 1) {
        if (this->data->value.get()->size() == 2) {
            uint32_t tmp = this->data->value.get()->at(0);
            uint32_t res = this->data->value.get()->at(1);
            delete (this->data);
            this->small_value = tmp;
            this->contains_small = 3;
            return res;
        } else {
            this->data->make_own();
            uint32_t res = this->data->value.get()->back();
            this->data->value.get()->pop_back();
            return res;
        }
    } else {
        uint32_t res = small_value;
        small_value = 0;
        contains_small = 1;
        return res;
    }
}

void optvector::push_back(uint32_t val) {
    if (this->is_small()) {
        if (!this->small_size()) {
            this->small_value = val;
            this->contains_small = 3;
        } else {
            uint32_t tmp = this->small_value;
            data = new shared_data;
            data->value.get()->push_back(tmp);
            data->value.get()->push_back(val);
            this->contains_small = 0;
        }
    } else {
        this->data->make_own();
        this->data->value.get()->push_back(val);
    }
}
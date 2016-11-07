#ifndef BIGINT_OPTVECTOR_H
#define BIGINT_OPTVECTOR_H

#include <vector>
#include <cstddef>
#include <memory>

struct shared_data {
    std::shared_ptr<std::vector<uint32_t>> value;

    shared_data();

    shared_data(shared_data const &other);

    void make_own();
};

struct optvector {
    optvector();

    ~optvector();

    optvector(optvector const &other);

    uint32_t &back();

    uint32_t back() const;

    void resize(size_t i);

    void resize(size_t i, uint32_t val);

    size_t size() const;

    uint32_t &operator[](size_t i);

    uint32_t operator[](size_t i) const;

    optvector &operator=(optvector const &other);

    uint32_t pop_back();

    void push_back(uint32_t val);

    void swap(optvector &b);

private:
    union {
        uint32_t small_value;
        shared_data *data;
    };
    char contains_small; //helps to see whether we have small_value or not

    bool is_small() const {
        return bool(this->contains_small & 1);
    }

    bool small_size() const {
        return bool((this->contains_small & 2) >> 1);
    }
};

#endif //BIGINT_OPTVECTOR_H
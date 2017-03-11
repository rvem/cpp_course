//
// Created by roman on 10.03.17.
//
#ifndef ANY_H
#define ANY_H

#include <typeinfo>
#include <cstdio>

struct any {

    any() : state(EMPTY), storage_wrapper(nullptr) {}

    any(const any &other) : state(other.state), storage_wrapper(other.storage_wrapper) {
        if (!other.empty()) {
            other.storage_wrapper->copy(other.storage, storage);
        }
    }

    any(any &&other) : state(other.state), storage_wrapper(other.storage_wrapper) {
        if (!other.empty()) {
            other.storage_wrapper->move(other.storage, storage);
            other.storage_wrapper = nullptr;
        }
    }

    any &operator=(const any &other) {
        any(other).swap(*this);
        return *this;
    }

    any &operator=(any &&other) {
        any(std::move(other)).swap(*this);
        return *this;
    }

    template<typename ValueType>
    any(ValueType &&value) {
        construct(std::forward<ValueType>(value));
    }

    template<typename ValueType>
    any &operator=(ValueType &&value) {
        any(std::forward<ValueType>(value)).swap(*this);
        return *this;
    }

    ~any() {
        clear();
    }

    void swap(any &other) {
        if (storage_wrapper != other.storage_wrapper) {
            any tmp(std::move(other));
            other.storage_wrapper = storage_wrapper;
            other.state = state;
            if (state != EMPTY) {
                storage_wrapper->move(storage, other.storage);
            }
            storage_wrapper = tmp.storage_wrapper;
            state = tmp.state;
            if (tmp.storage_wrapper != nullptr) {
                tmp.storage_wrapper->move(tmp.storage, storage);
                tmp.storage_wrapper = nullptr;
                tmp.state = EMPTY;
            }
        } else {
            if (state != EMPTY) {
                storage_wrapper->swap(storage, other.storage);
            }
        }
    }

    bool empty() const {
        return state == EMPTY;
    }

    const std::type_info &type() const {
        return empty() ? typeid(void) : storage_wrapper->type();
    }

    template<typename T>
    T *cast() {
        switch (state) {
            case EMPTY:
                return nullptr;
            case SMALL:
                return reinterpret_cast<T *>(&storage.stack);
            case BIG:
                return reinterpret_cast<T *>(storage.dynamic);
        }
        return nullptr;
    }

    template<typename T>
    const T *cast() const {
        switch (state) {
            case EMPTY:
                return nullptr;
            case SMALL:
                return reinterpret_cast<const T *>(&storage.stack);
            case BIG:
                return reinterpret_cast<const T *>(storage.dynamic);
        }
        return nullptr;
    }

    enum storage_state {
        EMPTY,
        SMALL,
        BIG
    };


    void clear() {
        if (!empty()) {
            storage_wrapper->destroy(storage);
            storage_wrapper = nullptr;
            state = EMPTY;
        }
    }

    storage_state state;

    static const size_t SMALL_SIZE = 16;

    union storage_union {
        void *dynamic;
        std::aligned_storage<SMALL_SIZE, SMALL_SIZE>::type stack;
    };

    storage_union storage;

    struct storage_t {

        const std::type_info &(*type)();

        void (*destroy)(storage_union &);

        void (*copy)(const storage_union &src, storage_union &dest);

        void (*move)(storage_union &src, storage_union &dest);

        void (*swap)(storage_union &lhs, storage_union &rhs);
    };

    storage_t *storage_wrapper;

    template<typename T>
    struct dynamic_storage {
        dynamic_storage() {}

        static const std::type_info &type() {
            return typeid(T);
        }

        static void destroy(storage_union &storage) {
            delete reinterpret_cast<T *>(storage.dynamic);
        }

        static void copy(const storage_union &src, storage_union &dest) {
            dest.dynamic = new T(*reinterpret_cast<const T *>(src.dynamic));
        }

        static void move(storage_union &src, storage_union &dest) {
            dest.dynamic = src.dynamic;
            src.dynamic = nullptr;
        }

        static void swap(storage_union &lhs, storage_union &rhs) {
            std::swap(lhs.dynamic, rhs.dynamic);
        }
    };

    template<typename T>
    struct static_storage {
        static_storage() {}

        static const std::type_info &type() {
            return typeid(T);
        }

        static void destroy(storage_union &storage) {
            reinterpret_cast<T *>(&storage.stack)->~T();
        }

        static void copy(const storage_union &src, storage_union &dest) {
            new(&dest.stack) T(reinterpret_cast<const T &>(src.stack));
        }

        static void move(storage_union &src, storage_union &dest) {
            new(&dest.stack) T(std::move(reinterpret_cast<T &>(src.stack)));
            destroy(src);
        }

        static void swap(storage_union &lhs, storage_union &rhs) {
            std::swap(reinterpret_cast<T &>(lhs.stack), reinterpret_cast<T &>(rhs.stack));
        }
    };

    template<typename ValueType>
    struct is_small {
        const static bool value =
                (std::is_nothrow_copy_constructible<ValueType>::value) && (sizeof(ValueType) <= SMALL_SIZE);
    };

    template<typename ValueType>
    typename std::enable_if<!is_small<ValueType>::value>::type construct(ValueType &&value) {
        using T = typename std::decay<ValueType>::type;
        static storage_t wrapper = {
                dynamic_storage<T>::type,
                dynamic_storage<T>::destroy,
                dynamic_storage<T>::copy,
                dynamic_storage<T>::move,
                dynamic_storage<T>::swap
        };
        storage_wrapper = &wrapper;
        storage.dynamic = new T(std::forward<ValueType>(value));
        state = BIG;
    }

    template<typename ValueType>
    typename std::enable_if<is_small<ValueType>::value>::type construct(ValueType &&value) {
        using T = typename std::decay<ValueType>::type;
        static storage_t wrapper = {
                static_storage<T>::type,
                static_storage<T>::destroy,
                static_storage<T>::copy,
                static_storage<T>::move,
                static_storage<T>::swap
        };
        storage_wrapper = &wrapper;
        new(&storage.dynamic) T(std::forward<ValueType>(value));
        state = SMALL;
    }
};

template<typename T>
const T *any_cast(const any *operand) {
    if (operand == nullptr || operand->type() != typeid(T))
        throw "invalid type cast";
    else
        return operand->cast<T>();
}

template<typename T>
T *any_cast(any *operand) {
    if (operand == nullptr || operand->type() != typeid(T))
        throw "invalid type cast";
    else
        return operand->cast<T>();
}


#endif

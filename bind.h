//
// Created by roman on 27.12.16.
//


#ifndef BIND_BIND_H
#define BIND_BIND_H

#include <tuple>

template<size_t N>
struct place_holder {
};

place_holder<0> _1;
place_holder<1> _2;
place_holder<2> _3;
place_holder<3> _4;
place_holder<4> _5;

template<typename F, typename... Args>
struct binder {
    binder(F &&func, Args &&...args) : func(std::forward<F>(func)), arguments(std::forward<Args>(args)...) {}

    //simple get
    template<typename T, typename... Get_args>
    auto &&get(T &&curr, Get_args &... args) const {
        return curr;
    }

    //placeholder get
    template<size_t N, typename... Get_args>
    auto &&get(place_holder<N>, Get_args &... args) const {
        return std::get<N>(std::forward_as_tuple(args...));
    }

    //another bind get
    template<typename Func, typename ... Func_args, typename ... Get_args>
    auto get(const binder<Func, Func_args ...> &f, Get_args &... args) const {
        return f(args ...);
    }

    //sequence structure
    template<size_t ... N>
    struct sequence {
    };

    template<size_t First, size_t... Other>
    struct sequence_builder {
        typedef typename sequence_builder<First - 1, First - 1, Other...>::type type;
    };

    template<size_t ... Other>
    struct sequence_builder<0, Other...> {
        typedef sequence<Other...> type;
    };

    template<typename... Operator_args>
    auto operator()(Operator_args &&...args) const {
        return call(typename sequence_builder<std::tuple_size<tuple>::value>::type(),
                    std::forward<Operator_args>(args) ...);
    }

    template<typename... Call_args, size_t... N>
    auto call(const sequence<N ...> &, Call_args &&... args) const {
        return func(get(std::get<N>(arguments), args ...) ...);
    }

    typedef std::tuple<Args...> tuple;
    F func;
    tuple arguments;
};

template<typename F, typename ... Args>
binder<F, Args...> bind(F &&f, Args &&... args) {
    return binder<F, Args...>(std::forward<F>(f), std::forward<Args>(args)...);
};

#endif //BIND_BIND_H

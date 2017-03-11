#include <typeinfo>
#include <cstdio>
#include <memory>
#include <iostream>
#include "any.h"

struct x {
    int a, b, c, d, e;
    x(int a, int b, int c, int d, int e) : a(a), b(b), c(c), d(d), e(e) {}
};


int main() {
    x xx(1, 2, 3, 4, 5);
    any a(5);
    any b(xx);

    try {
        int *p = any_cast<int>(&a); //valid cast
    } catch (char const * e) {
        std::cerr << e << std::endl;
    }
    try {
        x* xxx= any_cast<x>(&b); //valid cast
    } catch (char const * e) {
        std::cerr << e << std::endl;
    }
    try {
        int* t = any_cast<int>(&b); //invalid cast
    } catch (char const * e) {
        std::cerr << e << std::endl;
    }

    const any d(xx);
    any t = d; //operator=(const any &)
    any p = any(xx); //operator= (any&&)
    a.swap(b); //different size swap


    any q(5);
    any w(6);
    q.swap(w); //same size swap


    return 0;
}
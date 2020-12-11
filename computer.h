#include <cstdint>
#include <array>
#include <cassert>
#include <iostream>

constexpr unsigned int Id(const char *input) {
    unsigned int base = 42;
    unsigned int x = 0;

    assert(input[0] != '\0');
    unsigned int base2 = 1;
    for (std::size_t i = 0; input[i] != '\0'; i++) {
        if (i == 6)
            assert(false);
        assert(('0' <= input[i] && input[i] <= '9') ||
               ('A' <= input[i] && input[i] <= 'Z') ||
               ('a' <= input[i] && input[i] <= 'z'));
        int integer = 0;

        if ('0' <= input[i] && input[i] <= '9') //[0-9]
            integer = input[i] - '0';
        if ('A' <= input[i] && input[i] <= 'Z') //[A-Z]
            integer = input[i] - 'A';
        if ('a' <= input[i] && input[i] <= 'z') //[a-z]
            integer = input[i] - 'a';

        x += base2 * integer;
        base2 *= base;
    }

    const unsigned int result = x;
    return result;
}

template<uint64_t N> //Tu pewnie trzeba cos zmienic
struct Num{
};

template <typename Dst, typename Src>
struct Mov{
};

template <unsigned int Expr>
struct Jmp {
};

template <unsigned int Expr>
struct Jz {
};

template <unsigned int Expr>
struct Js {
};

template <unsigned int Expr>
struct Label{
};

template<unsigned int Expr>
struct Lea {
};

template<typename Left, typename Right>
struct Add {
};

template<typename Left, typename Right>
struct Sub{
};

template<unsigned int Left, typename Right>
struct D{
};

template<typename Expr>
struct Inc {
};

template<typename Expr>
struct Dec {
};

template<typename Expr>
struct Mem {
};

template<typename Expr, typename... RestExpr>
struct Program {
};

template<int size, class T> //Tu pewnie trzeba cos zmienic
class Computer {
public:
    template<typename Expr>
    static constexpr std::array<T, size> boot() {
        const std::array<T, size> res;
        return res;
    }
private:
    template<uint64_t N> //Tu pewnie trzeba cos zmienic
    struct Num{
        constexpr static uint64_t val = N;
    };
};
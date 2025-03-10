#include "computer.h"
#include <array>
#include <iostream>

// Operator == dla std::array jest constexpr dopiero od C++20.
template<class T, std::size_t N>
constexpr bool compare(std::array<T, N> const& arg1, std::array<T, N> const& arg2) {
    for (size_t i = 0; i < N; ++i)
        if (arg1[i] != arg2[i]) return false;
    return true;
}

/*
 * Tests by Wojciech Przytuła
 * All rights left
 * */

using test_machine = Computer<4, int>;

// the following tests presume that you use unsigned long long for storing IDs


// tests that should compile and not raise static_assert errors

using test_empty = Program<>;
constexpr std::array<int, 4> test_empty_res = {0, 0, 0, 0};
static_assert(compare(test_machine::boot<test_empty>(), test_empty_res));

using test_jumps = Program<
        Dec<Mem<Num<0>>>,
        Jz<Id("XD")>,
        Inc<Mem<Num<0>>>,
        Jz<Id("XD")>,
        Inc<Mem<Num<1>>>,
        Label<Id("xd")>
>;
//constexpr std::array<int, 4> test_jumps_res = {0, 0, 0, 0};
//static_assert(compare(test_machine::boot<test_jumps>(), test_jumps_res));

using test_declarations = Program<
        D<Id("a"), Num<-3>>,
        D<Id("b"), Num<-2>>,
        D<Id("c"), Num<-1>>,
        D<Id("a"), Num<-1>>
>;
//constexpr std::array<int, 4> test_declarations_res = {-3, -2, -1, -1};
//static_assert(compare(test_machine::boot<test_declarations>(), test_declarations_res));


// tests that should compile AND ofc not result in infinite loop:
using test_id1 = Program<
        D<Id("a"), Num<1>>,
        Label<Id("a")>,
        Dec<Mem<Num<0>>>,
        Js<Id("end")>,
        Label<Id("A")>,
        Jz<Id("A")>,
        Label<Id("loop")>,
        Jmp<Id("loop")>,
        Label<Id("end")>
        >;

using test_underflow = Program<
        D<Id("mem0"), Num<INT32_MIN>>,
        Dec<Mem<Lea<Id("MEM0")>>>>;
//constexpr auto test_underflow_res = test_machine::boot<test_underflow>();

using test_finite_loop = Program<
        Label<Id("loop")>,
        Inc<Mem<Lea<Id("MEM0")>>>,
        Js<Id("LOOP")>,
        D<Id("mem0"), Num<-100>>
>;
 auto test_finite_loop_res = test_machine::boot<test_finite_loop>();


// tests that should not compile (i.e. template parsing error):

//using test_bad_id1 = Program<
//        Label<Id("%")>>;

//using test_bad_id2 = Program<
//        Label<Id("3324234")>>;
//
//using test_bad_syntax1 = Program<
//        Inc<Lea<Lea<Id("a")>>>>;


/* tests that should not run as constexpr,
 * i.e. constexpr res = machine::boot<test>() should result in compile-time error.
 * (if they don't even compile then it's good either way) */

using test_infinite_loop = Program<
        Label<Id("loop")>,
        Mov<Mem<Lea<Id("MEM0")>>, Num<0>>,
        Jmp<Id("LOOP")>,
        D<Id("mem0"), Num<-1000000>>
>;
//constexpr auto test_infinite_loop_res = test_machine::boot<test_infinite_loop>();

using test_bad_program = Num<4>;
//constexpr auto bad_program = test_machine::boot<test_bad_program>();

using test_size1 = Program<
        Mov<Mem<Num<2137>>, Num<1>>>;
//constexpr auto test_size1_res = test_machine::boot<test_size1>();

using test_syntax1 = Program<
        Num<0>>;
//constexpr auto test_syntax1_res = test_machine::boot<test_syntax1>();

using test_syntax2 = Program<
        Lea<Id("a")>>;
//constexpr auto test_syntax2_res = test_machine::boot<test_syntax2>();

using test_syntax3 = Program<
        Mem<Num<0>>>;
//constexpr auto test_syntax3_res = test_machine::boot<test_syntax3>();

using test_syntax4 = Program<
        Inc<Num<0>>>;
//constexpr auto test_syntax4_res = test_machine::boot<test_syntax4>();

using test_syntax5 = Program<
        Inc<Inc<Num<0>>>>;
//constexpr auto test_syntax5_res = test_machine::boot<test_syntax5>();

// I can't make this test fail, yet I believe it should.
using test_syntax6 = Program<
        Label<4ULL>>;
//constexpr auto test_syntax6_res = test_machine::boot<test_syntax6>();

using test_D_syntax1 = Program<
        D<Id("a"), Mem<Num<1>>>>;
//constexpr auto test_D_syntax1_res = test_machine::boot<test_D_syntax1>();

using test_D_syntax2 = Program<
        D<Id("1"), Num<1>>,
        D<Id("a"), Lea<Id("1")>>>;
//constexpr auto test_D_syntax2_res = test_machine::boot<test_D_syntax1>();



int main() {
//    test_machine::boot<test_finite_loop>();
//    test_machine::boot<test_id1>();
//
//    auto arr = test_machine::boot<test_id1>();
//    for (auto itr : arr)
//        std::cout << (int)itr << std::endl;
}

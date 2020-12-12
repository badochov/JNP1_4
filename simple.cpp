#include "computer.h"

int main() {
    using b = Program<Label<Id("42")>, D<Id("xd"), Num<42>>, D<Id("xd2"), Num<43>>>;


    constexpr auto a = Computer<2, uint8_t>::boot<b>();

    using e = Program<Inc<Mem<Mem<Num<10>>>>>;

    using f = Program<Inc<Mem<Num<10>>>>;

    using tmpasm_operations = Program<
            D<Id("a"), Num<4>>,
            D<Id("b"), Num<3>>,
            D<Id("c"), Num<2>>,
            D<Id("d"), Num<1>>,
            //Add<Mem<Lea<Id("a")>>, Mem<Lea<Id("c")>>>,
            //Sub<Mem<Lea<Id("b")>>, Mem<Lea<Id("d")>>>,
            Mov<Mem<Lea<Id("c")>>, Num<0>>,
            Mov<Mem<Lea<Id("d")>>, Num<6>>>;

    for (auto x : Computer<5, int64_t>::boot<tmpasm_operations>())
        std::cout << x << "\n";

    constexpr auto ce = Computer<11, int>::boot<e>();
    constexpr auto cf = Computer<11, int>::boot<f>();

    using c = Program<
            Mov<Mem<Mem<Num<10>>>, Num<'h'>>,
            Inc<Mem<Num<10>>>>;

    constexpr auto d = Computer<11, int>::boot<c>();

//    for (auto x : d) {
//        std::cout << (int) x << "\n";
//    }
//    static_assert(a.vars[0] == Id("xd"), "Nope");
//    static_assert(a.vars[1] == Id("xd2"), "Nope");
}
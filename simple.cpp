#include "computer.h"

int main() {
    using test_finite_loop = Program<
        Label<Id("loop")>,
        Inc<Mem<Lea<Id("MEM0")>>>,
        Js<Id("LOOP")>,
        D<Id("mem0"), Num<-100>>
    >;
    constexpr auto test_finite_loop_res = Computer<4,int64_t>::boot<test_finite_loop>();


        using test_machine = Computer<4, int>;



//        using too_many = Program<D<Id("a"), Num<42>>, D<Id("b"), Num<42>>, D<Id("c"), Num<42>>, D<Id("d"), Num<42>>, D<Id("e"), Num<42>>>;
//        test_machine::boot<too_many>();



//        using too_many_same_name = Program<D<Id("a"), Num<42>>, D<Id("a"), Num<42>>, D<Id("a"), Num<42>>, D<Id("a"), Num<42>>, D<Id("a"), Num<42>>>;
//        test_machine::boot<too_many_same_name>();

}
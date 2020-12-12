#include "computer.h"

int main() {
    using test_finite_loop = Program<
        Label<Id("loop")>,
        Inc<Mem<Lea<Id("MEM0")>>>,
        Js<Id("LOOP")>,
        D<Id("mem0"), Num<-100>>
    >;
    constexpr auto test_finite_loop_res = Computer<4,int64_t>::boot<test_finite_loop>();
}
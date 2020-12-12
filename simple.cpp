#include "computer.h"

int main() {
    using b = Program<
        Label<Id("loop")>,
        Inc<Mem<Lea<Id("MEM0")>>>,
        Js<Id("LOOP")>,
        D<Id("mem0"), Num<-100>>>;


     auto d = Computer<11, int>::boot<b>();

}
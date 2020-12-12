#include "computer.h"

int main() {
    using b = Program<D<Id("xd"), Num<42>>, D<Id("xd2"), Num<43>>>;

    constexpr auto a = Computer<2, uint8_t>::boot<b>();

    static_assert(a[0] == Id("xd"), "Nope" );
    static_assert(a[1] == Id("xd2"), "Nope" );
}
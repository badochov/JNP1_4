#ifndef JNP1_4_COMPUTER_H
#define JNP1_4_COMPUTER_H

#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include <array>
#include <cstdint>
#include <string>
struct Instruction {};
template <typename... Instructions>
struct Program {};

template <typename D, typename S>
struct Mov : Instruction {};

template <typename A>
struct Mem {};

struct Id{
    constexpr Id(const char* s) : id(s){};
    const char*id;
};

struct NumericInstruction;

template <Id id, NumericInstruction nr>
struct D {};

template <typename D, typename S>
struct Add : Instruction {};

template <typename D>
using Inc = Add<D,Num<1>>;

template <size_t size, class Word, std::enable_if_t<std::is_integral<Word>::value, bool> = true>
class Computer {
    using memory_t = std::array<Word, size>;
    using instruction_set_t = std::vector<Instruction>;
    template <typename P>
    struct ASMProgram;
    template <typename... Instructions>
    struct ASMProgram<Program<Instructions...>> {
        constexpr static auto evaluate(memory_t &mem) {
            static_assert(((std::is_base_of
                              <Instruction,Instructions>::value) && ... && true),
                          "Error: a program should contain instructions only!");
            Evaluator<Instructions...>::evaluate(mem);
            return mem;
        }
    };

    template <typename... Instructions>
    struct Evaluator {
        constexpr static void evaluate(memory_t& mem){}
    };
    template <typename D, typename... Rest>
    struct Evaluator<I,Rest...> {
        constexpr static void evaluate(memory_t& mem){}
    };
  public:
    template <typename P>
    constexpr static auto boot() {
        memory_t memory{};
        return ASMProgram<P>::evaluate(memory);
        //Wywolanie zmieniajace zawartosc pamieci
    }
};


#endif // JNP1_4_COMPUTER_H

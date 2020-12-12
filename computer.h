#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

constexpr unsigned int Id(const char *input) {
    unsigned int base = 42;
    unsigned int x = 0;

    assert(input[0] != '\0');
    unsigned int base2 = 1;
    for (std::size_t i = 0; input[i] != '\0'; i++) {
        if (i == 6)
            assert(false);
        assert(('0' <= input[i] && input[i] <= '9') || ('A' <= input[i] && input[i] <= 'Z') ||
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

struct PValue {};

struct LValue {};

struct Instruction {};

template <uint64_t N> // Tu pewnie trzeba cos zmienic
struct Num {
    static const uint64_t value = N;
};

template <typename Dst, typename Src>
struct Mov {};

template <unsigned int Expr>
struct Jmp {};

template <unsigned int Expr>
struct Jz {};

template <unsigned int Expr>
struct Js {};

template <unsigned int Expr>
struct Label {};

template <unsigned int Expr>
struct Lea {};

template <typename Left, typename Right>
struct Add {};

template <typename Left, typename Right>
struct Sub {};

template <unsigned int Left, typename Right>
struct D : Instruction {};

template <typename Expr>
struct Inc {};

template <typename Expr>
struct Dec {};

template <typename Expr>
struct Mem {};

template <typename Expr, typename... RestExpr>
struct Program {};

template <typename... Ts>
struct dependent_false : std::false_type {};

template <std::size_t size, typename T> // Tu pewnie trzeba cos zmienic
class Computer {
    using memory_t = std::array<T, size>;

    using vars_memory_t = std::array<unsigned int, size>;

    struct Variables {
        constexpr static int add(unsigned int id, vars_memory_t &vars) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == 0) {
                    vars[i] = id;
                    return i;
                }
            }
            return -1;
        }

        constexpr int idx(unsigned int id, vars_memory_t &vars) {
            for (typename vars_memory_t::size_t i = 0; i < size; i++) {
                if (vars[i] == id) {
                    return i;
                }
            }
            assert(false);
        }
    };

    template <typename P>
    struct ASBProgram {
        constexpr static auto evaluate(memory_t &mem, vars_memory_t &vars) {
        }
    };
    template <typename... Instructions>
    struct ASBProgram<Program<Instructions...>> {
        using instructions_memory_t = std::array<Instruction, sizeof...(Instructions)>;
        constexpr static auto evaluate(memory_t &mem, vars_memory_t &vars) {
            instructions_memory_t instructions_memory{};
            static_assert(((std::is_base_of<Instruction, Instructions>::value) && ...),
                          "Error: program should consist only of instructions!");
            Declarations<Instructions...>::declare_variables(mem, vars);
//            EvalHelper<0, mem, vars, Instructions...>::res;

            return vars;
        }
    };

    // Structs responsible for initial declarations.
    template <typename... Instructions>
    struct Declarations {
        constexpr static void declare_variables(memory_t &mem, vars_memory_t &vars) {
            static_assert(sizeof...(Instructions) == 0,
                          "Program should contain only instructions!");
        }
    };

    template <typename Ins, typename... Instructions>
    struct Declarations<Ins, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, vars_memory_t &vars) {
            static_assert(std::is_base_of<Instruction, Ins>::value,
                          "Program should contain only instructions!");
            Declarations<Instructions...>::declare_variables(mem, vars);
        }
    };

    template <typename... Instructions>
    struct Declarations<Instruction, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, vars_memory_t &vars) {
            Declarations<Instructions...>::declare_variables(mem, vars);
        }
    };

    template <unsigned int id, typename Val, typename... Rest>
    struct Declarations<D<id, Val>, Rest...> {
        constexpr static void declare_variables(memory_t &mem, vars_memory_t &vars) {
            int idx = Variables::add(id, vars);
            mem[idx] = Val::value;

            Declarations<Rest...>::declare_variables(mem, vars);
        }
    };

    template <unsigned int id, typename... Instructions>
    struct Evaluator {
        constexpr static unsigned int evaluate(memory_t &mem, vars_memory_t &vars) {
            return 0;
        }
    };

  public:
    template <typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        vars_memory_t vars{0};
        return ASBProgram<P>::evaluate(memory, vars);
    }
};

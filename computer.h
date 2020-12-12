#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

using id_t = unsigned int;

constexpr id_t Id(const char *input) {
    id_t base = 42;
    id_t x = 0;

    assert(input[0] != '\0');
    id_t base2 = 1;
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

    const id_t result = x;
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

template <id_t Expr>
struct Jmp {};

template <id_t Expr>
struct Jz {};

template <id_t Expr>
struct Js {};

template <id_t Expr>
struct Label : Instruction {};

template <id_t Expr>
struct Lea {};

template <typename Left, typename Right>
struct Add {};

template <typename Left, typename Right>
struct Sub {};

template <id_t Left, typename Right>
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

    using vars_memory_t = std::array<id_t, size>;

    using asb_program_memory_t = struct {
        using flag_t = bool;
        vars_memory_t vars{0};
        id_t sough_label = 0;
        flag_t ZF = false;
        flag_t SF = false;
    };

    struct Variables {
        constexpr static int add(id_t id, vars_memory_t &vars) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == 0) {
                    vars[i] = id;
                    return i;
                }
            }
            return -1;
        }

        constexpr int idx(id_t id, vars_memory_t &vars) {
            for (typename vars_memory_t::size_t i = 0; i < size; i++) {
                if (vars[i] == id) {
                    return i;
                }
            }
            return -1;
        }
    };

    template <typename P>
    struct ASBProgram {
        constexpr static auto evaluate(memory_t &mem, vars_memory_t &vars) {
        }
    };
    template <typename... Instructions>
    struct ASBProgram<Program<Instructions...>> {
        constexpr static auto evaluate(memory_t &mem) {
            asb_program_memory_t asb_program_memory{};
            Declarations<Instructions...>::declare_variables(mem, asb_program_memory);
            id_t id = Evaluator<Instructions...>::evaluate(mem, asb_program_memory);
            assert(id == 0);
            return asb_program_memory;
        }
    };

    // Structs responsible for initial declarations.
    template <typename... Instructions>
    struct Declarations {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
        }
    };

    template <typename Ins, typename... Instructions>
    struct Declarations<Ins, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(std::is_base_of<Instruction, Ins>::value,
                          "Program should contain only instructions!");
            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    template <id_t id, typename Val, typename... Rest>
    struct Declarations<D<id, Val>, Rest...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            int idx = Variables::add(id, program_mem.vars);
            mem[idx] = Val::value;

            Declarations<Rest...>::declare_variables(mem, program_mem);
        }
    };

    template <typename... Instructions>
    struct Evaluator {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            ;
            return program_mem.sough_label;
        }
    };

    template <id_t id, typename... Instructions>
    struct Evaluator<Label<id>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            int sought_id = program_mem.sough_label;
            do {
                if (sought_id == id) {
                    program_mem.sough_label = 0;
                }
                sought_id = Evaluator<Instructions...>::evaluate(mem, program_mem);
            } while (sought_id == id);

            return sought_id;
        }
    };

    template <T id, typename... Instructions>
    struct Evaluator<Num<id>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label != 0) {
                return Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
            // Perform Action
        }
    };

  public:
    template <typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        return ASBProgram<P>::evaluate(memory);
    }
};

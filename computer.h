#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

using id_t = unsigned int;

constexpr id_t Id(const char *input) {
    id_t base = 42;
    id_t x = 1;

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
            integer = input[i] - 'A' + 10;
        if ('a' <= input[i] && input[i] <= 'z') //[a-z]
            integer = input[i] - 'a' + 10;

        x += base2 * integer;
        base2 *= base;
    }

    const id_t result = x;
    return result;
}

struct Numeric {};

struct PValue {};

struct LValue {};

struct Instruction {};

template <auto N> // Tu pewnie trzeba cos zmienic
struct Num : Numeric, PValue {};

template <id_t Expr>
struct Lea : Numeric, PValue {};

template <typename Expr>
struct Mem : Numeric, PValue, LValue {};

template <typename Dst, typename Src>
struct Mov : Instruction {};

template <id_t Expr>
struct Jmp : Instruction {};

template <id_t Expr>
struct Jz : Instruction {};

template <id_t Expr>
struct Js : Instruction {};

template <id_t Expr>
struct Label : Instruction {};

template <typename Left, typename Right>
struct Add : Instruction {};

template <typename Left, typename Right>
struct Cmp : Instruction {};

template <typename Left, typename Right>
struct And : Instruction {};

template <typename Left, typename Right>
struct Or : Instruction {};

template <typename Left>
struct Not : Instruction {};

template <typename Left, typename Right>
struct Sub : Instruction {};

template <id_t Left, typename Right>
struct D : Instruction {};

template <typename Expr>
struct Inc : Instruction {};

template <typename Expr>
struct Dec : Instruction {};

template <typename... Instrucions>
struct Program {};

template <typename... Ts>
struct dependent_false : std::false_type {};

template <std::size_t size, typename T>
class Computer {
    using memory_t = std::array<T, size>;

    using vars_memory_t = std::array<id_t, size>;

    using asb_program_memory_t = struct {
        using flag_t = bool;
        vars_memory_t vars{0};
        id_t sough_label = 0;
        flag_t ZF = false;
        flag_t SF = false;

        constexpr int add(id_t id) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == 0) {
                    vars[i] = id;
                    return i;
                }
            }
            return -1;
        }

        constexpr int idx(id_t id) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == id) {
                    return i;
                }
            }
            return -1;
        }

        constexpr void set_flag_ZF(T lval) {
            ZF = lval == 0;
        }

        constexpr void set_flag_SF(T lval) {
            SF = lval < 0;
        }

        constexpr void set_flags(T lval) {
            set_flag_ZF(lval);
            set_flag_SF(lval);
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
            static_assert(std::is_base_of_v<Instruction, Ins>,
                          "Program should contain only instructions!");
            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    template <id_t id, auto num, typename... Instructions>
    struct Declarations<D<id, Num<num>>, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            int idx = program_mem.add(id);
            mem[idx] = num;

            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    template <id_t id, typename Val, typename... Instructions>
    struct Declarations<D<id, Val>, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(dependent_false<Val>::value, "D's second parameter should be Num");
        }
    };

    template <typename... Instructions>
    struct Evaluator {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            return program_mem.sough_label;
        }
    };

    template <id_t id, typename Val, typename... Instructions>
    struct Evaluator<D<id, Val>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
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

    template <typename L, typename R, typename... Instructions>
    struct Evaluator<Mov<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0)
                LValueEvaluator<L>::get_reference(mem, program_mem) = RValueEvaluator<R>::get_value(mem, program_mem);
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <id_t id, typename... Instructions>
    struct Evaluator<Jmp<id>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                program_mem.sough_label = id;
            }

            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <id_t id, typename... Instructions>
    struct Evaluator<Jz<id>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0 && program_mem.ZF) {
                program_mem.sough_label = id;
            }

            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <id_t id, typename... Instructions>
    struct Evaluator<Js<id>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0 && program_mem.SF) {
                program_mem.sough_label = id;
            }

            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Arithmetic functions
    template <typename L, typename R, typename... Instructions>
    struct Evaluator<Add<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0 ) {
                auto &lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref += RValueEvaluator<R>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename R, typename... Instructions>
    struct Evaluator<Sub<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                auto &lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref -= RValueEvaluator<R>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename... Instructions>
    struct Evaluator<Inc<L>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            return Evaluator<Add<L, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename... Instructions>
    struct Evaluator<Dec<L>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            return Evaluator<Sub<L, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Logic functions
    template <typename L, typename R, typename... Instructions>
    struct Evaluator<Cmp<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                auto lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref -= RValueEvaluator<R>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename R, typename... Instructions>
    struct Evaluator<And<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                auto &lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref &= RValueEvaluator<R>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename R, typename... Instructions>
    struct Evaluator<Or<L, R>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                auto &lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref |= RValueEvaluator<R>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename L, typename... Instructions>
    struct Evaluator<Not<L>, Instructions...> {
        constexpr static id_t evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.sough_label == 0) {
                auto &lref = LValueEvaluator<L>::get_reference(mem, program_mem);
                lref = ~lref;
                program_mem.set_flag_ZF(lref);
            }
            return Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //LValueEvaluator
    template <typename L>
    struct LValueEvaluator {
        constexpr static auto &get_reference(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(dependent_false<L>::value, "WTf");
        }
    };

    template <typename L>
    struct LValueEvaluator<Mem<L>> {
        constexpr static auto &get_reference(memory_t &mem, asb_program_memory_t &program_mem) {
            return mem[RValueEvaluator<L>::get_value(mem, program_mem)];
        }
    };

    //RValueEvaluator
    template <typename R>
    struct RValueEvaluator {};

    template <auto id>
    struct RValueEvaluator<Num<id>> {
        constexpr static auto get_value(memory_t &mem, asb_program_memory_t &program_mem) {
            return id;
        }
    };

    template <auto id>
    struct RValueEvaluator<Lea<id>> {
        constexpr static auto get_value(memory_t &mem, asb_program_memory_t &program_mem) {
            return program_mem.idx(id);
        }
    };

    template <typename R>
    struct RValueEvaluator<Mem<R>> {
        constexpr static auto &get_value(memory_t &mem, asb_program_memory_t &program_mem) {
            //auto id = program_mem.idx();
            return mem[RValueEvaluator<R>::get_value(mem, program_mem)];
        }
    };

  public:
    template <typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        ASBProgram<P>::evaluate(memory);
        return memory;
    }
};

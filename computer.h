#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

using identifier_t = int32_t;

constexpr identifier_t Id(const char *input) {
    identifier_t base = 42;
    identifier_t x = 1;

    assert(input[0] != '\0');
    identifier_t base2 = 1;
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

    const identifier_t result = x;
    return result;
}

struct Instruction {};

template <auto num>
struct Num {};

template <identifier_t id>
struct Lea {};

template <typename PValue>
struct Mem {};

template <typename LValue, typename PValue>
struct Mov : Instruction {};

template <identifier_t label_id>
struct Jmp : Instruction {};

template <identifier_t label_id>
struct Jz : Instruction {};

template <identifier_t label_id>
struct Js : Instruction {};

template <identifier_t label_id>
struct Label : Instruction {};

template <typename LValue, typename RValue>
struct Add : Instruction {};

template <typename LValue, typename RValue>
struct Cmp : Instruction {};

template <typename LValue, typename RValue>
struct And : Instruction {};

template <typename LValue, typename RValue>
struct Or : Instruction {};

template <typename LValue>
struct Not : Instruction {};

template <typename LValue, typename RValue>
struct Sub : Instruction {};

template <identifier_t LValue, typename RValue>
struct D : Instruction {};

template <typename LValue>
struct Inc : Instruction {};

template <typename LValue>
struct Dec : Instruction {};

template <typename... Instrucions>
struct Program {};

template <std::size_t size, typename MemoryWordType>
class Computer {
    using memory_t = std::array<MemoryWordType, size>;

    using vars_memory_t = std::array<identifier_t, size>;

    using asb_program_memory_t = struct ASBProgramMemory{
        using flag_t = bool;
        vars_memory_t vars{0};
        identifier_t sought_label = 0;
        flag_t ZF = false;
        flag_t SF = false;

        constexpr int add(identifier_t id) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == 0) {
                    vars[i] = id;
                    return i;
                }
            }
            return -1;
        }

        constexpr int idx(identifier_t id) {
            for (std::size_t i = 0; i < size; i++) {
                if (vars[i] == id) {
                    return i;
                }
            }
            return -1;
        }

        constexpr void set_flag_ZF(MemoryWordType value) {
            ZF = value == 0;
        }

        constexpr void set_flag_SF(MemoryWordType value) {
            SF = value < 0;
        }

        constexpr void set_flags(MemoryWordType value) {
            set_flag_ZF(value);
            set_flag_SF(value);
        }

        constexpr bool not_searching_for_label() {
            return sought_label == 0;
        }
    };

    template <typename...>
    struct FailHelper : std::false_type {};

    template <typename P>
    struct ASBProgram {};
    template <typename... Instructions>
    struct ASBProgram<Program<Instructions...>> {
        constexpr static auto evaluate(memory_t &mem) {
            asb_program_memory_t asb_program_memory{};
            Declarations<Instructions...>::declare_variables(mem, asb_program_memory);
            auto id = asb_program_memory.sought_label;
            do {
                Evaluator<Instructions...>::evaluate(mem, asb_program_memory);
                id = asb_program_memory.sought_label;
            } while (id > 0);
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

    template <typename PotentialInstruction, typename... Instructions>
    struct Declarations<PotentialInstruction, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(std::is_base_of_v<Instruction, PotentialInstruction>,
                          "Program should contain only instructions!");
            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    template <identifier_t id, auto num, typename... Instructions>
    struct Declarations<D<id, Num<num>>, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            int idx = program_mem.add(id);
            mem[idx] = num;

            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    template <identifier_t id, typename Val, typename... Instructions>
    struct Declarations<D<id, Val>, Instructions...> {
        constexpr static void declare_variables(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(FailHelper<Val>::value, "D's second parameter should be Num");
        }
    };

    template <typename... Instructions>
    struct Evaluator {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (!program_mem.not_searching_for_label()) {
                assert(((void)"Label not found", false));
            }
        }
    };

    template <identifier_t id, typename Val, typename... Instructions>
    struct Evaluator<D<id, Val>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <identifier_t id, typename... Instructions>
    struct Evaluator<Label<id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (id == program_mem.sought_label) {
                program_mem.sought_label = 0;
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Mov<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                LValueEvaluator<LValue>::get_reference(mem, program_mem) =
                    RValueEvaluator<RValue>::get_value(mem, program_mem);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Jmp<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                program_mem.sought_label = label_id;
            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Jz<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label() && program_mem.ZF) {
                program_mem.sought_label = label_id;

            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Js<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label() && program_mem.SF) {
                program_mem.sought_label = label_id;
            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    // Arithmetic functions
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Add<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref += RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Sub<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref -= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename... Instructions>
    struct Evaluator<Inc<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            Evaluator<Add<LValue, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename... Instructions>
    struct Evaluator<Dec<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            Evaluator<Sub<LValue, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    // Logic functions
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Cmp<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref -= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<And<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref &= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Or<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref |= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    template <typename LValue, typename... Instructions>
    struct Evaluator<Not<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem, asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref = ~lref;
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    // LValueEvaluator
    template <typename LValue>
    struct LValueEvaluator {
        constexpr static auto &get_reference(memory_t &mem, asb_program_memory_t &program_mem) {
            static_assert(FailHelper<LValue>::value, "Not a l-value!");
        }
    };

    template <typename RValue>
    struct LValueEvaluator<Mem<RValue>> {
        constexpr static auto &get_reference(memory_t &mem, asb_program_memory_t &program_mem) {
            return mem[RValueEvaluator<RValue>::get_value(mem, program_mem)];
        }
    };

    // RValueEvaluator
    template <typename RValue>
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

    template <typename RValue>
    struct RValueEvaluator<Mem<RValue>> {
        constexpr static auto &get_value(memory_t &mem, asb_program_memory_t &program_mem) {
            auto id = RValueEvaluator<RValue>::get_value(mem, program_mem);
            return mem[id];
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

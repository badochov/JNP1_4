#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>

using identifier_t = int32_t;

//Returns identifier for provided [input]. Big and small letters are undifferentiated.
constexpr identifier_t Id(const char *input) {
    identifier_t base = 42;
    identifier_t x = 1;

    if (input[0] == '\0')
        throw std::invalid_argument("Input should be not empty");
    identifier_t base2 = 1;
    for (std::size_t i = 0; input[i] != '\0'; i++) {
        if (i == 6)
            throw std::invalid_argument("Input's size is over the limit of 6 characters");
        if (!(('0' <= input[i] && input[i] <= '9') ||
              ('A' <= input[i] && input[i] <= 'Z') ||
              ('a' <= input[i] && input[i] <= 'z')))
            throw std::invalid_argument("Input's letters should be letters or numbers");
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

    //Memory of the computer.
    using asb_program_memory_t = struct ASBProgramMemory {
        using flag_t = bool;

        using vars_memory_t = std::array<identifier_t, size>;
        using vars_size_t = typename vars_memory_t::size_type;
        vars_memory_t vars{0};

        identifier_t sought_label = 0;
        flag_t ZF = false;
        flag_t SF = false;
        vars_size_t last_index = 0;

        //Assigns the identifier to one of memory's cells.
        //Throws an error if there are more assigned identifiers than memory's cells.
        constexpr vars_size_t add(identifier_t id) {
            if (last_index == size)
                throw std::invalid_argument("Too many variables");

            vars[last_index] = id;
            return last_index++;
        }

        //Finds which index of the memory identifier is assigned to and returns it.
        //Throws an error if it can't find it.
        constexpr vars_size_t idx(identifier_t id) {
            bool found = false;
            for (vars_size_t i = 0; i < size; i++) {
                if (vars[i] == id) {
                    found = true;
                    return i;
                }
            }
            if (!found)
                throw std::invalid_argument("Variable not found");
            return -1;
        }

        //Sets the flag ZF based on last changed value.
        constexpr void set_flag_ZF(MemoryWordType value) {
            ZF = value == 0;
        }

        //Sets the flag SF based on last changed value.
        constexpr void set_flag_SF(MemoryWordType value) {
            SF = value < 0;
        }

        //Sets up both flags based on last changed value.
        constexpr void set_flags(MemoryWordType value) {
            set_flag_ZF(value);
            set_flag_SF(value);
        }

        //Returns whether the program is currently searching for a label or not.
        constexpr bool not_searching_for_label() {
            return sought_label == 0;
        }
    };

    //Helper to return false at any point in the evaluator.
    template <typename...>
    struct FailHelper : std::false_type {};

    template <typename P>
    struct ASBProgram {};

    //Main backbone of the computer. First declares all variables
    //it can find and then evaluates the program.
    template <typename... Instructions>
    struct ASBProgram<Program<Instructions...>> {
        constexpr static auto evaluate( memory_t &mem) {
            asb_program_memory_t asb_program_memory{};
            Declarations<Instructions...>::declare_variables(mem, asb_program_memory);
            identifier_t id = 0;
            do {
                Evaluator<Instructions...>::evaluate(mem, asb_program_memory);
                id = asb_program_memory.sought_label;
            } while (id > 0);
            if (id != 0)
                throw std::exception();
            return asb_program_memory;
        }
    };

    // Structs responsible for initial declarations.
    template <typename... Instructions>
    struct Declarations {
        constexpr static void declare_variables([[maybe_unused]] memory_t &mem,
                                                [[maybe_unused]] asb_program_memory_t &program_mem) {
        }
    };

    //Checks whether other instructions are proper instructions but skips its evaluation.
    template <typename PotentialInstruction, typename... Instructions>
    struct Declarations<PotentialInstruction, Instructions...> {
        constexpr static void declare_variables(memory_t &mem,
                                                asb_program_memory_t &program_mem) {
            static_assert(std::is_base_of_v<Instruction, PotentialInstruction>,
                          "Program should contain only instructions!");
            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    //Assigns the identifier to first free memory's cell and assigns value [num] to it.
    template <identifier_t id, auto num, typename... Instructions>
    struct Declarations<D<id, Num<num>>, Instructions...> {
        constexpr static void declare_variables(memory_t &mem,
                                                asb_program_memory_t &program_mem) {
            auto idx = program_mem.add(id);
            mem[idx] = num;

            Declarations<Instructions...>::declare_variables(mem, program_mem);
        }
    };

    //Since the second parameter of D is not a Num instruction it throws an error.
    template <identifier_t id, typename Val, typename... Instructions>
    struct Declarations<D<id, Val>, Instructions...> {
        constexpr static void declare_variables([[maybe_unused]] memory_t &mem,
                                                [[maybe_unused]] asb_program_memory_t &program_mem) {
            static_assert(FailHelper<Val>::value, "D's second parameter should be Num");
        }
    };

    //Should only enter here after program's last operation.
    //Checks whether computer was searching for a label.
    //If it was throw an exception.
    template <typename... Instructions>
    struct Evaluator {
        constexpr static void evaluate([[maybe_unused]] memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (!program_mem.not_searching_for_label()) {
                throw std::invalid_argument("Label not found");
            }
        }
    };

    //Skips declarations.
    template <identifier_t id, typename Val, typename... Instructions>
    struct Evaluator<D<id, Val>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //If program tries to jump to the label [id] it should resume evaluation, skip otherwise.
    template <identifier_t id, typename... Instructions>
    struct Evaluator<Label<id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (id == program_mem.sought_label) {
                program_mem.sought_label = 0;
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Unless the program is already during a jump,
    //performs the unconditional jump.
    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Jmp<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                program_mem.sought_label = label_id;
            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    //Unless the program is already during a jump,
    //performs conditional jump based on flag ZF.
    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Jz<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label() && program_mem.ZF) {
                program_mem.sought_label = label_id;

            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    //Unless the program is already during a jump,
    //performs conditional jump based on flag ZS.
    template <identifier_t label_id, typename... Instructions>
    struct Evaluator<Js<label_id>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label() && program_mem.SF) {
                program_mem.sought_label = label_id;
            } else {
                Evaluator<Instructions...>::evaluate(mem, program_mem);
            }
        }
    };

    //Assigns value of RValue to LValue.
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Mov<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                LValueEvaluator<LValue>::get_reference(mem, program_mem) =
                        RValueEvaluator<RValue>::get_value(mem, program_mem);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Arithmetic functions.
    //Adds value of RValue to LValue and sets flags if needed.
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Add<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref += RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Subtracts value of RValue to LValue and sets flags if needed.
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Sub<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref -= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flags(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Increments value LValue by 1 and sets flags if needed.
    template <typename LValue, typename... Instructions>
    struct Evaluator<Inc<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            Evaluator<Add<LValue, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Decrements value RValue by 1 and sets flags if needed.
    template <typename LValue, typename... Instructions>
    struct Evaluator<Dec<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            Evaluator<Sub<LValue, Num<1>>, Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Logic functions.
    //Compares two arguments and sets flags if needed.
    template <typename RValue, typename RValue2, typename... Instructions>
    struct Evaluator<Cmp<RValue, RValue2>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto arg1 = RValueEvaluator<RValue>::get_value(mem, program_mem);
                arg1 -= RValueEvaluator<RValue2>::get_value(mem, program_mem);
                program_mem.set_flags(arg1);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Performs AND logic operator on both parameters and stores its result in first one.
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<And<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref &= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Performs OR logic operator on both parameters and stores its result in first one.
    template <typename LValue, typename RValue, typename... Instructions>
    struct Evaluator<Or<LValue, RValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref |= RValueEvaluator<RValue>::get_value(mem, program_mem);
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Performs NOT logic operator on LValue and stores its result.
    //Sets flag ZF if needed.
    template <typename LValue, typename... Instructions>
    struct Evaluator<Not<LValue>, Instructions...> {
        constexpr static void evaluate(memory_t &mem,
                                       asb_program_memory_t &program_mem) {
            if (program_mem.not_searching_for_label()) {
                auto &lref = LValueEvaluator<LValue>::get_reference(mem, program_mem);
                lref = ~lref;
                program_mem.set_flag_ZF(lref);
            }
            Evaluator<Instructions...>::evaluate(mem, program_mem);
        }
    };

    //Evaluates l-values of the program. If the l-value is not a valid l-value it throws an error.
    template <typename LValue>
    struct LValueEvaluator {
        constexpr static auto &get_reference([[maybe_unused]] memory_t &mem,
                                             [[maybe_unused]] asb_program_memory_t &program_mem) {
            static_assert(FailHelper<LValue>::value, "Not a l-value!");
        }
    };

    //Returns the reference to what RValue is pointing to.
    template <typename RValue>
    struct LValueEvaluator<Mem<RValue>> {
        constexpr static auto &get_reference(memory_t &mem,
                                             asb_program_memory_t &program_mem) {
            return mem[RValueEvaluator<RValue>::get_value(mem, program_mem)];
        }
    };

    //Evaluates r-values of the program. If the r-value is not a valid r-value it throws an error.
    template <typename RValue>
    struct RValueEvaluator {
        constexpr static auto &get_reference([[maybe_unused]] memory_t &mem,
                                             [[maybe_unused]] asb_program_memory_t &program_mem) {
            static_assert(FailHelper<RValue>::value, "Not a l-value!");
        }
    };

    //Returns its id.
    template <auto id>
        struct RValueEvaluator<Num<id>> {
            constexpr static auto get_value([[maybe_unused]] memory_t &mem,
                                            [[maybe_unused]] asb_program_memory_t &program_mem) {
                return id;
            }
    };

    //Returns index of memory to which identifier id is assigned.
    template <auto id>
    struct RValueEvaluator<Lea<id>> {
        constexpr static auto get_value([[maybe_unused]] memory_t &mem,
                                        asb_program_memory_t &program_mem) {
            return program_mem.idx(id);
        }
    };

    //Returns reference to memory of index of RValue.
    template <typename RValue>
    struct RValueEvaluator<Mem<RValue>> {
        constexpr static auto &get_value(memory_t &mem,
                                         asb_program_memory_t &program_mem) {
            auto id = RValueEvaluator<RValue>::get_value(mem, program_mem);
            return mem[id];
        }
    };

public:
    //Boots the computer and performs evaluations of the program.
    template <typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        ASBProgram<P>::evaluate(memory);
        return memory;
    }
};

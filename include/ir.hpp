#ifndef IR_HPP
#define IR_HPP

#include <string>
#include <vector>
#include <optional>

// ============================================================================
// ir.hpp — Three-Address Intermediate Representation (IR)
// ============================================================================
//
// Three-address code (TAC) is an intermediate representation where each
// instruction has at most one operator and at most three addresses (two operands
// and one destination).
//
// Supported instructions:
//   - LOAD_CONST dest, value
//   - MOVE dest, src
//   - ADD dest, left, right
//   - SUB dest, left, right
//   - MUL dest, left, right
//   - DIV dest, left, right
//   - CMP_EQ dest, left, right
//   - CMP_NE dest, left, right
//   - CMP_LT dest, left, right
//   - CMP_LE dest, left, right
//   - CMP_GT dest, left, right
//   - CMP_GE dest, left, right
//   - PRINT src
//   - LABEL label
//   - JUMP label
//   - JUMP_IF_FALSE condition, label
// ============================================================================

enum class IROpcode {
    LOAD_CONST,
    MOVE,
    ADD,
    SUB,
    MUL,
    DIV,
    CMP_EQ,
    CMP_NE,
    CMP_LT,
    CMP_LE,
    CMP_GT,
    CMP_GE,
    PRINT,
    LABEL,
    JUMP,
    JUMP_IF_FALSE
};

std::string irOpcodeToString(IROpcode opcode);

struct IRInstruction {
    IROpcode opcode;
    std::string dest;                  // Destination variable / temporary / condition / label
    std::string arg1;                  // First operand / variable / label
    std::string arg2;                  // Second operand
    std::optional<int> constValue;     // Constant value (for LOAD_CONST)

    // Factory methods for clean instruction creation
    static IRInstruction makeLoadConst(const std::string& dest, int value);
    static IRInstruction makeMove(const std::string& dest, const std::string& src);
    static IRInstruction makeBinary(IROpcode op, const std::string& dest,
                                   const std::string& left, const std::string& right);
    static IRInstruction makePrint(const std::string& src);
    static IRInstruction makeLabel(const std::string& label);
    static IRInstruction makeJump(const std::string& label);
    static IRInstruction makeJumpIfFalse(const std::string& cond, const std::string& label);

    // Human-readable textual representation
    std::string toString() const;
};

// Represents a sequential list of IR instructions
class IRProgram {
public:
    std::vector<IRInstruction> instructions;

    void addInstruction(const IRInstruction& inst);
    std::string toString() const;
};

#endif // IR_HPP

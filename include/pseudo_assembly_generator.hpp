#ifndef PSEUDO_ASSEMBLY_GENERATOR_HPP
#define PSEUDO_ASSEMBLY_GENERATOR_HPP

#include "ir.hpp"
#include <string>
#include <vector>

// ============================================================================
// pseudo_assembly_generator.hpp — Pseudo-Assembly Code Generator
// ============================================================================
//
// Converts Three-Address IR into a clean, educational pseudo-assembly language.
//
// Note: This target is an educational pseudo-assembly representation for learning
// compiler backends. It is NOT native x86-64 machine code, ARM, PTX, or GPU code.
//
// Instruction Set Architecture (MiniAsm):
//   - MOVI  dest, imm         : Load immediate integer into register/variable
//   - MOV   dest, src         : Copy value between registers/variables
//   - ADD   dest, left, right : dest = left + right
//   - SUB   dest, left, right : dest = left - right
//   - MUL   dest, left, right : dest = left * right
//   - DIV   dest, left, right : dest = left / right (integer division)
//   - CMPEQ dest, left, right : dest = (left == right ? 1 : 0)
//   - CMPNE dest, left, right : dest = (left != right ? 1 : 0)
//   - CMPLT dest, left, right : dest = (left <  right ? 1 : 0)
//   - CMPLE dest, left, right : dest = (left <= right ? 1 : 0)
//   - CMPGT dest, left, right : dest = (left >  right ? 1 : 0)
//   - CMPGE dest, left, right : dest = (left >= right ? 1 : 0)
//   - PRINT src               : Console output of integer operand
//   - label:                  : Jump target definition
//   - JMP   label             : Unconditional jump to target
//   - JMPF  cond, label       : Conditional jump to target if cond == 0
// ============================================================================

struct PseudoAssemblyInstruction {
    std::string opcode;             // "MOVI", "ADD", "JMP", etc. (empty for labels)
    std::string labelName;          // If isLabel is true, name of label (e.g. "L0")
    std::vector<std::string> args;  // Operands
    bool isLabel = false;           // Formats as "label:" without indentation

    std::string toString() const;
};

class PseudoAssemblyProgram {
public:
    std::vector<PseudoAssemblyInstruction> instructions;

    void addInstruction(const PseudoAssemblyInstruction& inst);
    std::string toString() const;
};

class PseudoAssemblyGenerator {
public:
    PseudoAssemblyGenerator() = default;

    // Translates an IRProgram into a PseudoAssemblyProgram
    // Throws std::runtime_error on unsupported or unknown opcodes
    PseudoAssemblyProgram generate(const IRProgram& ir);

    // Helper: translate single IR instruction
    static PseudoAssemblyInstruction translateInstruction(const IRInstruction& inst);
};

#endif // PSEUDO_ASSEMBLY_GENERATOR_HPP

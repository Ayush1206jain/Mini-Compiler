#include "pseudo_assembly_generator.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

std::string PseudoAssemblyInstruction::toString() const {
    if (isLabel) {
        return labelName + ":";
    }

    std::ostringstream oss;
    oss << "    ";
    oss << std::left << std::setw(8) << opcode;

    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << args[i];
    }

    return oss.str();
}

void PseudoAssemblyProgram::addInstruction(const PseudoAssemblyInstruction& inst) {
    instructions.push_back(inst);
}

std::string PseudoAssemblyProgram::toString() const {
    std::ostringstream oss;
    for (const auto& inst : instructions) {
        oss << inst.toString() << "\n";
    }
    return oss.str();
}

PseudoAssemblyInstruction PseudoAssemblyGenerator::translateInstruction(const IRInstruction& inst) {
    PseudoAssemblyInstruction pasm;

    switch (inst.opcode) {
        case IROpcode::LOAD_CONST:
            pasm.opcode = "MOVI";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(std::to_string(inst.constValue.value_or(0)));
            break;

        case IROpcode::MOVE:
            pasm.opcode = "MOV";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            break;

        case IROpcode::ADD:
            pasm.opcode = "ADD";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::SUB:
            pasm.opcode = "SUB";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::MUL:
            pasm.opcode = "MUL";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::DIV:
            pasm.opcode = "DIV";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_EQ:
            pasm.opcode = "CMPEQ";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_NE:
            pasm.opcode = "CMPNE";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_LT:
            pasm.opcode = "CMPLT";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_LE:
            pasm.opcode = "CMPLE";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_GT:
            pasm.opcode = "CMPGT";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::CMP_GE:
            pasm.opcode = "CMPGE";
            pasm.args.push_back(inst.dest);
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.arg2);
            break;

        case IROpcode::PRINT:
            pasm.opcode = "PRINT";
            pasm.args.push_back(inst.arg1);
            break;

        case IROpcode::LABEL:
            pasm.isLabel = true;
            pasm.labelName = inst.dest;
            break;

        case IROpcode::JUMP:
            pasm.opcode = "JMP";
            pasm.args.push_back(inst.dest);
            break;

        case IROpcode::JUMP_IF_FALSE:
            pasm.opcode = "JMPF";
            pasm.args.push_back(inst.arg1);
            pasm.args.push_back(inst.dest);
            break;

        default:
            throw std::runtime_error("Unsupported or unknown IR opcode for pseudo-assembly generation: " +
                                     irOpcodeToString(inst.opcode));
    }

    return pasm;
}

PseudoAssemblyProgram PseudoAssemblyGenerator::generate(const IRProgram& ir) {
    PseudoAssemblyProgram pasmProgram;
    for (const auto& inst : ir.instructions) {
        pasmProgram.addInstruction(translateInstruction(inst));
    }
    return pasmProgram;
}

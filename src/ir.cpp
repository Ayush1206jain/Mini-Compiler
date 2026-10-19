#include "ir.hpp"
#include <sstream>

std::string irOpcodeToString(IROpcode opcode) {
    switch (opcode) {
        case IROpcode::LOAD_CONST:   return "LOAD_CONST";
        case IROpcode::MOVE:         return "MOVE";
        case IROpcode::ADD:          return "ADD";
        case IROpcode::SUB:          return "SUB";
        case IROpcode::MUL:          return "MUL";
        case IROpcode::DIV:          return "DIV";
        case IROpcode::CMP_EQ:       return "CMP_EQ";
        case IROpcode::CMP_NE:       return "CMP_NE";
        case IROpcode::CMP_LT:       return "CMP_LT";
        case IROpcode::CMP_LE:       return "CMP_LE";
        case IROpcode::CMP_GT:       return "CMP_GT";
        case IROpcode::CMP_GE:       return "CMP_GE";
        case IROpcode::PRINT:        return "PRINT";
        case IROpcode::LABEL:        return "LABEL";
        case IROpcode::JUMP:         return "JUMP";
        case IROpcode::JUMP_IF_FALSE:return "JUMP_IF_FALSE";
        default:                     return "UNKNOWN";
    }
}

IRInstruction IRInstruction::makeLoadConst(const std::string& dest, int value) {
    IRInstruction inst;
    inst.opcode = IROpcode::LOAD_CONST;
    inst.dest = dest;
    inst.constValue = value;
    return inst;
}

IRInstruction IRInstruction::makeMove(const std::string& dest, const std::string& src) {
    IRInstruction inst;
    inst.opcode = IROpcode::MOVE;
    inst.dest = dest;
    inst.arg1 = src;
    return inst;
}

IRInstruction IRInstruction::makeBinary(IROpcode op, const std::string& dest,
                                       const std::string& left, const std::string& right) {
    IRInstruction inst;
    inst.opcode = op;
    inst.dest = dest;
    inst.arg1 = left;
    inst.arg2 = right;
    return inst;
}

IRInstruction IRInstruction::makePrint(const std::string& src) {
    IRInstruction inst;
    inst.opcode = IROpcode::PRINT;
    inst.arg1 = src;
    return inst;
}

IRInstruction IRInstruction::makeLabel(const std::string& label) {
    IRInstruction inst;
    inst.opcode = IROpcode::LABEL;
    inst.dest = label;
    return inst;
}

IRInstruction IRInstruction::makeJump(const std::string& label) {
    IRInstruction inst;
    inst.opcode = IROpcode::JUMP;
    inst.dest = label;
    return inst;
}

IRInstruction IRInstruction::makeJumpIfFalse(const std::string& cond, const std::string& label) {
    IRInstruction inst;
    inst.opcode = IROpcode::JUMP_IF_FALSE;
    inst.arg1 = cond;
    inst.dest = label;
    return inst;
}

std::string IRInstruction::toString() const {
    std::ostringstream oss;
    switch (opcode) {
        case IROpcode::LOAD_CONST:
            oss << "LOAD_CONST " << dest << ", " << (constValue.has_value() ? *constValue : 0);
            break;
        case IROpcode::MOVE:
            oss << "MOVE " << dest << ", " << arg1;
            break;
        case IROpcode::ADD:
        case IROpcode::SUB:
        case IROpcode::MUL:
        case IROpcode::DIV:
        case IROpcode::CMP_EQ:
        case IROpcode::CMP_NE:
        case IROpcode::CMP_LT:
        case IROpcode::CMP_LE:
        case IROpcode::CMP_GT:
        case IROpcode::CMP_GE:
            oss << irOpcodeToString(opcode) << " " << dest << ", " << arg1 << ", " << arg2;
            break;
        case IROpcode::PRINT:
            oss << "PRINT " << arg1;
            break;
        case IROpcode::LABEL:
            oss << "LABEL " << dest;
            break;
        case IROpcode::JUMP:
            oss << "JUMP " << dest;
            break;
        case IROpcode::JUMP_IF_FALSE:
            oss << "JUMP_IF_FALSE " << arg1 << ", " << dest;
            break;
    }
    return oss.str();
}

void IRProgram::addInstruction(const IRInstruction& inst) {
    instructions.push_back(inst);
}

std::string IRProgram::toString() const {
    std::ostringstream oss;
    for (const auto& inst : instructions) {
        oss << inst.toString() << "\n";
    }
    return oss.str();
}

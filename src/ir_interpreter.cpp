#include "ir_interpreter.hpp"

IRInterpreter::IRInterpreter(size_t maxSteps)
    : maxSteps_(maxSteps) {}

void IRInterpreter::buildLabelMap(const IRProgram& program) {
    labelMap_.clear();
    for (size_t i = 0; i < program.instructions.size(); ++i) {
        const auto& inst = program.instructions[i];
        if (inst.opcode == IROpcode::LABEL) {
            if (labelMap_.find(inst.dest) != labelMap_.end()) {
                throw RuntimeError("Duplicate label definition: " + inst.dest);
            }
            labelMap_[inst.dest] = i;
        }
    }
}

int IRInterpreter::readOperand(const std::string& name) const {
    auto it = env_.find(name);
    if (it == env_.end()) {
        throw RuntimeError("Use of uninitialized variable or temporary: '" + name + "'");
    }
    return it->second;
}

int IRInterpreter::getVariable(const std::string& name) const {
    return readOperand(name);
}

bool IRInterpreter::hasVariable(const std::string& name) const {
    return env_.find(name) != env_.end();
}

std::vector<int> IRInterpreter::execute(const IRProgram& program) {
    env_.clear();
    output_.clear();
    buildLabelMap(program);

    size_t pc = 0; // Program counter
    size_t steps = 0;
    const size_t n = program.instructions.size();

    while (pc < n) {
        if (++steps > maxSteps_) {
            throw RuntimeError("Instruction execution step limit exceeded (" +
                               std::to_string(maxSteps_) + " steps). Possible infinite loop.");
        }

        const auto& inst = program.instructions[pc];

        switch (inst.opcode) {
            case IROpcode::LOAD_CONST: {
                env_[inst.dest] = inst.constValue.value_or(0);
                pc++;
                break;
            }
            case IROpcode::MOVE: {
                int val = readOperand(inst.arg1);
                env_[inst.dest] = val;
                pc++;
                break;
            }
            case IROpcode::ADD: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = left + right;
                pc++;
                break;
            }
            case IROpcode::SUB: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = left - right;
                pc++;
                break;
            }
            case IROpcode::MUL: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = left * right;
                pc++;
                break;
            }
            case IROpcode::DIV: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                if (right == 0) {
                    throw RuntimeError("Division by zero: cannot divide " + std::to_string(left) + " by 0");
                }
                env_[inst.dest] = left / right;
                pc++;
                break;
            }
            case IROpcode::CMP_EQ: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left == right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::CMP_NE: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left != right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::CMP_LT: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left < right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::CMP_LE: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left <= right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::CMP_GT: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left > right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::CMP_GE: {
                int left = readOperand(inst.arg1);
                int right = readOperand(inst.arg2);
                env_[inst.dest] = (left >= right) ? 1 : 0;
                pc++;
                break;
            }
            case IROpcode::PRINT: {
                int val = readOperand(inst.arg1);
                output_.push_back(val);
                pc++;
                break;
            }
            case IROpcode::LABEL: {
                // Label instruction is a no-op during linear execution
                pc++;
                break;
            }
            case IROpcode::JUMP: {
                auto it = labelMap_.find(inst.dest);
                if (it == labelMap_.end()) {
                    throw RuntimeError("Invalid jump target: label '" + inst.dest + "' not found");
                }
                pc = it->second;
                break;
            }
            case IROpcode::JUMP_IF_FALSE: {
                int cond = readOperand(inst.arg1);
                if (cond == 0) {
                    auto it = labelMap_.find(inst.dest);
                    if (it == labelMap_.end()) {
                        throw RuntimeError("Invalid jump target: label '" + inst.dest + "' not found");
                    }
                    pc = it->second;
                } else {
                    pc++;
                }
                break;
            }
        }
    }

    return output_;
}

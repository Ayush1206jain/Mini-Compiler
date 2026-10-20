#include "ir_optimizer.hpp"
#include "dead_code_elimination.hpp"
#include <climits>

IROptimizer::IROptimizer(int maxPasses)
    : maxPasses_(maxPasses) {}

bool IROptimizer::isArithmeticOp(IROpcode op) {
    return op == IROpcode::ADD || op == IROpcode::SUB ||
           op == IROpcode::MUL || op == IROpcode::DIV;
}

bool IROptimizer::isComparisonOp(IROpcode op) {
    return op == IROpcode::CMP_EQ || op == IROpcode::CMP_NE ||
           op == IROpcode::CMP_LT || op == IROpcode::CMP_LE ||
           op == IROpcode::CMP_GT || op == IROpcode::CMP_GE;
}

std::optional<int> IROptimizer::evaluateBinary(IROpcode op, int left, int right) {
    switch (op) {
        case IROpcode::ADD: {
            // Check for potential signed overflow before computing
            long long res = static_cast<long long>(left) + static_cast<long long>(right);
            if (res > INT_MAX || res < INT_MIN) return std::nullopt;
            return static_cast<int>(res);
        }
        case IROpcode::SUB: {
            long long res = static_cast<long long>(left) - static_cast<long long>(right);
            if (res > INT_MAX || res < INT_MIN) return std::nullopt;
            return static_cast<int>(res);
        }
        case IROpcode::MUL: {
            long long res = static_cast<long long>(left) * static_cast<long long>(right);
            if (res > INT_MAX || res < INT_MIN) return std::nullopt;
            return static_cast<int>(res);
        }
        case IROpcode::DIV: {
            // Strict safety: DO NOT fold division by zero; leave it for runtime error
            if (right == 0) return std::nullopt;
            // Prevent INT_MIN / -1 overflow
            if (left == INT_MIN && right == -1) return std::nullopt;
            return left / right;
        }
        case IROpcode::CMP_EQ: return (left == right) ? 1 : 0;
        case IROpcode::CMP_NE: return (left != right) ? 1 : 0;
        case IROpcode::CMP_LT: return (left < right)  ? 1 : 0;
        case IROpcode::CMP_LE: return (left <= right) ? 1 : 0;
        case IROpcode::CMP_GT: return (left > right)  ? 1 : 0;
        case IROpcode::CMP_GE: return (left >= right) ? 1 : 0;
        default:
            return std::nullopt;
    }
}

bool IROptimizer::runSinglePass(IRProgram& program) {
    bool changed = false;
    std::unordered_map<std::string, int> constEnv;

    for (size_t i = 0; i < program.instructions.size(); ++i) {
        auto& inst = program.instructions[i];

        // 1. Control flow boundary: reset constant environment conservatively
        if (inst.opcode == IROpcode::LABEL ||
            inst.opcode == IROpcode::JUMP ||
            inst.opcode == IROpcode::JUMP_IF_FALSE) {
            constEnv.clear();
            continue;
        }

        // 2. Handle LOAD_CONST
        if (inst.opcode == IROpcode::LOAD_CONST) {
            if (inst.constValue.has_value()) {
                constEnv[inst.dest] = *inst.constValue;
            } else {
                constEnv.erase(inst.dest);
            }
            continue;
        }

        // 3. Handle MOVE
        if (inst.opcode == IROpcode::MOVE) {
            auto it = constEnv.find(inst.arg1);
            if (it != constEnv.end()) {
                // MOVE dest, src where src is known constant => LOAD_CONST dest, val
                inst = IRInstruction::makeLoadConst(inst.dest, it->second);
                constEnv[inst.dest] = it->second;
                stats_.propagatedConstants++;
                changed = true;
            } else {
                // Destination is assigned non-constant => invalidate destination fact
                constEnv.erase(inst.dest);
            }
            continue;
        }

        // 4. Handle Arithmetic and Comparison operations (Binary operations)
        if (isArithmeticOp(inst.opcode) || isComparisonOp(inst.opcode)) {
            auto it1 = constEnv.find(inst.arg1);
            auto it2 = constEnv.find(inst.arg2);

            // Both operands are known constants: Constant Folding!
            if (it1 != constEnv.end() && it2 != constEnv.end()) {
                auto folded = evaluateBinary(inst.opcode, it1->second, it2->second);
                if (folded.has_value()) {
                    inst = IRInstruction::makeLoadConst(inst.dest, *folded);
                    constEnv[inst.dest] = *folded;
                    stats_.foldedOperations++;
                    changed = true;
                    continue;
                }
            }

            // If not folded, destination has a new non-constant value => invalidate destination fact
            constEnv.erase(inst.dest);
            continue;
        }

        // 5. Handle PRINT or other instructions
        if (inst.opcode == IROpcode::PRINT) {
            // Note: In TAC, PRINT prints variable/temp. It does not modify destination.
            continue;
        }

        // Fallback: if any unknown instruction has a destination, invalidate it
        if (!inst.dest.empty()) {
            constEnv.erase(inst.dest);
        }
    }

    return changed;
}

IRProgram IROptimizer::optimize(const IRProgram& program) {
    IRProgram optimized = program; // Work on a copy; preserve original
    stats_ = OptimizationStats();

    for (int pass = 0; pass < maxPasses_; ++pass) {
        stats_.passesRun++;
        bool changed = runSinglePass(optimized);
        if (!changed) {
            break; // Reached fixed point convergence
        }
    }

    return optimized;
}

IRProgram IROptimizer::optimizeWithDCE(const IRProgram& program) {
    IRProgram constFolded = optimize(program);
    DeadCodeElimination dce(maxPasses_);
    return dce.eliminate(constFolded);
}


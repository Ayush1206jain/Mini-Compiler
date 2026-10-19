#ifndef IR_OPTIMIZER_HPP
#define IR_OPTIMIZER_HPP

#include "ir.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <optional>

// ============================================================================
// ir_optimizer.hpp — Constant Folding and Constant Propagation on Three-Address IR
// ============================================================================
//
// Day 4 Optimization Passes:
//   1. Constant Propagation:
//      Tracks known constant values in a constant environment (symbol -> int)
//      and propagates them when operands can be resolved to constants.
//
//   2. Constant Folding:
//      Evaluates arithmetic (+, -, *, /) and comparison (==, !=, <, <=, >, >=)
//      operations at compile time when both operands are known constants.
//      Converts:
//         OP dest, left, right  -->  LOAD_CONST dest, evaluated_value
//
//   3. Conservative Control-Flow Safety:
//      Whenever a label or jump instruction (LABEL, JUMP, JUMP_IF_FALSE) is
//      encountered, the constant environment is cleared/reset. This prevents
//      unsafe cross-basic-block propagation into branches or loops where
//      variables may have differing values on alternate paths.
//
//   4. Iterative Convergence:
//      Repeats propagation and folding passes until no further changes occur
//      or a fixed maximum iteration limit is reached (default 10 iterations).
// ============================================================================

struct OptimizationStats {
    int foldedOperations = 0;
    int propagatedConstants = 0;
    int passesRun = 0;
};

class IROptimizer {
public:
    explicit IROptimizer(int maxPasses = 10);

    // Runs optimization on an IRProgram and returns a new optimized IRProgram
    IRProgram optimize(const IRProgram& program);

    const OptimizationStats& getStats() const { return stats_; }

private:
    int maxPasses_;
    OptimizationStats stats_;

    // Single optimization pass over instructions
    bool runSinglePass(IRProgram& program);

    // Helpers to evaluate operations safely
    static bool isArithmeticOp(IROpcode op);
    static bool isComparisonOp(IROpcode op);
    static std::optional<int> evaluateBinary(IROpcode op, int left, int right);
};

#endif // IR_OPTIMIZER_HPP

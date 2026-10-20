#ifndef DEAD_CODE_ELIMINATION_HPP
#define DEAD_CODE_ELIMINATION_HPP

#include "ir.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

// ============================================================================
// dead_code_elimination.hpp — Dead-Code Elimination (DCE) Pass on Three-Address IR
// ============================================================================
//
// Day 5 Optimization Pass:
//   Identifies and eliminates instructions whose computed values are never used
//   and whose execution produces no observable side effects.
//
// Key Principles:
//   1. Side-Effect Preservation:
//      - PRINT instructions produce observable console output and are NEVER removed.
//      - Control flow instructions (LABEL, JUMP, JUMP_IF_FALSE) structure program flow
//        and are NEVER removed by standard DCE.
//
//   2. Potentially Trapping Operations:
//      - Division (DIV) can trap at runtime if the divisor is zero.
//      - To preserve exact runtime language semantics and error reporting,
//        DIV instructions are treated as potentially trapping and are NOT removed.
//
//   3. Control-Flow-Aware Liveness Analysis:
//      - Deconstructs the IRProgram into a Control Flow Graph (CFG) of Basic Blocks.
//      - Computes block-level `gen` (upward-exposed uses) and `kill` (definitions).
//      - Solves standard backward dataflow equations:
//          Out[B] = Union_{S in Succ[B]} In[S]
//          In[B]  = gen[B] Union (Out[B] \ kill[B])
//      - Inside each basic block, scans backwards from Out[B]. An instruction defining
//        `dest` is dead if `dest` is not in the active live set at that point.
//
//   4. Fixed-Point Iterative Convergence:
//      - Removing one dead instruction may cause the instructions defining its operands
//        to become dead in turn (cascading DCE).
//      - The pass repeats until no further instructions are eliminated or maxPasses is reached.
// ============================================================================

struct DCEStats {
    int eliminatedInstructions = 0;
    int passesRun = 0;
};

class DeadCodeElimination {
public:
    explicit DeadCodeElimination(int maxPasses = 10);

    // Runs dead-code elimination on the given IRProgram and returns an optimized copy
    IRProgram eliminate(const IRProgram& program);

    const DCEStats& getStats() const { return stats_; }

    // Helpers to classify instruction purity and side-effects
    static bool isPure(const IRInstruction& inst);
    static bool isPotentiallyTrapping(const IRInstruction& inst);
    static bool hasSideEffects(const IRInstruction& inst);
    static std::vector<std::string> getOperandsUsed(const IRInstruction& inst);
    static std::string getDestinationDefined(const IRInstruction& inst);

private:
    int maxPasses_;
    DCEStats stats_;

    // Runs a single backward-liveness DCE pass over the program
    // Returns true if one or more instructions were eliminated
    bool runSinglePass(IRProgram& program);
};

#endif // DEAD_CODE_ELIMINATION_HPP

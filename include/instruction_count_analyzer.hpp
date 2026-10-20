#ifndef INSTRUCTION_COUNT_ANALYZER_HPP
#define INSTRUCTION_COUNT_ANALYZER_HPP

#include "ir.hpp"
#include <string>

// ============================================================================
// instruction_count_analyzer.hpp — IR Instruction Count & Optimization Metrics
// ============================================================================
//
// Analyzes and compares the size and instruction makeup of IRProgram instances
// across different stages of the compiler pipeline:
//   1. Original IR (from IRGenerator)
//   2. Folded IR (after Constant Folding & Propagation)
//   3. Final Optimized IR (after Dead-Code Elimination)
//
// Definition of an IR Instruction:
//   Every entry in `IRProgram::instructions` is counted as 1 instruction,
//   including computational operations, moves, memory/constants, prints,
//   labels, and branch/jump instructions.
//
// Percentage Reduction Formula:
//   ((originalCount - optimizedCount) / originalCount) * 100
//   Safely handles originalCount == 0 (returns 0.0%).
// ============================================================================

struct InstructionStats {
    int originalCount = 0;
    int foldedCount = 0;
    int dceCount = 0;            // Final optimized count after DCE
    int absoluteReduction = 0;   // originalCount - dceCount
    double percentageReduction = 0.0;

    // Detailed breakdown of the final optimized IR
    int arithmeticCount = 0;
    int comparisonCount = 0;
    int moveOrLoadCount = 0;
    int controlFlowCount = 0;    // JUMP, JUMP_IF_FALSE, LABEL
    int printCount = 0;

    std::string toString() const;
};

class InstructionCountAnalyzer {
public:
    // Analyzes instruction counts across original, folded, and DCE programs
    static InstructionStats analyze(const IRProgram& original,
                                   const IRProgram& folded,
                                   const IRProgram& dce);

    // Calculates percentage reduction safely
    static double calculateReductionPercentage(int originalCount, int optimizedCount);

    // Counts instructions in a single IR program
    static int countInstructions(const IRProgram& program);
};

#endif // INSTRUCTION_COUNT_ANALYZER_HPP

#include "instruction_count_analyzer.hpp"
#include <sstream>
#include <iomanip>

double InstructionCountAnalyzer::calculateReductionPercentage(int originalCount, int optimizedCount) {
    if (originalCount <= 0) {
        return 0.0;
    }
    double diff = static_cast<double>(originalCount - optimizedCount);
    return (diff / static_cast<double>(originalCount)) * 100.0;
}

int InstructionCountAnalyzer::countInstructions(const IRProgram& program) {
    return static_cast<int>(program.instructions.size());
}

InstructionStats InstructionCountAnalyzer::analyze(const IRProgram& original,
                                                  const IRProgram& folded,
                                                  const IRProgram& dce) {
    InstructionStats stats;
    stats.originalCount = countInstructions(original);
    stats.foldedCount = countInstructions(folded);
    stats.dceCount = countInstructions(dce);
    stats.absoluteReduction = stats.originalCount - stats.dceCount;
    stats.percentageReduction = calculateReductionPercentage(stats.originalCount, stats.dceCount);

    // Inspect breakdown of final optimized program
    for (const auto& inst : dce.instructions) {
        switch (inst.opcode) {
            case IROpcode::ADD:
            case IROpcode::SUB:
            case IROpcode::MUL:
            case IROpcode::DIV:
                stats.arithmeticCount++;
                break;
            case IROpcode::CMP_EQ:
            case IROpcode::CMP_NE:
            case IROpcode::CMP_LT:
            case IROpcode::CMP_LE:
            case IROpcode::CMP_GT:
            case IROpcode::CMP_GE:
                stats.comparisonCount++;
                break;
            case IROpcode::LOAD_CONST:
            case IROpcode::MOVE:
                stats.moveOrLoadCount++;
                break;
            case IROpcode::LABEL:
            case IROpcode::JUMP:
            case IROpcode::JUMP_IF_FALSE:
                stats.controlFlowCount++;
                break;
            case IROpcode::PRINT:
                stats.printCount++;
                break;
        }
    }

    return stats;
}

std::string InstructionStats::toString() const {
    std::ostringstream oss;
    oss << "=== IR Instruction Count Statistics ===" << "\n";
    oss << "Original IR:                    " << originalCount << " instructions\n";
    oss << "After Constant Folding (Day 4): " << foldedCount << " instructions\n";
    oss << "After Dead-Code Elim (Day 5):   " << dceCount << " instructions\n";
    oss << "Absolute Reduction:             " << absoluteReduction << " instructions\n";
    oss << std::fixed << std::setprecision(2);
    oss << "Percentage Reduction:           " << percentageReduction << "%\n";
    oss << "Optimized IR Breakdown:         "
        << moveOrLoadCount << " load/mov, "
        << arithmeticCount << " arith, "
        << comparisonCount << " cmp, "
        << controlFlowCount << " ctrl, "
        << printCount << " print\n";
    return oss.str();
}

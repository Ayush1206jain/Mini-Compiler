// ============================================================================
// instruction_count_tests.cpp — Tests for IR Instruction-Count Analysis
// ============================================================================

#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include "instruction_count_analyzer.hpp"
#include "ir_generator.hpp"
#include "ir_optimizer.hpp"
#include "dead_code_elimination.hpp"
#include "pipeline_validator.hpp"

static int testsRun = 0;
static int testsPassed = 0;

#define RUN_TEST(testFunc) do { \
    testsRun++; \
    std::cout << "  Running: " << #testFunc << "... "; \
    try { \
        testFunc(); \
        testsPassed++; \
        std::cout << "PASSED" << std::endl; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << std::endl; \
    } catch (...) { \
        std::cout << "FAILED: unknown exception" << std::endl; \
    } \
} while(0)

// Helper: check floating point proximity
static bool approxEqual(double a, double b, double eps = 0.01) {
    return std::fabs(a - b) < eps;
}

// ---------------------------------------------------------------------------
// 1. Significant Instruction Reduction via Constant Folding & DCE
// ---------------------------------------------------------------------------
void testSignificantInstructionReduction() {
    std::string src =
        "let x = 10 + 20;\n"
        "let dead1 = 100 * 2;\n"
        "let dead2 = dead1 + 5;\n"
        "print(x);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);

    // Original IR contains instructions for x arithmetic + dead1 + dead2
    // Optimized IR folds x to 30, and eliminates dead1 and dead2 completely
    assert(report.instructionStats.originalCount > report.instructionStats.dceCount);
    assert(report.instructionStats.absoluteReduction > 0);
    assert(report.instructionStats.percentageReduction > 50.0);
}

// ---------------------------------------------------------------------------
// 2. Program Where Optimization Does NOT Reduce Instruction Count
// ---------------------------------------------------------------------------
void testZeroReductionWhenAlreadyMinimal() {
    // A single direct print of a variable cannot be folded or eliminated
    std::string src =
        "let x = 10;\n"
        "print(x);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);

    // Original IR:
    // LOAD_CONST x, 10
    // PRINT x
    // Both are strictly necessary and live!
    assert(report.instructionStats.originalCount == 2);
    assert(report.instructionStats.dceCount == 2);
    assert(report.instructionStats.absoluteReduction == 0);
    assert(approxEqual(report.instructionStats.percentageReduction, 0.0));
}

// ---------------------------------------------------------------------------
// 3. Safe Zero Handling (Original Count Zero)
// ---------------------------------------------------------------------------
void testZeroOriginalCountSafeHandling() {
    IRProgram empty1;
    IRProgram empty2;
    IRProgram empty3;

    InstructionStats stats = InstructionCountAnalyzer::analyze(empty1, empty2, empty3);
    assert(stats.originalCount == 0);
    assert(stats.dceCount == 0);
    assert(stats.absoluteReduction == 0);
    assert(approxEqual(stats.percentageReduction, 0.0));

    // Direct calculation helper test
    double pct = InstructionCountAnalyzer::calculateReductionPercentage(0, 0);
    assert(approxEqual(pct, 0.0));
}

// ---------------------------------------------------------------------------
// 4. Reduction Percentage Mathematical Correctness
// ---------------------------------------------------------------------------
void testPercentageReductionFormula() {
    // 100 -> 25 should be 75.0%
    double pct1 = InstructionCountAnalyzer::calculateReductionPercentage(100, 25);
    assert(approxEqual(pct1, 75.0));

    // 10 -> 5 should be 50.0%
    double pct2 = InstructionCountAnalyzer::calculateReductionPercentage(10, 5);
    assert(approxEqual(pct2, 50.0));

    // 20 -> 20 should be 0.0%
    double pct3 = InstructionCountAnalyzer::calculateReductionPercentage(20, 20);
    assert(approxEqual(pct3, 0.0));

    // 35 -> 24 should be ((35 - 24) / 35) * 100 = 11 / 35 * 100 = 31.428...%
    double pct4 = InstructionCountAnalyzer::calculateReductionPercentage(35, 24);
    assert(approxEqual(pct4, 31.43));
}

// ---------------------------------------------------------------------------
// 5. Instruction Category Breakdown Accuracy
// ---------------------------------------------------------------------------
void testInstructionBreakdownCategories() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("x", 10)); // moveOrLoad
    prog.addInstruction(IRInstruction::makeMove("y", "x"));     // moveOrLoad
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t0", "x", "y")); // arithmetic
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_LT, "t1", "x", "y")); // comparison
    prog.addInstruction(IRInstruction::makeLabel("L0"));        // controlFlow
    prog.addInstruction(IRInstruction::makeJump("L0"));         // controlFlow
    prog.addInstruction(IRInstruction::makePrint("x"));         // print

    InstructionStats stats = InstructionCountAnalyzer::analyze(prog, prog, prog);
    assert(stats.originalCount == 7);
    assert(stats.dceCount == 7);
    assert(stats.moveOrLoadCount == 2);
    assert(stats.arithmeticCount == 1);
    assert(stats.comparisonCount == 1);
    assert(stats.controlFlowCount == 2);
    assert(stats.printCount == 1);
}

// ---------------------------------------------------------------------------
// 6. Loop Preservation and Instruction Count
// ---------------------------------------------------------------------------
void testLoopInstructionCountIntegrity() {
    std::string src =
        "let i = 0;\n"
        "let sum = 0;\n"
        "while (i < 3) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "print(sum);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);

    // Loops retain necessary branch targets and accumulators
    assert(report.instructionStats.controlFlowCount >= 3); // LABEL, JUMP_IF_FALSE, JUMP, exit LABEL
    assert(report.instructionStats.printCount == 1);
    assert(report.instructionStats.dceCount > 0);
}

int main() {
    std::cout << "=== Mini Compiler Instruction-Count Analysis Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testSignificantInstructionReduction);
    RUN_TEST(testZeroReductionWhenAlreadyMinimal);
    RUN_TEST(testZeroOriginalCountSafeHandling);
    RUN_TEST(testPercentageReductionFormula);
    RUN_TEST(testInstructionBreakdownCategories);
    RUN_TEST(testLoopInstructionCountIntegrity);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

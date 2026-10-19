// ============================================================================
// ir_optimizer_tests.cpp — Automated tests for Constant Folding & Propagation
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "ir.hpp"
#include "ir_generator.hpp"
#include "ir_interpreter.hpp"
#include "ir_optimizer.hpp"

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

// Helper: compile source string to IR
static IRProgram compileToIR(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parseProgram();
    IRGenerator irGen;
    return irGen.generate(*ast);
}

// Helper: optimize IR
static IRProgram optimizeIR(const IRProgram& program, IROptimizer* optOut = nullptr) {
    IROptimizer optimizer;
    auto opt = optimizer.optimize(program);
    if (optOut) *optOut = optimizer;
    return opt;
}

// ---------------------------------------------------------------------------
// 1. Addition of constants
// ---------------------------------------------------------------------------
void testFoldAddition() {
    std::string src = "let x = 10 + 20; print(x);";
    IRProgram orig = compileToIR(src);
    IROptimizer opt;
    IRProgram optimized = opt.optimize(orig);

    // Verify ADD was folded
    assert(orig.toString().find("ADD") != std::string::npos);
    assert(optimized.toString().find("ADD") == std::string::npos);
    assert(opt.getStats().foldedOperations >= 1);

    // Verify execution output matches
    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({30}));
}

// ---------------------------------------------------------------------------
// 2. Subtraction of constants
// ---------------------------------------------------------------------------
void testFoldSubtraction() {
    std::string src = "let x = 50 - 15; print(x);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    assert(orig.toString().find("SUB") != std::string::npos);
    assert(optimized.toString().find("SUB") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({35}));
}

// ---------------------------------------------------------------------------
// 3. Multiplication of constants
// ---------------------------------------------------------------------------
void testFoldMultiplication() {
    std::string src = "let x = 6 * 7; print(x);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    assert(orig.toString().find("MUL") != std::string::npos);
    assert(optimized.toString().find("MUL") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({42}));
}

// ---------------------------------------------------------------------------
// 4. Division of constants where safe
// ---------------------------------------------------------------------------
void testFoldDivision() {
    std::string src = "let x = 100 / 4; print(x);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    assert(orig.toString().find("DIV") != std::string::npos);
    assert(optimized.toString().find("DIV") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({25}));
}

// ---------------------------------------------------------------------------
// 5. Supported comparisons with constant operands
// ---------------------------------------------------------------------------
void testFoldComparisons() {
    std::string src = "if (10 > 5) { print(1); } else { print(0); }";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    // CMP_GT 10, 5 should be folded
    assert(orig.toString().find("CMP_GT") != std::string::npos);
    assert(optimized.toString().find("CMP_GT") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({1}));
}

// ---------------------------------------------------------------------------
// 6. Nested arithmetic expressions
// ---------------------------------------------------------------------------
void testFoldNestedArithmetic() {
    std::string src = "let x = 2 + 3 * 4; print(x);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    // All arithmetic should fold to 14
    assert(orig.toString().find("MUL") != std::string::npos);
    assert(orig.toString().find("ADD") != std::string::npos);
    assert(optimized.toString().find("MUL") == std::string::npos);
    assert(optimized.toString().find("ADD") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({14}));
}

// ---------------------------------------------------------------------------
// 7. Expressions requiring multiple optimization passes
// ---------------------------------------------------------------------------
void testMultiplePassesRequired() {
    // a = 5; b = a; c = b + 10;
    std::string src = "let a = 5; let b = a; let c = b + 10; print(c);";
    IRProgram orig = compileToIR(src);
    IROptimizer opt;
    IRProgram optimized = opt.optimize(orig);

    // Requires propagation from a -> b, then folding b + 10
    assert(opt.getStats().passesRun >= 2);
    assert(optimized.toString().find("ADD") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({15}));
}

// ---------------------------------------------------------------------------
// 8. Division by zero remains safe and preserves existing error behavior
// ---------------------------------------------------------------------------
void testDivisionByZeroSafety() {
    std::string src = "let x = 10 / 0; print(x);";
    IRProgram orig = compileToIR(src);
    IROptimizer opt;
    IRProgram optimized = opt.optimize(orig);

    // DIV must NOT be folded away; division by zero should be preserved
    assert(optimized.toString().find("DIV") != std::string::npos);

    // Both should throw identical division by zero RuntimeError
    bool origThrew = false;
    try {
        IRInterpreter interp;
        interp.execute(orig);
    } catch (const RuntimeError& e) {
        origThrew = true;
    }
    assert(origThrew);

    bool optThrew = false;
    try {
        IRInterpreter interp;
        interp.execute(optimized);
    } catch (const RuntimeError& e) {
        optThrew = true;
    }
    assert(optThrew);
}

// ---------------------------------------------------------------------------
// 9. Propagation through a constant assignment
// ---------------------------------------------------------------------------
void testPropagationThroughAssignment() {
    std::string src = "let a = 10; let b = a; print(b);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    // MOVE b, a should be transformed into LOAD_CONST b, 10
    assert(orig.toString().find("MOVE") != std::string::npos);
    assert(optimized.toString().find("LOAD_CONST b, 10") != std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 10. Propagation through a temporary
// ---------------------------------------------------------------------------
void testPropagationThroughTemporary() {
    std::string src = "let x = 10; let y = 20; let z = x + y; print(z);";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    // Both x and y propagated and folded to 30
    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({30}));
}

// ---------------------------------------------------------------------------
// 11. Invalidation after reassignment
// ---------------------------------------------------------------------------
void testInvalidationAfterReassignment() {
    std::string src =
        "let x = 10;\n"
        "let y = x;\n"
        "x = 20;\n"
        "let z = x;\n"
        "print(y);\n"
        "print(z);\n";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == (std::vector<int>{10, 20}));
}

// ---------------------------------------------------------------------------
// 12. No stale constant facts after control-flow boundaries
// ---------------------------------------------------------------------------
void testControlFlowConservativeReset() {
    std::string src =
        "let x = 1;\n"
        "if (x > 0) {\n"
        "    x = 2;\n"
        "} else {\n"
        "    x = 3;\n"
        "}\n"
        "print(x);\n";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    // After LABEL L_end, x could be either 2 or 3.
    // The optimizer must NOT assume x == 1 or x == 2.
    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({2}));
}

// ---------------------------------------------------------------------------
// 13. While loop preserves correct runtime semantics
// ---------------------------------------------------------------------------
void testWhileLoopSemanticsPreserved() {
    std::string src =
        "let i = 0;\n"
        "let sum = 0;\n"
        "while (i < 4) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "print(sum);\n";
    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(optimized));
    assert(interp2.execute(optimized) == std::vector<int>({6}));
}

// ---------------------------------------------------------------------------
// 14. Full plan example equivalence test
// ---------------------------------------------------------------------------
void testFullPlanExampleEquivalence() {
    std::string src =
        "let x = 2 + 3 * 4;\n"
        "\n"
        "if (x > 10) {\n"
        "    print(x);\n"
        "} else {\n"
        "    print(0);\n"
        "}\n"
        "\n"
        "let i = 0;\n"
        "while (i < 3) {\n"
        "    print(i);\n"
        "    i = i + 1;\n"
        "}\n";

    IRProgram orig = compileToIR(src);
    IRProgram optimized = optimizeIR(orig);

    IRInterpreter interp1, interp2;
    std::vector<int> outOrig = interp1.execute(orig);
    std::vector<int> outOpt  = interp2.execute(optimized);

    std::vector<int> expected = {14, 0, 1, 2};
    assert(outOrig == expected);
    assert(outOpt == expected);
}

int main() {
    std::cout << "=== Mini Compiler IR Optimizer Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testFoldAddition);
    RUN_TEST(testFoldSubtraction);
    RUN_TEST(testFoldMultiplication);
    RUN_TEST(testFoldDivision);
    RUN_TEST(testFoldComparisons);
    RUN_TEST(testFoldNestedArithmetic);
    RUN_TEST(testMultiplePassesRequired);
    RUN_TEST(testDivisionByZeroSafety);
    RUN_TEST(testPropagationThroughAssignment);
    RUN_TEST(testPropagationThroughTemporary);
    RUN_TEST(testInvalidationAfterReassignment);
    RUN_TEST(testControlFlowConservativeReset);
    RUN_TEST(testWhileLoopSemanticsPreserved);
    RUN_TEST(testFullPlanExampleEquivalence);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

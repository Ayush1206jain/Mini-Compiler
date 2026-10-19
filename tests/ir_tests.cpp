// ============================================================================
// ir_tests.cpp — Automated tests for IR Generation and IR Interpreter
// ============================================================================
//
// Covers all Day 3 test requirements:
//  1. Constant assignment and printing
//  2. Variable-to-variable assignment
//  3. Addition and subtraction
//  4. Multiplication and operator precedence
//  5. Nested arithmetic expressions
//  6. Division and division-by-zero handling
//  7. Comparisons supported by the language
//  8. if/else branch behavior
//  9. while loop behavior
// 10. Multiple print statements and output order
// 11. Temporary-variable creation and deterministic IR generation
// 12. Invalid jump targets or equivalent defensive checks
// 13. Use of an uninitialized variable
// 14. Execution-limit handling for a deliberately non-terminating loop
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

// Helper: compiles source to IR
static IRProgram compileToIR(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parseProgram();
    IRGenerator irGen;
    return irGen.generate(*ast);
}

// Helper: runs source and returns captured integers
static std::vector<int> runProgram(const std::string& source, size_t maxSteps = 100000) {
    IRProgram ir = compileToIR(source);
    IRInterpreter interpreter(maxSteps);
    return interpreter.execute(ir);
}

// ---------------------------------------------------------------------------
// 1. Constant assignment and printing
// ---------------------------------------------------------------------------
void testConstantAssignmentAndPrint() {
    std::string src = "let x = 42; print(x);";
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 42);
}

// ---------------------------------------------------------------------------
// 2. Variable-to-variable assignment
// ---------------------------------------------------------------------------
void testVariableToVariableAssignment() {
    std::string src = "let a = 10; let b = a; b = b + 5; print(a); print(b);";
    auto out = runProgram(src);
    assert(out.size() == 2);
    assert(out[0] == 10);
    assert(out[1] == 15);
}

// ---------------------------------------------------------------------------
// 3. Addition and subtraction
// ---------------------------------------------------------------------------
void testAdditionAndSubtraction() {
    std::string src = "let x = 20 + 10 - 5; print(x);";
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 25);
}

// ---------------------------------------------------------------------------
// 4. Multiplication and operator precedence
// ---------------------------------------------------------------------------
void testPrecedenceMultiplication() {
    // 2 + 3 * 4 should be 14 (not 20)
    std::string src = "let x = 2 + 3 * 4; print(x);";
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 14);

    // Parentheses override: (2 + 3) * 4 should be 20
    std::string src2 = "let y = (2 + 3) * 4; print(y);";
    auto out2 = runProgram(src2);
    assert(out2.size() == 1);
    assert(out2[0] == 20);
}

// ---------------------------------------------------------------------------
// 5. Nested arithmetic expressions
// ---------------------------------------------------------------------------
void testNestedArithmetic() {
    std::string src = "let a = 10; let b = 2; let c = (a * 3 + (b + 4) * 2) - 10; print(c);";
    // a * 3 = 30
    // (b + 4) = 6; 6 * 2 = 12
    // 30 + 12 = 42; 42 - 10 = 32
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 32);
}

// ---------------------------------------------------------------------------
// 6. Division and division-by-zero handling
// ---------------------------------------------------------------------------
void testDivisionAndZeroHandling() {
    // Normal division
    std::string src = "let x = 40 / 5; print(x);";
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 8);

    // Division by zero
    std::string srcZero = "let x = 10 / 0; print(x);";
    bool caughtDivZero = false;
    try {
        runProgram(srcZero);
    } catch (const RuntimeError& e) {
        caughtDivZero = true;
        std::string msg = e.what();
        assert(msg.find("Division by zero") != std::string::npos);
    }
    assert(caughtDivZero);
}

// ---------------------------------------------------------------------------
// 7. Comparisons supported by the language
// ---------------------------------------------------------------------------
void testComparisons() {
    // Test == and !=
    std::string src1 = "if (5 == 5) { print(1); } else { print(0); }";
    auto out1 = runProgram(src1);
    assert(out1 == std::vector<int>({1}));

    std::string src2 = "if (5 != 5) { print(1); } else { print(0); }";
    auto out2 = runProgram(src2);
    assert(out2 == std::vector<int>({0}));

    // Test <, <=, >, >=
    std::string src3 = "if (3 < 5) { print(10); }";
    assert(runProgram(src3) == std::vector<int>({10}));

    std::string src4 = "if (5 <= 5) { print(20); }";
    assert(runProgram(src4) == std::vector<int>({20}));

    std::string src5 = "if (6 > 5) { print(30); }";
    assert(runProgram(src5) == std::vector<int>({30}));

    std::string src6 = "if (5 >= 5) { print(40); }";
    assert(runProgram(src6) == std::vector<int>({40}));
}

// ---------------------------------------------------------------------------
// 8. If/else branch behavior
// ---------------------------------------------------------------------------
void testIfElseBranching() {
    std::string srcThen =
        "let x = 15;\n"
        "if (x > 10) {\n"
        "    print(1);\n"
        "} else {\n"
        "    print(2);\n"
        "}\n";
    assert(runProgram(srcThen) == std::vector<int>({1}));

    std::string srcElse =
        "let x = 5;\n"
        "if (x > 10) {\n"
        "    print(1);\n"
        "} else {\n"
        "    print(2);\n"
        "}\n";
    assert(runProgram(srcElse) == std::vector<int>({2}));
}

// ---------------------------------------------------------------------------
// 9. While loop behavior
// ---------------------------------------------------------------------------
void testWhileLoop() {
    std::string src =
        "let i = 0;\n"
        "let sum = 0;\n"
        "while (i < 5) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "print(sum);\n";
    // 0 + 1 + 2 + 3 + 4 = 10
    auto out = runProgram(src);
    assert(out.size() == 1);
    assert(out[0] == 10);
}

// ---------------------------------------------------------------------------
// 10. Multiple print statements and output order
// ---------------------------------------------------------------------------
void testMultiplePrints() {
    std::string src =
        "print(100);\n"
        "print(200);\n"
        "print(300);\n";
    auto out = runProgram(src);
    assert(out == (std::vector<int>{100, 200, 300}));
}

// ---------------------------------------------------------------------------
// 11. Temporary-variable creation and deterministic IR generation
// ---------------------------------------------------------------------------
void testDeterministicIRGeneration() {
    std::string src =
        "let a = 10;\n"
        "let b = 20;\n"
        "let c = a + b * 2;\n"
        "print(c);\n";

    IRProgram ir1 = compileToIR(src);
    IRProgram ir2 = compileToIR(src);

    std::string str1 = ir1.toString();
    std::string str2 = ir2.toString();

    // Verify determinism
    assert(str1 == str2);

    // Verify instructions contain temporaries in expected precedence order
    // b * 2 must happen before addition
    assert(str1.find("MUL") < str1.find("ADD"));
    assert(str1.find("t0") != std::string::npos);
    assert(str1.find("t1") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 12. Invalid jump targets or equivalent defensive checks
// ---------------------------------------------------------------------------
void testInvalidJumpTarget() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeJump("NON_EXISTENT_LABEL"));

    IRInterpreter interp;
    bool caught = false;
    try {
        interp.execute(prog);
    } catch (const RuntimeError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Invalid jump target") != std::string::npos);
    }
    assert(caught);
}

// ---------------------------------------------------------------------------
// 13. Use of an uninitialized variable
// ---------------------------------------------------------------------------
void testUninitializedVariable() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makePrint("uninitialized_var"));

    IRInterpreter interp;
    bool caught = false;
    try {
        interp.execute(prog);
    } catch (const RuntimeError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("uninitialized") != std::string::npos);
    }
    assert(caught);
}

// ---------------------------------------------------------------------------
// 14. Execution-limit handling for infinite loop
// ---------------------------------------------------------------------------
void testExecutionLimit() {
    // Deliberate infinite loop: while (1 == 1) {}
    std::string src =
        "let x = 1;\n"
        "while (x == 1) {\n"
        "    // infinite loop\n"
        "}\n";

    bool caught = false;
    try {
        runProgram(src, 1000); // 1,000 steps limit
    } catch (const RuntimeError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("limit exceeded") != std::string::npos);
    }
    assert(caught);
}

// ---------------------------------------------------------------------------
// 15. Complete Language Example (as specified in plan.md)
// ---------------------------------------------------------------------------
void testFullPlanExample() {
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

    auto out = runProgram(src);
    std::vector<int> expected = {14, 0, 1, 2};
    assert(out == expected);
}

int main() {
    std::cout << "=== Mini Compiler IR & Interpreter Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testConstantAssignmentAndPrint);
    RUN_TEST(testVariableToVariableAssignment);
    RUN_TEST(testAdditionAndSubtraction);
    RUN_TEST(testPrecedenceMultiplication);
    RUN_TEST(testNestedArithmetic);
    RUN_TEST(testDivisionAndZeroHandling);
    RUN_TEST(testComparisons);
    RUN_TEST(testIfElseBranching);
    RUN_TEST(testWhileLoop);
    RUN_TEST(testMultiplePrints);
    RUN_TEST(testDeterministicIRGeneration);
    RUN_TEST(testInvalidJumpTarget);
    RUN_TEST(testUninitializedVariable);
    RUN_TEST(testExecutionLimit);
    RUN_TEST(testFullPlanExample);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    if (testsPassed == testsRun) {
        std::cout << "All IR & Interpreter tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}

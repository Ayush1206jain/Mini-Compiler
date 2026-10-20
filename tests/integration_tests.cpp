// ============================================================================
// integration_tests.cpp — End-to-End Compiler Pipeline & Equivalence Tests
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
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

// ---------------------------------------------------------------------------
// 1. Arithmetic and Operator Precedence
// ---------------------------------------------------------------------------
void testArithmeticAndPrecedence() {
    std::string src =
        "let x = 2 + 3 * 4;\n"
        "let y = (2 + 3) * 4;\n"
        "let z = 100 - 20 / 4;\n"
        "print(x);\n"
        "print(y);\n"
        "print(z);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.outputsMatch);
    assert(report.categoriesMatch);
    assert(report.unoptimized.output == (std::vector<int>{14, 20, 95}));
    assert(report.optimized.output == (std::vector<int>{14, 20, 95}));
    assert(report.pseudoAssemblyGenerated);
}

// ---------------------------------------------------------------------------
// 2. Variable Reassignment
// ---------------------------------------------------------------------------
void testVariableReassignment() {
    std::string src =
        "let x = 10;\n"
        "x = x + 5;\n"
        "x = x * 2;\n"
        "print(x);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({30}));
    assert(report.optimized.output == std::vector<int>({30}));
}

// ---------------------------------------------------------------------------
// 3. If/Else True Branch
// ---------------------------------------------------------------------------
void testIfElseTrueBranch() {
    std::string src =
        "let cond = 1;\n"
        "if (cond == 1) {\n"
        "    print(42);\n"
        "} else {\n"
        "    print(0);\n"
        "}\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({42}));
    assert(report.optimized.output == std::vector<int>({42}));
}

// ---------------------------------------------------------------------------
// 4. If/Else False Branch
// ---------------------------------------------------------------------------
void testIfElseFalseBranch() {
    std::string src =
        "let cond = 0;\n"
        "if (cond == 1) {\n"
        "    print(42);\n"
        "} else {\n"
        "    print(99);\n"
        "}\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({99}));
    assert(report.optimized.output == std::vector<int>({99}));
}

// ---------------------------------------------------------------------------
// 5. While Loop with Zero Iterations
// ---------------------------------------------------------------------------
void testWhileLoopZeroIterations() {
    std::string src =
        "let i = 10;\n"
        "while (i < 5) {\n"
        "    print(i);\n"
        "    i = i + 1;\n"
        "}\n"
        "print(i);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({10}));
    assert(report.optimized.output == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 6. While Loop with One Iteration
// ---------------------------------------------------------------------------
void testWhileLoopOneIteration() {
    std::string src =
        "let i = 0;\n"
        "while (i < 1) {\n"
        "    print(111);\n"
        "    i = i + 1;\n"
        "}\n"
        "print(i);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == (std::vector<int>{111, 1}));
    assert(report.optimized.output == (std::vector<int>{111, 1}));
}

// ---------------------------------------------------------------------------
// 7. While Loop with Multiple Iterations
// ---------------------------------------------------------------------------
void testWhileLoopMultipleIterations() {
    std::string src =
        "let i = 1;\n"
        "let product = 1;\n"
        "while (i <= 4) {\n"
        "    product = product * i;\n"
        "    i = i + 1;\n"
        "}\n"
        "print(product);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({24}));
    assert(report.optimized.output == std::vector<int>({24}));
}

// ---------------------------------------------------------------------------
// 8. Nested Control Flow (If inside While)
// ---------------------------------------------------------------------------
void testNestedControlFlow() {
    std::string src =
        "let i = 0;\n"
        "while (i < 4) {\n"
        "    if (i == 2) {\n"
        "        print(100);\n"
        "    } else {\n"
        "        print(i);\n"
        "    }\n"
        "    i = i + 1;\n"
        "}\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == (std::vector<int>{0, 1, 100, 3}));
    assert(report.optimized.output == (std::vector<int>{0, 1, 100, 3}));
}

// ---------------------------------------------------------------------------
// 9. Runtime Error Fidelity: Division by Zero
// ---------------------------------------------------------------------------
void testDivisionByZeroFidelity() {
    std::string src =
        "let x = 10 / 0;\n"
        "print(x);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.categoriesMatch);
    assert(report.unoptimized.category == ExecutionCategory::DIVISION_BY_ZERO);
    assert(report.optimized.category == ExecutionCategory::DIVISION_BY_ZERO);
}

// ---------------------------------------------------------------------------
// 10. Runtime Error Fidelity: Uninitialized Variable
// ---------------------------------------------------------------------------
void testUninitializedVariableFidelity() {
    // In MiniLang grammar, let x = y; where y is undeclared throws ParserError (variable not declared)
    // or uninitialized variable error in interpreter.
    // Let's test a program that fails parsing on undeclared variable:
    std::string src =
        "let x = y + 1;\n"
        "print(x);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.categoriesMatch);
    assert(report.unoptimized.category == ExecutionCategory::UNINITIALIZED_VARIABLE);
    assert(report.optimized.category == ExecutionCategory::UNINITIALIZED_VARIABLE);
}

// ---------------------------------------------------------------------------
// 11. Runtime Error Fidelity: Execution Step Limit Exceeded
// ---------------------------------------------------------------------------
void testExecutionStepLimitExceeded() {
    std::string src =
        "let i = 0;\n"
        "while (i == 0) {\n"
        "    // Infinite loop\n"
        "}\n";

    // Run with low step limit (e.g., 500 steps)
    ValidationReport report = PipelineValidator::validate(src, 500);
    assert(report.passed);
    assert(report.categoriesMatch);
    assert(report.unoptimized.category == ExecutionCategory::STEP_LIMIT_EXCEEDED);
    assert(report.optimized.category == ExecutionCategory::STEP_LIMIT_EXCEEDED);
}

// ---------------------------------------------------------------------------
// 12. Lexer Error Handling
// ---------------------------------------------------------------------------
void testLexerErrorHandling() {
    std::string src = "let x = @invalid;\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.categoriesMatch);
    assert(report.unoptimized.category == ExecutionCategory::LEXER_ERROR);
    assert(report.optimized.category == ExecutionCategory::LEXER_ERROR);
}

// ---------------------------------------------------------------------------
// 13. Parser Error Handling
// ---------------------------------------------------------------------------
void testParserErrorHandling() {
    std::string src = "let x = 10\n"; // Missing semicolon

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.categoriesMatch);
    assert(report.unoptimized.category == ExecutionCategory::PARSER_ERROR);
    assert(report.optimized.category == ExecutionCategory::PARSER_ERROR);
}

// ---------------------------------------------------------------------------
// 14. Full Pipeline with Optimization & Dead Code
// ---------------------------------------------------------------------------
void testFullPipelineWithDeadCode() {
    std::string src =
        "let a = 10;\n"
        "let b = 20;\n"
        "let dead = a * b * 5;\n"
        "let c = a + b;\n"
        "print(c);\n";

    ValidationReport report = PipelineValidator::validate(src);
    assert(report.passed);
    assert(report.unoptimized.output == std::vector<int>({30}));
    assert(report.optimized.output == std::vector<int>({30}));
    assert(report.instructionStats.dceCount < report.instructionStats.originalCount);
    assert(report.pseudoAssemblyGenerated);
}

int main() {
    std::cout << "=== Mini Compiler End-to-End Integration & Validation Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testArithmeticAndPrecedence);
    RUN_TEST(testVariableReassignment);
    RUN_TEST(testIfElseTrueBranch);
    RUN_TEST(testIfElseFalseBranch);
    RUN_TEST(testWhileLoopZeroIterations);
    RUN_TEST(testWhileLoopOneIteration);
    RUN_TEST(testWhileLoopMultipleIterations);
    RUN_TEST(testNestedControlFlow);
    RUN_TEST(testDivisionByZeroFidelity);
    RUN_TEST(testUninitializedVariableFidelity);
    RUN_TEST(testExecutionStepLimitExceeded);
    RUN_TEST(testLexerErrorHandling);
    RUN_TEST(testParserErrorHandling);
    RUN_TEST(testFullPipelineWithDeadCode);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

// ============================================================================
// dead_code_elimination_tests.cpp — Automated Unit Tests for Day 5 DCE Pass
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
#include "dead_code_elimination.hpp"

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

// ---------------------------------------------------------------------------
// 1. Removing an unused constant-producing temporary
// ---------------------------------------------------------------------------
void testRemoveUnusedConstantTemporary() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 42)); // Unused
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 99)); // Used
    prog.addInstruction(IRInstruction::makePrint("t1"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.instructions.size() == 2);
    assert(opt.instructions[0].opcode == IROpcode::LOAD_CONST);
    assert(opt.instructions[0].dest == "t1");
    assert(opt.instructions[1].opcode == IROpcode::PRINT);
    assert(dce.getStats().eliminatedInstructions == 1);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({99}));
}

// ---------------------------------------------------------------------------
// 2. Removing an unused arithmetic result
// ---------------------------------------------------------------------------
void testRemoveUnusedArithmeticResult() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 10));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 20));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t2", "t0", "t1")); // Unused
    prog.addInstruction(IRInstruction::makePrint("t0"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    // t2 is dead. Once t2 is removed, t1 is dead as well.
    // Result should only contain LOAD_CONST t0, 10 and PRINT t0
    assert(opt.toString().find("ADD") == std::string::npos);
    assert(opt.toString().find("t1") == std::string::npos);
    assert(opt.instructions.size() == 2);
    assert(opt.instructions[0].dest == "t0");
    assert(opt.instructions[1].opcode == IROpcode::PRINT);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 3. Preserving computations whose results are used
// ---------------------------------------------------------------------------
void testPreserveUsedComputation() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 10));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 20));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t2", "t0", "t1"));
    prog.addInstruction(IRInstruction::makePrint("t2"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.instructions.size() == 4);
    assert(dce.getStats().eliminatedInstructions == 0);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({30}));
}

// ---------------------------------------------------------------------------
// 4. Preserving chained dependencies
// ---------------------------------------------------------------------------
void testPreserveChainedDependencies() {
    // t0 = 2, t1 = 3, t2 = t0 * t1 (6), t3 = 4, t4 = t2 + t3 (10), print(t4)
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 2));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 3));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::MUL, "t2", "t0", "t1"));
    prog.addInstruction(IRInstruction::makeLoadConst("t3", 4));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t4", "t2", "t3"));
    prog.addInstruction(IRInstruction::makePrint("t4"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.instructions.size() == 6);
    assert(dce.getStats().eliminatedInstructions == 0);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 5. Preserving all observable PRINT instructions
// ---------------------------------------------------------------------------
void testPreserveAllPrintInstructions() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 1));
    prog.addInstruction(IRInstruction::makePrint("t0"));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 2));
    prog.addInstruction(IRInstruction::makePrint("t1"));
    prog.addInstruction(IRInstruction::makeLoadConst("dead", 999)); // dead

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.instructions.size() == 4);
    assert(opt.toString().find("dead") == std::string::npos);

    IRInterpreter interp;
    assert(interp.execute(opt) == (std::vector<int>{1, 2}));
}

// ---------------------------------------------------------------------------
// 6. Preserving source-variable assignments when their values are later used
// ---------------------------------------------------------------------------
void testPreserveSourceVariableUsedLater() {
    std::string src =
        "let x = 100;\n"
        "let y = 200;\n"
        "let z = x + y;\n"
        "print(z);\n";

    IRProgram orig = compileToIR(src);
    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(orig);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(opt));
    assert(interp2.execute(opt) == std::vector<int>({300}));
    assert(interp2.hasVariable("x"));
    assert(interp2.hasVariable("y"));
    assert(interp2.hasVariable("z"));
}

// ---------------------------------------------------------------------------
// 7. Preserving labels and jump targets
// ---------------------------------------------------------------------------
void testPreserveLabelsAndJumps() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 1));
    prog.addInstruction(IRInstruction::makeJump("L_target"));
    prog.addInstruction(IRInstruction::makeLoadConst("dead", 50)); // dead
    prog.addInstruction(IRInstruction::makeLabel("L_target"));
    prog.addInstruction(IRInstruction::makePrint("t0"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.toString().find("L_target") != std::string::npos);
    assert(opt.toString().find("JUMP L_target") != std::string::npos);
    assert(opt.toString().find("LABEL L_target") != std::string::npos);
    assert(opt.toString().find("dead") == std::string::npos);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({1}));
}

// ---------------------------------------------------------------------------
// 8. Preserving values used across conditional branches
// ---------------------------------------------------------------------------
void testPreserveValuesAcrossBranches() {
    std::string src =
        "let x = 10;\n"
        "let y = 20;\n"
        "let cond = 1;\n"
        "if (cond == 1) {\n"
        "    print(x);\n"
        "} else {\n"
        "    print(y);\n"
        "}\n";

    IRProgram orig = compileToIR(src);
    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(orig);

    // Both x and y must be preserved because both branches could be taken
    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(opt));
    assert(interp2.execute(opt) == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 9. Preserving loop-carried values
// ---------------------------------------------------------------------------
void testPreserveLoopCarriedValues() {
    std::string src =
        "let i = 0;\n"
        "let sum = 0;\n"
        "while (i < 5) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "print(sum);\n";

    IRProgram orig = compileToIR(src);
    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(orig);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(opt));
    assert(interp2.execute(opt) == std::vector<int>({10}));
}

// ---------------------------------------------------------------------------
// 10. Preserving potentially trapping operations (Division by Zero)
// ---------------------------------------------------------------------------
void testPreservePotentiallyTrappingDivision() {
    // Under language semantics, division by zero raises an observable runtime error.
    // An unused DIV must NOT be eliminated by DCE.
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 10));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 0));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::DIV, "t2", "t0", "t1")); // Unused result!
    prog.addInstruction(IRInstruction::makeLoadConst("t3", 42));
    prog.addInstruction(IRInstruction::makePrint("t3"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    // DIV must still be present
    assert(opt.toString().find("DIV") != std::string::npos);

    // When executed, the interpreter MUST raise Division by zero error
    IRInterpreter interp;
    bool caught = false;
    try {
        interp.execute(opt);
    } catch (const RuntimeError& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Division by zero") != std::string::npos);
    }
    assert(caught);
}

// ---------------------------------------------------------------------------
// 11. Repeating DCE when one removal makes another computation dead (Cascading)
// ---------------------------------------------------------------------------
void testCascadingDeadCodeElimination() {
    // Chain: t0 -> t1 -> t2 -> t3 (none of them printed or used)
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("t0", 5));
    prog.addInstruction(IRInstruction::makeLoadConst("t1", 10));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t2", "t0", "t1"));
    prog.addInstruction(IRInstruction::makeLoadConst("k", 2));
    prog.addInstruction(IRInstruction::makeBinary(IROpcode::MUL, "t3", "t2", "k"));
    prog.addInstruction(IRInstruction::makeLoadConst("alive", 777));
    prog.addInstruction(IRInstruction::makePrint("alive"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    // All dead chain instructions should be gone
    assert(opt.toString().find("t0") == std::string::npos);
    assert(opt.toString().find("t1") == std::string::npos);
    assert(opt.toString().find("t2") == std::string::npos);
    assert(opt.toString().find("t3") == std::string::npos);
    assert(opt.toString().find("k") == std::string::npos);

    // Only "alive" and print remain
    assert(opt.instructions.size() == 2);
    assert(opt.instructions[0].dest == "alive");
    assert(opt.instructions[1].opcode == IROpcode::PRINT);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({777}));
}

// ---------------------------------------------------------------------------
// 12. Dead store elimination (overwriting variable value before use)
// ---------------------------------------------------------------------------
void testDeadStoreElimination() {
    IRProgram prog;
    prog.addInstruction(IRInstruction::makeLoadConst("x", 10)); // Overwritten before read
    prog.addInstruction(IRInstruction::makeLoadConst("x", 20)); // Used
    prog.addInstruction(IRInstruction::makePrint("x"));

    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(prog);

    assert(opt.instructions.size() == 2);
    assert(opt.instructions[0].constValue.value_or(0) == 20);
    assert(opt.instructions[1].opcode == IROpcode::PRINT);

    IRInterpreter interp;
    assert(interp.execute(opt) == std::vector<int>({20}));
}

// ---------------------------------------------------------------------------
// 13. Completely unused declared variable eliminated
// ---------------------------------------------------------------------------
void testCompletelyUnusedVariableEliminated() {
    std::string src =
        "let unused = 12345;\n"
        "let used = 42;\n"
        "print(used);\n";

    IRProgram orig = compileToIR(src);
    DeadCodeElimination dce;
    IRProgram opt = dce.eliminate(orig);

    assert(orig.toString().find("unused") != std::string::npos);
    assert(opt.toString().find("unused") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(opt));
    assert(interp2.execute(opt) == std::vector<int>({42}));
}

// ---------------------------------------------------------------------------
// 14. Full pipeline: Constant Folding followed by DCE
// ---------------------------------------------------------------------------
void testFullPipelineFoldingThenDCE() {
    std::string src =
        "let x = 2 + 3 * 4;\n"
        "let dead = 100 * 2;\n"
        "print(x);\n";

    IRProgram orig = compileToIR(src);

    // Pass 1: Constant folding & propagation
    IROptimizer optimizer;
    IRProgram folded = optimizer.optimize(orig);

    // Pass 2: Dead-code elimination
    DeadCodeElimination dce;
    IRProgram finalOpt = dce.eliminate(folded);

    // Intermediate temporaries and 'dead' must be gone
    assert(finalOpt.toString().find("dead") == std::string::npos);
    assert(finalOpt.toString().find("ADD") == std::string::npos);
    assert(finalOpt.toString().find("MUL") == std::string::npos);

    IRInterpreter interp1, interp2;
    assert(interp1.execute(orig) == interp2.execute(finalOpt));
    assert(interp2.execute(finalOpt) == std::vector<int>({14}));
}

int main() {
    std::cout << "=== Mini Compiler Dead-Code Elimination (DCE) Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testRemoveUnusedConstantTemporary);
    RUN_TEST(testRemoveUnusedArithmeticResult);
    RUN_TEST(testPreserveUsedComputation);
    RUN_TEST(testPreserveChainedDependencies);
    RUN_TEST(testPreserveAllPrintInstructions);
    RUN_TEST(testPreserveSourceVariableUsedLater);
    RUN_TEST(testPreserveLabelsAndJumps);
    RUN_TEST(testPreserveValuesAcrossBranches);
    RUN_TEST(testPreserveLoopCarriedValues);
    RUN_TEST(testPreservePotentiallyTrappingDivision);
    RUN_TEST(testCascadingDeadCodeElimination);
    RUN_TEST(testDeadStoreElimination);
    RUN_TEST(testCompletelyUnusedVariableEliminated);
    RUN_TEST(testFullPipelineFoldingThenDCE);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

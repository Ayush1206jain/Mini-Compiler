// ============================================================================
// pseudo_assembly_tests.cpp — Automated Unit Tests for Day 5 Pseudo-Assembly
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
#include "ir_optimizer.hpp"
#include "dead_code_elimination.hpp"
#include "pseudo_assembly_generator.hpp"

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
// 1. Constant Loading (MOVI)
// ---------------------------------------------------------------------------
void testConstantLoading() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeLoadConst("x", 42));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 1);
    assert(pasm.instructions[0].opcode == "MOVI");
    assert(pasm.instructions[0].args == (std::vector<std::string>{"x", "42"}));
    assert(pasm.toString().find("MOVI    x, 42") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 2. Variable Moves (MOV)
// ---------------------------------------------------------------------------
void testVariableMove() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeMove("y", "x"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 1);
    assert(pasm.instructions[0].opcode == "MOV");
    assert(pasm.instructions[0].args == (std::vector<std::string>{"y", "x"}));
    assert(pasm.toString().find("MOV     y, x") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 3. Arithmetic Instructions (ADD, SUB, MUL, DIV)
// ---------------------------------------------------------------------------
void testArithmeticInstructions() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "t0", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::SUB, "t1", "c", "d"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::MUL, "t2", "e", "f"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::DIV, "t3", "g", "h"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 4);
    assert(pasm.instructions[0].opcode == "ADD");
    assert(pasm.instructions[1].opcode == "SUB");
    assert(pasm.instructions[2].opcode == "MUL");
    assert(pasm.instructions[3].opcode == "DIV");

    std::string text = pasm.toString();
    assert(text.find("ADD     t0, a, b") != std::string::npos);
    assert(text.find("SUB     t1, c, d") != std::string::npos);
    assert(text.find("MUL     t2, e, f") != std::string::npos);
    assert(text.find("DIV     t3, g, h") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 4. Comparison Instructions (CMPEQ, CMPNE, CMPLT, CMPLE, CMPGT, CMPGE)
// ---------------------------------------------------------------------------
void testComparisonInstructions() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_EQ, "t0", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_NE, "t1", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_LT, "t2", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_LE, "t3", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_GT, "t4", "a", "b"));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::CMP_GE, "t5", "a", "b"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 6);
    assert(pasm.instructions[0].opcode == "CMPEQ");
    assert(pasm.instructions[1].opcode == "CMPNE");
    assert(pasm.instructions[2].opcode == "CMPLT");
    assert(pasm.instructions[3].opcode == "CMPLE");
    assert(pasm.instructions[4].opcode == "CMPGT");
    assert(pasm.instructions[5].opcode == "CMPGE");
}

// ---------------------------------------------------------------------------
// 5. Print Instructions (PRINT)
// ---------------------------------------------------------------------------
void testPrintInstruction() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makePrint("result"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 1);
    assert(pasm.instructions[0].opcode == "PRINT");
    assert(pasm.instructions[0].args == (std::vector<std::string>{"result"}));
    assert(pasm.toString().find("PRINT   result") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 6. Labels and Unconditional Jumps (label:, JMP)
// ---------------------------------------------------------------------------
void testLabelsAndUnconditionalJumps() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeLabel("L_loop"));
    ir.addInstruction(IRInstruction::makeJump("L_loop"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 2);
    assert(pasm.instructions[0].isLabel);
    assert(pasm.instructions[0].labelName == "L_loop");
    assert(pasm.instructions[0].toString() == "L_loop:");
    assert(pasm.instructions[1].opcode == "JMP");
    assert(pasm.instructions[1].args == (std::vector<std::string>{"L_loop"}));
}

// ---------------------------------------------------------------------------
// 7. Conditional Jumps (JMPF)
// ---------------------------------------------------------------------------
void testConditionalJumps() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeJumpIfFalse("cond0", "L_else"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 1);
    assert(pasm.instructions[0].opcode == "JMPF");
    assert(pasm.instructions[0].args == (std::vector<std::string>{"cond0", "L_else"}));
    assert(pasm.toString().find("JMPF    cond0, L_else") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 8. Correct Ordering of Generated Instructions
// ---------------------------------------------------------------------------
void testInstructionOrdering() {
    IRProgram ir;
    ir.addInstruction(IRInstruction::makeLoadConst("a", 10));
    ir.addInstruction(IRInstruction::makeLoadConst("b", 20));
    ir.addInstruction(IRInstruction::makeBinary(IROpcode::ADD, "c", "a", "b"));
    ir.addInstruction(IRInstruction::makePrint("c"));

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(ir);

    assert(pasm.instructions.size() == 4);
    assert(pasm.instructions[0].opcode == "MOVI");
    assert(pasm.instructions[1].opcode == "MOVI");
    assert(pasm.instructions[2].opcode == "ADD");
    assert(pasm.instructions[3].opcode == "PRINT");
}

// ---------------------------------------------------------------------------
// 9. Meaningful Handling of Unsupported Instructions
// ---------------------------------------------------------------------------
void testUnsupportedInstructionHandling() {
    IRInstruction badInst;
    badInst.opcode = static_cast<IROpcode>(9999); // Invalid opcode

    IRProgram ir;
    ir.addInstruction(badInst);

    PseudoAssemblyGenerator gen;
    bool caught = false;
    try {
        gen.generate(ir);
    } catch (const std::runtime_error& e) {
        caught = true;
        std::string msg = e.what();
        assert(msg.find("Unsupported") != std::string::npos || msg.find("UNKNOWN") != std::string::npos);
    }
    assert(caught);
}

// ---------------------------------------------------------------------------
// 10. End-to-End Program Pseudo-Assembly Generation
// ---------------------------------------------------------------------------
void testEndToEndPseudoAssembly() {
    std::string src =
        "let x = 10;\n"
        "let y = 20;\n"
        "let total = x + y;\n"
        "if (total > 25) {\n"
        "    print(total);\n"
        "} else {\n"
        "    print(0);\n"
        "}\n";

    IRProgram orig = compileToIR(src);
    IROptimizer opt;
    IRProgram folded = opt.optimize(orig);
    DeadCodeElimination dce;
    IRProgram optimized = dce.eliminate(folded);

    PseudoAssemblyGenerator gen;
    PseudoAssemblyProgram pasm = gen.generate(optimized);

    std::string text = pasm.toString();
    assert(text.find("PRINT") != std::string::npos);
    assert(!text.empty());
}

int main() {
    std::cout << "=== Mini Compiler Pseudo-Assembly Generator Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testConstantLoading);
    RUN_TEST(testVariableMove);
    RUN_TEST(testArithmeticInstructions);
    RUN_TEST(testComparisonInstructions);
    RUN_TEST(testPrintInstruction);
    RUN_TEST(testLabelsAndUnconditionalJumps);
    RUN_TEST(testConditionalJumps);
    RUN_TEST(testInstructionOrdering);
    RUN_TEST(testUnsupportedInstructionHandling);
    RUN_TEST(testEndToEndPseudoAssembly);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

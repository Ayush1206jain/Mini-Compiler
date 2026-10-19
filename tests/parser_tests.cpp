// ============================================================================
// parser_tests.cpp — Automated tests for Parser and AST construction
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"

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

static std::unique_ptr<ProgramNode> parseSource(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parseProgram();
}

void testPrecedenceInAST() {
    auto prog = parseSource("let x = 2 + 3 * 4;");
    assert(prog->statements.size() == 1);
    auto letStmt = static_cast<LetStmtNode*>(prog->statements[0].get());
    assert(letStmt->initializer->getType() == ASTNodeType::BINARY_EXPR);
    auto binExpr = static_cast<BinaryExprNode*>(letStmt->initializer.get());
    assert(binExpr->op == TokenType::PLUS);
    assert(binExpr->left->getType() == ASTNodeType::INTEGER_LITERAL);
    assert(binExpr->right->getType() == ASTNodeType::BINARY_EXPR);
    auto multExpr = static_cast<BinaryExprNode*>(binExpr->right.get());
    assert(multExpr->op == TokenType::STAR);
}

void testIfElseParsing() {
    auto prog = parseSource("if (x > 10) { print(x); } else { print(0); }");
    assert(prog->statements.size() == 1);
    auto ifStmt = static_cast<IfStmtNode*>(prog->statements[0].get());
    assert(ifStmt->condition->op == TokenType::GREATER);
    assert(ifStmt->thenBranch->statements.size() == 1);
    assert(ifStmt->elseBranch != nullptr);
    assert(ifStmt->elseBranch->statements.size() == 1);
}

void testWhileParsing() {
    auto prog = parseSource("while (i < 3) { print(i); i = i + 1; }");
    assert(prog->statements.size() == 1);
    auto whileStmt = static_cast<WhileStmtNode*>(prog->statements[0].get());
    assert(whileStmt->condition->op == TokenType::LESS);
    assert(whileStmt->body->statements.size() == 2);
}

void testSyntaxErrorMissingSemicolon() {
    bool caught = false;
    try {
        parseSource("let x = 10");
    } catch (const ParserError& e) {
        caught = true;
    }
    assert(caught);
}

void testSyntaxErrorUnmatchedBrace() {
    bool caught = false;
    try {
        parseSource("if (x > 0) { print(x);");
    } catch (const ParserError& e) {
        caught = true;
    }
    assert(caught);
}

int main() {
    std::cout << "=== Mini Compiler Parser Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testPrecedenceInAST);
    RUN_TEST(testIfElseParsing);
    RUN_TEST(testWhileParsing);
    RUN_TEST(testSyntaxErrorMissingSemicolon);
    RUN_TEST(testSyntaxErrorUnmatchedBrace);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}

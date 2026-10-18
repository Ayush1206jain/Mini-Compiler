// ============================================================================
// lexer_tests.cpp — Automated tests for the Mini Compiler Lexer
// ============================================================================
//
// This file uses simple assertions to test the lexer. No external test
// framework is required — just standard C++ with <cassert>.
//
// Each test function:
//   1. Creates a Lexer with a specific input string.
//   2. Calls tokenize() to get the tokens.
//   3. Checks that the tokens have the expected types, lexemes, and positions.
//
// If any assertion fails, the program terminates with an error message
// showing which file and line the failure occurred on.
//
// Build and run:
//   g++ -std=c++17 -I../include ../src/lexer.cpp lexer_tests.cpp -o lexer_tests
//   ./lexer_tests
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"

// ---------------------------------------------------------------------------
// Test counter for tracking progress
// ---------------------------------------------------------------------------
static int testsRun = 0;
static int testsPassed = 0;

// Macro that prints the test name and runs it
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
// Test 1: Empty input
// ---------------------------------------------------------------------------
// An empty string should produce exactly one token: END_OF_FILE.
void testEmptyInput() {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    assert(tokens.size() == 1);
    assert(tokens[0].type == TokenType::END_OF_FILE);
    assert(tokens[0].line == 1);
    assert(tokens[0].column == 1);
}

// ---------------------------------------------------------------------------
// Test 2: Variable declaration — "let x = 10;"
// ---------------------------------------------------------------------------
void testVariableDeclaration() {
    Lexer lexer("let x = 10;");
    auto tokens = lexer.tokenize();

    // Expected: LET, IDENTIFIER("x"), EQUAL, INTEGER("10"), SEMICOLON, EOF
    assert(tokens.size() == 6);

    assert(tokens[0].type == TokenType::LET);
    assert(tokens[0].lexeme == "let");
    assert(tokens[0].line == 1);
    assert(tokens[0].column == 1);

    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].lexeme == "x");
    assert(tokens[1].line == 1);
    assert(tokens[1].column == 5);

    assert(tokens[2].type == TokenType::EQUAL);
    assert(tokens[2].lexeme == "=");
    assert(tokens[2].line == 1);
    assert(tokens[2].column == 7);

    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[3].lexeme == "10");
    assert(tokens[3].line == 1);
    assert(tokens[3].column == 9);

    assert(tokens[4].type == TokenType::SEMICOLON);
    assert(tokens[4].lexeme == ";");
    assert(tokens[4].line == 1);
    assert(tokens[4].column == 11);

    assert(tokens[5].type == TokenType::END_OF_FILE);
}

// ---------------------------------------------------------------------------
// Test 3: Keywords versus identifiers
// ---------------------------------------------------------------------------
// "if" is a keyword, but "iffy" is an identifier.
// "let" is a keyword, but "letter" is an identifier.
// "while" is a keyword, but "whileTrue" is an identifier.
void testKeywordsVsIdentifiers() {
    Lexer lexer("if iffy let letter while whileTrue else print");
    auto tokens = lexer.tokenize();

    // 8 words + EOF = 9 tokens
    assert(tokens.size() == 9);

    assert(tokens[0].type == TokenType::IF);
    assert(tokens[0].lexeme == "if");

    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].lexeme == "iffy");

    assert(tokens[2].type == TokenType::LET);
    assert(tokens[2].lexeme == "let");

    assert(tokens[3].type == TokenType::IDENTIFIER);
    assert(tokens[3].lexeme == "letter");

    assert(tokens[4].type == TokenType::WHILE);
    assert(tokens[4].lexeme == "while");

    assert(tokens[5].type == TokenType::IDENTIFIER);
    assert(tokens[5].lexeme == "whileTrue");

    assert(tokens[6].type == TokenType::ELSE);
    assert(tokens[6].lexeme == "else");

    assert(tokens[7].type == TokenType::PRINT);
    assert(tokens[7].lexeme == "print");
}

// ---------------------------------------------------------------------------
// Test 4: All arithmetic operators
// ---------------------------------------------------------------------------
void testArithmeticOperators() {
    Lexer lexer("+ - * /");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 5); // 4 operators + EOF

    assert(tokens[0].type == TokenType::PLUS);
    assert(tokens[0].lexeme == "+");

    assert(tokens[1].type == TokenType::MINUS);
    assert(tokens[1].lexeme == "-");

    assert(tokens[2].type == TokenType::STAR);
    assert(tokens[2].lexeme == "*");

    assert(tokens[3].type == TokenType::SLASH);
    assert(tokens[3].lexeme == "/");
}

// ---------------------------------------------------------------------------
// Test 5: All comparison operators
// ---------------------------------------------------------------------------
void testComparisonOperators() {
    Lexer lexer("== != < <= > >=");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 7); // 6 operators + EOF

    assert(tokens[0].type == TokenType::EQUAL_EQUAL);
    assert(tokens[0].lexeme == "==");

    assert(tokens[1].type == TokenType::BANG_EQUAL);
    assert(tokens[1].lexeme == "!=");

    assert(tokens[2].type == TokenType::LESS);
    assert(tokens[2].lexeme == "<");

    assert(tokens[3].type == TokenType::LESS_EQUAL);
    assert(tokens[3].lexeme == "<=");

    assert(tokens[4].type == TokenType::GREATER);
    assert(tokens[4].lexeme == ">");

    assert(tokens[5].type == TokenType::GREATER_EQUAL);
    assert(tokens[5].lexeme == ">=");
}

// ---------------------------------------------------------------------------
// Test 6: Distinguishing = from ==
// ---------------------------------------------------------------------------
// This is a critical test. "=" and "==" have very different meanings.
// "=" is assignment, "==" is comparison.
void testEqualVsEqualEqual() {
    Lexer lexer("x = 5 == 5");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 6); // IDENTIFIER, EQUAL, INTEGER, EQUAL_EQUAL, INTEGER, EOF

    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[1].type == TokenType::EQUAL);
    assert(tokens[1].lexeme == "=");
    assert(tokens[2].type == TokenType::INTEGER);
    assert(tokens[3].type == TokenType::EQUAL_EQUAL);
    assert(tokens[3].lexeme == "==");
    assert(tokens[4].type == TokenType::INTEGER);
}

// ---------------------------------------------------------------------------
// Test 7: Parentheses, braces, and semicolons
// ---------------------------------------------------------------------------
void testDelimiters() {
    Lexer lexer("( ) { } ;");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 6); // 5 delimiters + EOF

    assert(tokens[0].type == TokenType::LPAREN);
    assert(tokens[1].type == TokenType::RPAREN);
    assert(tokens[2].type == TokenType::LBRACE);
    assert(tokens[3].type == TokenType::RBRACE);
    assert(tokens[4].type == TokenType::SEMICOLON);
}

// ---------------------------------------------------------------------------
// Test 8: Whitespace and newlines
// ---------------------------------------------------------------------------
// Various whitespace should be skipped, but line/column tracking must be correct.
void testWhitespaceAndNewlines() {
    Lexer lexer("  x\n  y\n  z");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 4); // x, y, z, EOF

    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].lexeme == "x");
    assert(tokens[0].line == 1);
    assert(tokens[0].column == 3);

    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].lexeme == "y");
    assert(tokens[1].line == 2);
    assert(tokens[1].column == 3);

    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].lexeme == "z");
    assert(tokens[2].line == 3);
    assert(tokens[2].column == 3);
}

// ---------------------------------------------------------------------------
// Test 9: Correct line and column tracking with mixed tokens
// ---------------------------------------------------------------------------
void testLineColumnTracking() {
    // Line 1: "let x = 10;"  (columns 1,5,7,9,11)
    // Line 2: "print(x);"    (columns 1,6,7,8,9)
    Lexer lexer("let x = 10;\nprint(x);");
    auto tokens = lexer.tokenize();

    // Line 1: LET(1,1) IDENTIFIER(1,5) EQUAL(1,7) INTEGER(1,9) SEMICOLON(1,11)
    assert(tokens[0].line == 1); assert(tokens[0].column == 1);   // let
    assert(tokens[1].line == 1); assert(tokens[1].column == 5);   // x
    assert(tokens[2].line == 1); assert(tokens[2].column == 7);   // =
    assert(tokens[3].line == 1); assert(tokens[3].column == 9);   // 10
    assert(tokens[4].line == 1); assert(tokens[4].column == 11);  // ;

    // Line 2: PRINT(2,1) LPAREN(2,6) IDENTIFIER(2,7) RPAREN(2,8) SEMICOLON(2,9)
    assert(tokens[5].type == TokenType::PRINT);
    assert(tokens[5].line == 2); assert(tokens[5].column == 1);   // print
    assert(tokens[6].type == TokenType::LPAREN);
    assert(tokens[6].line == 2); assert(tokens[6].column == 6);   // (
    assert(tokens[7].type == TokenType::IDENTIFIER);
    assert(tokens[7].line == 2); assert(tokens[7].column == 7);   // x
    assert(tokens[8].type == TokenType::RPAREN);
    assert(tokens[8].line == 2); assert(tokens[8].column == 8);   // )
    assert(tokens[9].type == TokenType::SEMICOLON);
    assert(tokens[9].line == 2); assert(tokens[9].column == 9);   // ;
}

// ---------------------------------------------------------------------------
// Test 10: Unsupported characters
// ---------------------------------------------------------------------------
// The lexer should throw a LexerError for characters it doesn't recognize.
void testUnsupportedCharacter() {
    bool caughtError = false;
    try {
        Lexer lexer("let x = @;");
        lexer.tokenize();
    } catch (const LexerError& e) {
        caughtError = true;
        // The error should mention the '@' character and its position
        assert(e.line == 1);
        assert(e.column == 9);
    }
    assert(caughtError);
}

// ---------------------------------------------------------------------------
// Test 11: Standalone '!' without '=' should be an error
// ---------------------------------------------------------------------------
void testStandaloneBang() {
    bool caughtError = false;
    try {
        Lexer lexer("x ! y");
        lexer.tokenize();
    } catch (const LexerError& e) {
        caughtError = true;
        assert(e.line == 1);
        assert(e.column == 3);
    }
    assert(caughtError);
}

// ---------------------------------------------------------------------------
// Test 12: If/else and while statements
// ---------------------------------------------------------------------------
void testControlFlowKeywords() {
    Lexer lexer("if (x > 0) { print(x); } else { while (x < 10) { x = x + 1; } }");
    auto tokens = lexer.tokenize();

    // Verify key tokens
    assert(tokens[0].type == TokenType::IF);
    assert(tokens[1].type == TokenType::LPAREN);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].lexeme == "x");
    assert(tokens[3].type == TokenType::GREATER);
    assert(tokens[4].type == TokenType::INTEGER);
    assert(tokens[4].lexeme == "0");
    assert(tokens[5].type == TokenType::RPAREN);
    assert(tokens[6].type == TokenType::LBRACE);
    assert(tokens[7].type == TokenType::PRINT);
    assert(tokens[8].type == TokenType::LPAREN);
    assert(tokens[9].type == TokenType::IDENTIFIER);
    assert(tokens[10].type == TokenType::RPAREN);
    assert(tokens[11].type == TokenType::SEMICOLON);
    assert(tokens[12].type == TokenType::RBRACE);
    assert(tokens[13].type == TokenType::ELSE);
    assert(tokens[14].type == TokenType::LBRACE);
    assert(tokens[15].type == TokenType::WHILE);
}

// ---------------------------------------------------------------------------
// Test 13: End-of-file token behavior
// ---------------------------------------------------------------------------
// Even with just whitespace, the lexer should produce exactly one EOF token.
void testEndOfFile() {
    // Only whitespace
    Lexer lexer1("   \n\t\n   ");
    auto tokens1 = lexer1.tokenize();
    assert(tokens1.size() == 1);
    assert(tokens1[0].type == TokenType::END_OF_FILE);

    // Single token + EOF
    Lexer lexer2("42");
    auto tokens2 = lexer2.tokenize();
    assert(tokens2.size() == 2);
    assert(tokens2[0].type == TokenType::INTEGER);
    assert(tokens2[1].type == TokenType::END_OF_FILE);
}

// ---------------------------------------------------------------------------
// Test 14: Single-line comments
// ---------------------------------------------------------------------------
void testSingleLineComments() {
    Lexer lexer("// this is a comment\nlet x = 5; // another comment\n// final comment");
    auto tokens = lexer.tokenize();

    // Should only see: LET, IDENTIFIER, EQUAL, INTEGER, SEMICOLON, EOF
    assert(tokens.size() == 6);
    assert(tokens[0].type == TokenType::LET);
    assert(tokens[0].line == 2); // First real token is on line 2
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[2].type == TokenType::EQUAL);
    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[4].type == TokenType::SEMICOLON);
    assert(tokens[5].type == TokenType::END_OF_FILE);
}

// ---------------------------------------------------------------------------
// Test 15: Integer literals of various lengths
// ---------------------------------------------------------------------------
void testIntegerLiterals() {
    Lexer lexer("0 1 42 100 999999");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 6); // 5 integers + EOF
    assert(tokens[0].lexeme == "0");
    assert(tokens[1].lexeme == "1");
    assert(tokens[2].lexeme == "42");
    assert(tokens[3].lexeme == "100");
    assert(tokens[4].lexeme == "999999");
}

// ---------------------------------------------------------------------------
// Test 16: Identifiers with underscores
// ---------------------------------------------------------------------------
void testIdentifiersWithUnderscores() {
    Lexer lexer("_x my_var _temp _123");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 5); // 4 identifiers + EOF
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].lexeme == "_x");
    assert(tokens[1].lexeme == "my_var");
    assert(tokens[2].lexeme == "_temp");
    assert(tokens[3].lexeme == "_123");
}

// ---------------------------------------------------------------------------
// Test 17: Operators without spaces
// ---------------------------------------------------------------------------
// The lexer should correctly separate tokens even without whitespace.
void testOperatorsWithoutSpaces() {
    Lexer lexer("x+y*z-1");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 8); // x, +, y, *, z, -, 1, EOF
    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].lexeme == "x");
    assert(tokens[1].type == TokenType::PLUS);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].lexeme == "y");
    assert(tokens[3].type == TokenType::STAR);
    assert(tokens[4].type == TokenType::IDENTIFIER);
    assert(tokens[4].lexeme == "z");
    assert(tokens[5].type == TokenType::MINUS);
    assert(tokens[6].type == TokenType::INTEGER);
    assert(tokens[6].lexeme == "1");
}

// ---------------------------------------------------------------------------
// Test 18: Complete program tokenization
// ---------------------------------------------------------------------------
void testCompleteProgram() {
    std::string source =
        "let x = 2 + 3 * 4;\n"
        "if (x > 10) {\n"
        "    print(x);\n"
        "} else {\n"
        "    print(0);\n"
        "}\n";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Verify the token stream makes sense
    // Line 1: let x = 2 + 3 * 4 ;  (9 tokens)
    assert(tokens[0].type == TokenType::LET);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].lexeme == "x");
    assert(tokens[2].type == TokenType::EQUAL);
    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[3].lexeme == "2");
    assert(tokens[4].type == TokenType::PLUS);
    assert(tokens[5].type == TokenType::INTEGER);
    assert(tokens[5].lexeme == "3");
    assert(tokens[6].type == TokenType::STAR);
    assert(tokens[7].type == TokenType::INTEGER);
    assert(tokens[7].lexeme == "4");
    assert(tokens[8].type == TokenType::SEMICOLON);

    // Line 2: if ( x > 10 ) {
    assert(tokens[9].type == TokenType::IF);
    assert(tokens[9].line == 2);
    assert(tokens[10].type == TokenType::LPAREN);
    assert(tokens[11].type == TokenType::IDENTIFIER);
    assert(tokens[11].lexeme == "x");
    assert(tokens[12].type == TokenType::GREATER);
    assert(tokens[13].type == TokenType::INTEGER);
    assert(tokens[13].lexeme == "10");
    assert(tokens[14].type == TokenType::RPAREN);
    assert(tokens[15].type == TokenType::LBRACE);

    // Last token should be EOF
    assert(tokens.back().type == TokenType::END_OF_FILE);
}

// ---------------------------------------------------------------------------
// Test 19: Two-character operators adjacent to each other
// ---------------------------------------------------------------------------
void testAdjacentTwoCharOperators() {
    Lexer lexer("x<=y>=z==w!=v");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::IDENTIFIER);
    assert(tokens[0].lexeme == "x");
    assert(tokens[1].type == TokenType::LESS_EQUAL);
    assert(tokens[2].type == TokenType::IDENTIFIER);
    assert(tokens[2].lexeme == "y");
    assert(tokens[3].type == TokenType::GREATER_EQUAL);
    assert(tokens[4].type == TokenType::IDENTIFIER);
    assert(tokens[4].lexeme == "z");
    assert(tokens[5].type == TokenType::EQUAL_EQUAL);
    assert(tokens[6].type == TokenType::IDENTIFIER);
    assert(tokens[6].lexeme == "w");
    assert(tokens[7].type == TokenType::BANG_EQUAL);
    assert(tokens[8].type == TokenType::IDENTIFIER);
    assert(tokens[8].lexeme == "v");
}

// ===========================================================================
// Main — Run all tests
// ===========================================================================
int main() {
    std::cout << "=== Mini Compiler Lexer Tests ===" << std::endl;
    std::cout << std::endl;

    RUN_TEST(testEmptyInput);
    RUN_TEST(testVariableDeclaration);
    RUN_TEST(testKeywordsVsIdentifiers);
    RUN_TEST(testArithmeticOperators);
    RUN_TEST(testComparisonOperators);
    RUN_TEST(testEqualVsEqualEqual);
    RUN_TEST(testDelimiters);
    RUN_TEST(testWhitespaceAndNewlines);
    RUN_TEST(testLineColumnTracking);
    RUN_TEST(testUnsupportedCharacter);
    RUN_TEST(testStandaloneBang);
    RUN_TEST(testControlFlowKeywords);
    RUN_TEST(testEndOfFile);
    RUN_TEST(testSingleLineComments);
    RUN_TEST(testIntegerLiterals);
    RUN_TEST(testIdentifiersWithUnderscores);
    RUN_TEST(testOperatorsWithoutSpaces);
    RUN_TEST(testCompleteProgram);
    RUN_TEST(testAdjacentTwoCharOperators);

    std::cout << std::endl;
    std::cout << "=== Results: " << testsPassed << "/" << testsRun
              << " tests passed ===" << std::endl;

    if (testsPassed == testsRun) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}

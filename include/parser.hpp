#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "token.hpp"
#include "ast.hpp"

// ============================================================================
// parser.hpp — Recursive Descent Parser for MiniLang
// ============================================================================
//
// Grammar:
//   program      → statement*
//   statement    → "let" IDENTIFIER "=" expression ";"
//                | IDENTIFIER "=" expression ";"
//                | "print" "(" expression ")" ";"
//                | "if" "(" condition ")" block ("else" block)?
//                | "while" "(" condition ")" block
//   block        → "{" statement* "}"
//   condition    → expression comparison_operator expression
//   expression   → term (("+" | "-") term)*
//   term         → unary (("*" | "/") unary)*
//   unary        → "-" unary | primary
//   primary      → INTEGER | IDENTIFIER | "(" expression ")"
//   comparison_operator → "==" | "!=" | "<" | "<=" | ">" | ">="
// ============================================================================

class ParserError : public std::runtime_error {
public:
    int line;
    int column;

    ParserError(const std::string& message, int line, int column)
        : std::runtime_error(message), line(line), column(column) {}
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<ProgramNode> parseProgram();

private:
    std::vector<Token> tokens_;
    size_t current_;

    // Helpers
    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    const Token& advance();
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& message);

    // Parsing routines
    std::unique_ptr<StmtNode> parseStatement();
    std::unique_ptr<LetStmtNode> parseLetStatement();
    std::unique_ptr<AssignStmtNode> parseAssignStatement();
    std::unique_ptr<PrintStmtNode> parsePrintStatement();
    std::unique_ptr<IfStmtNode> parseIfStatement();
    std::unique_ptr<WhileStmtNode> parseWhileStatement();
    std::unique_ptr<BlockStmtNode> parseBlock();
    std::unique_ptr<ComparisonExprNode> parseCondition();

    std::unique_ptr<ExprNode> parseExpression();
    std::unique_ptr<ExprNode> parseTerm();
    std::unique_ptr<ExprNode> parseUnary();
    std::unique_ptr<ExprNode> parsePrimary();
};

#endif // PARSER_HPP

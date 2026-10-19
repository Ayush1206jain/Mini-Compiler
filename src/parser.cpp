#include "parser.hpp"

// ============================================================================
// parser.cpp — Recursive Descent Parser Implementation
// ============================================================================

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {}

const Token& Parser::peek() const {
    if (current_ >= tokens_.size()) {
        return tokens_.back(); // EOF token
    }
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

const Token& Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    const Token& tok = peek();
    throw ParserError(message + " (found '" + tok.lexeme + "' at line " +
                      std::to_string(tok.line) + ", column " +
                      std::to_string(tok.column) + ")", tok.line, tok.column);
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();
    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

std::unique_ptr<StmtNode> Parser::parseStatement() {
    if (check(TokenType::LET)) {
        return parseLetStatement();
    }
    if (check(TokenType::PRINT)) {
        return parsePrintStatement();
    }
    if (check(TokenType::IF)) {
        return parseIfStatement();
    }
    if (check(TokenType::WHILE)) {
        return parseWhileStatement();
    }
    if (check(TokenType::IDENTIFIER)) {
        return parseAssignStatement();
    }

    const Token& tok = peek();
    throw ParserError("Expected statement at line " + std::to_string(tok.line) +
                      ", column " + std::to_string(tok.column) + " but got '" +
                      tok.lexeme + "'", tok.line, tok.column);
}

std::unique_ptr<LetStmtNode> Parser::parseLetStatement() {
    const Token& letTok = consume(TokenType::LET, "Expected 'let'");
    const Token& idTok = consume(TokenType::IDENTIFIER, "Expected identifier after 'let'");
    consume(TokenType::EQUAL, "Expected '=' after variable name in declaration");
    auto init = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<LetStmtNode>(idTok.lexeme, std::move(init), letTok.line, letTok.column);
}

std::unique_ptr<AssignStmtNode> Parser::parseAssignStatement() {
    const Token& idTok = consume(TokenType::IDENTIFIER, "Expected identifier");
    consume(TokenType::EQUAL, "Expected '=' in assignment");
    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after assignment");
    return std::make_unique<AssignStmtNode>(idTok.lexeme, std::move(value), idTok.line, idTok.column);
}

std::unique_ptr<PrintStmtNode> Parser::parsePrintStatement() {
    const Token& printTok = consume(TokenType::PRINT, "Expected 'print'");
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after print expression");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    return std::make_unique<PrintStmtNode>(std::move(expr), printTok.line, printTok.column);
}

std::unique_ptr<IfStmtNode> Parser::parseIfStatement() {
    const Token& ifTok = consume(TokenType::IF, "Expected 'if'");
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseCondition();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto thenBranch = parseBlock();
    std::unique_ptr<BlockStmtNode> elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = parseBlock();
    }
    return std::make_unique<IfStmtNode>(std::move(cond), std::move(thenBranch),
                                        std::move(elseBranch), ifTok.line, ifTok.column);
}

std::unique_ptr<WhileStmtNode> Parser::parseWhileStatement() {
    const Token& whileTok = consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = parseCondition();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto body = parseBlock();
    return std::make_unique<WhileStmtNode>(std::move(cond), std::move(body), whileTok.line, whileTok.column);
}

std::unique_ptr<BlockStmtNode> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expected '{' to start block");
    auto block = std::make_unique<BlockStmtNode>();
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}' to close block");
    return block;
}

std::unique_ptr<ComparisonExprNode> Parser::parseCondition() {
    auto left = parseExpression();
    const Token& opTok = peek();
    if (opTok.type == TokenType::EQUAL_EQUAL ||
        opTok.type == TokenType::BANG_EQUAL ||
        opTok.type == TokenType::LESS ||
        opTok.type == TokenType::LESS_EQUAL ||
        opTok.type == TokenType::GREATER ||
        opTok.type == TokenType::GREATER_EQUAL) {
        advance();
        auto right = parseExpression();
        return std::make_unique<ComparisonExprNode>(opTok.type, std::move(left),
                                                    std::move(right), opTok.line, opTok.column);
    }
    throw ParserError("Expected comparison operator (==, !=, <, <=, >, >=) at line " +
                      std::to_string(opTok.line) + ", column " +
                      std::to_string(opTok.column) + " but got '" + opTok.lexeme + "'",
                      opTok.line, opTok.column);
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    auto expr = parseTerm();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        const Token& opTok = advance();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExprNode>(opTok.type, std::move(expr),
                                               std::move(right), opTok.line, opTok.column);
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseTerm() {
    auto expr = parseUnary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        const Token& opTok = advance();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExprNode>(opTok.type, std::move(expr),
                                               std::move(right), opTok.line, opTok.column);
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::parseUnary() {
    if (match(TokenType::MINUS)) {
        const Token& opTok = previous();
        auto operand = parseUnary();
        return std::make_unique<UnaryExprNode>(opTok.type, std::move(operand), opTok.line, opTok.column);
    }
    return parsePrimary();
}

std::unique_ptr<ExprNode> Parser::parsePrimary() {
    if (match(TokenType::INTEGER)) {
        const Token& tok = previous();
        int val = std::stoi(tok.lexeme);
        return std::make_unique<IntegerLiteralNode>(val, tok.line, tok.column);
    }
    if (match(TokenType::IDENTIFIER)) {
        const Token& tok = previous();
        return std::make_unique<VariableExprNode>(tok.lexeme, tok.line, tok.column);
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    const Token& tok = peek();
    throw ParserError("Expected primary expression (integer, variable, or parenthesized expression) at line " +
                      std::to_string(tok.line) + ", column " +
                      std::to_string(tok.column) + " but got '" + tok.lexeme + "'",
                      tok.line, tok.column);
}

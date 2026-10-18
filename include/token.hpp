#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

// ============================================================================
// token.hpp — Token type definitions for the Mini Compiler
// ============================================================================
//
// A token is the smallest meaningful unit of source code. The lexer reads
// raw text and produces a sequence of tokens. Each token has:
//   - A type   (what kind of token it is)
//   - A lexeme (the original text from the source code)
//   - A line   (which line it appeared on, starting from 1)
//   - A column (which column it started at, starting from 1)
//
// We use 'enum class' instead of plain 'enum' because:
//   1. It prevents name collisions (e.g. PLUS won't conflict with other code).
//   2. It requires the prefix TokenType::PLUS, making code clearer.
//   3. It does not implicitly convert to int, catching bugs at compile time.
// ============================================================================

enum class TokenType {
    // --- Keywords ---
    // Keywords are reserved words with special meaning in the language.
    // They look like identifiers but cannot be used as variable names.
    LET,        // "let"   — declares a new variable
    IF,         // "if"    — starts a conditional statement
    ELSE,       // "else"  — alternative branch of an if statement
    WHILE,      // "while" — starts a loop
    PRINT,      // "print" — outputs a value

    // --- Literals and identifiers ---
    IDENTIFIER, // A variable name, e.g. "total", "counter", "x"
    INTEGER,    // A whole number literal, e.g. "42", "0", "1000"

    // --- Arithmetic operators ---
    PLUS,       // "+"
    MINUS,      // "-"
    STAR,       // "*"
    SLASH,      // "/"

    // --- Assignment ---
    EQUAL,      // "=" — assigns a value to a variable

    // --- Comparison operators ---
    // These compare two values and produce a true/false result.
    EQUAL_EQUAL,    // "==" — equal to
    BANG_EQUAL,     // "!=" — not equal to
    LESS,           // "<"  — less than
    LESS_EQUAL,     // "<=" — less than or equal to
    GREATER,        // ">"  — greater than
    GREATER_EQUAL,  // ">=" — greater than or equal to

    // --- Delimiters ---
    LPAREN,     // "(" — left parenthesis
    RPAREN,     // ")" — right parenthesis
    LBRACE,     // "{" — left curly brace
    RBRACE,     // "}" — right curly brace
    SEMICOLON,  // ";" — statement terminator

    // --- Special ---
    END_OF_FILE // Marks the end of the input. Every token sequence ends with this.
};

// Convert a TokenType to a human-readable string for debugging and output.
// This is a simple switch-case function — nothing fancy.
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LET:            return "LET";
        case TokenType::IF:             return "IF";
        case TokenType::ELSE:           return "ELSE";
        case TokenType::WHILE:          return "WHILE";
        case TokenType::PRINT:          return "PRINT";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::INTEGER:        return "INTEGER";
        case TokenType::PLUS:           return "PLUS";
        case TokenType::MINUS:          return "MINUS";
        case TokenType::STAR:           return "STAR";
        case TokenType::SLASH:          return "SLASH";
        case TokenType::EQUAL:          return "EQUAL";
        case TokenType::EQUAL_EQUAL:    return "EQUAL_EQUAL";
        case TokenType::BANG_EQUAL:     return "BANG_EQUAL";
        case TokenType::LESS:           return "LESS";
        case TokenType::LESS_EQUAL:     return "LESS_EQUAL";
        case TokenType::GREATER:        return "GREATER";
        case TokenType::GREATER_EQUAL:  return "GREATER_EQUAL";
        case TokenType::LPAREN:         return "LPAREN";
        case TokenType::RPAREN:         return "RPAREN";
        case TokenType::LBRACE:         return "LBRACE";
        case TokenType::RBRACE:         return "RBRACE";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::END_OF_FILE:    return "END_OF_FILE";
        default:                        return "UNKNOWN";
    }
}

// ============================================================================
// Token — Represents a single token produced by the lexer.
//
// Example: For the source code "let x = 10;", the lexer produces:
//   Token{LET,       "let", line=1, column=1}
//   Token{IDENTIFIER, "x",  line=1, column=5}
//   Token{EQUAL,      "=",  line=1, column=7}
//   Token{INTEGER,    "10", line=1, column=9}
//   Token{SEMICOLON,  ";",  line=1, column=11}
//   Token{END_OF_FILE, "",  line=1, column=12}
// ============================================================================
struct Token {
    TokenType   type;       // What kind of token this is
    std::string lexeme;     // The original text from the source code
    int         line;       // Line number where this token starts (1-indexed)
    int         column;     // Column number where this token starts (1-indexed)

    // Constructor for convenience
    Token(TokenType type, const std::string& lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) {}
};

#endif // TOKEN_HPP

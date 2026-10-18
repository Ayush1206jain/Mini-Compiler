#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include "token.hpp"

// ============================================================================
// lexer.hpp — Lexer (tokenizer) for the Mini Compiler
// ============================================================================
//
// The Lexer class reads a source-code string and converts it into a list
// of tokens. It is the first stage of the compiler pipeline.
//
// How it works:
//   1. The lexer walks through the source string one character at a time.
//   2. It skips whitespace (spaces, tabs, newlines, carriage returns).
//   3. It recognizes keywords, identifiers, integer literals, operators,
//      and delimiters.
//   4. It tracks the current line and column for error reporting.
//   5. When it reaches the end of input, it produces an END_OF_FILE token.
//   6. If it encounters an unsupported character, it throws an error.
//
// Usage:
//   Lexer lexer("let x = 10;");
//   std::vector<Token> tokens = lexer.tokenize();
// ============================================================================

// Custom exception for lexer errors — includes position information.
class LexerError : public std::runtime_error {
public:
    int line;
    int column;

    LexerError(const std::string& message, int line, int column)
        : std::runtime_error(message), line(line), column(column) {}
};

class Lexer {
public:
    // Constructor: takes the entire source code as a string.
    explicit Lexer(const std::string& source);

    // Tokenize the entire source code and return all tokens.
    // The last token is always END_OF_FILE.
    // Throws LexerError if an unsupported character is encountered.
    std::vector<Token> tokenize();

private:
    std::string source_;    // The full source code
    size_t      pos_;       // Current position in the source string (0-indexed)
    int         line_;      // Current line number (1-indexed)
    int         column_;    // Current column number (1-indexed)

    // --- Helper methods ---

    // Returns the current character without advancing.
    // Returns '\0' if we've reached the end of input.
    char currentChar() const;

    // Returns the character at pos_ + 1 without advancing.
    // Returns '\0' if that position is past the end.
    char peekNext() const;

    // Advances to the next character, updating line and column.
    void advance();

    // Returns true if we've consumed all characters.
    bool isAtEnd() const;

    // Skips whitespace characters and single-line comments (// ...).
    void skipWhitespaceAndComments();

    // Reads an identifier or keyword starting at the current position.
    // Returns the appropriate token (IDENTIFIER or a keyword like LET).
    Token readIdentifierOrKeyword();

    // Reads an integer literal (a sequence of digits) starting at current position.
    Token readInteger();

    // Reads the next single token from the current position.
    // This is the core dispatch function that decides what kind of token
    // the current character begins.
    Token readNextToken();
};

#endif // LEXER_HPP

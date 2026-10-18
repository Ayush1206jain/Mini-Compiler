#include "lexer.hpp"
#include <cctype>       // For std::isalpha, std::isdigit, std::isalnum
#include <unordered_map>

// ============================================================================
// lexer.cpp — Lexer implementation for the Mini Compiler
// ============================================================================
//
// This file implements all the methods declared in lexer.hpp.
// The lexer converts source-code text into a sequence of tokens.
//
// Key design decisions:
//   - We use a single-pass approach: one forward scan through the source.
//   - Keywords are detected by first reading an identifier, then checking
//     if it matches a keyword. This avoids complex lookahead.
//   - Two-character operators (==, !=, <=, >=) are handled by peeking
//     at the next character after reading the first one.
//   - Line and column tracking is updated character-by-character in advance().
// ============================================================================

// ---------------------------------------------------------------------------
// Keyword lookup table
// ---------------------------------------------------------------------------
// After reading a word (letters/digits/underscores), we check this table.
// If the word is in the table, it's a keyword; otherwise, it's an identifier.
static const std::unordered_map<std::string, TokenType> keywords = {
    {"let",   TokenType::LET},
    {"if",    TokenType::IF},
    {"else",  TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"print", TokenType::PRINT}
};

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), column_(1) {}

// ---------------------------------------------------------------------------
// tokenize — Main entry point
// ---------------------------------------------------------------------------
// Repeatedly reads tokens until END_OF_FILE is produced.
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token = readNextToken();
        tokens.push_back(token);

        // Stop after adding the END_OF_FILE token
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return tokens;
}

// ---------------------------------------------------------------------------
// currentChar — Returns the character at the current position
// ---------------------------------------------------------------------------
// Returns '\0' when we've reached the end. This makes boundary checks simple:
// we can always call currentChar() without worrying about going out of bounds.
char Lexer::currentChar() const {
    if (pos_ >= source_.size()) {
        return '\0';
    }
    return source_[pos_];
}

// ---------------------------------------------------------------------------
// peekNext — Returns the character after the current one
// ---------------------------------------------------------------------------
// Used for two-character operators: when we see '=', we peek ahead to check
// if the next character is also '=' (making it '==').
char Lexer::peekNext() const {
    size_t nextPos = pos_ + 1;
    if (nextPos >= source_.size()) {
        return '\0';
    }
    return source_[nextPos];
}

// ---------------------------------------------------------------------------
// advance — Moves to the next character
// ---------------------------------------------------------------------------
// This is the ONLY place where pos_, line_, and column_ are updated.
// Centralizing position tracking here prevents bugs where we forget to
// update the line or column.
//
// Rules:
//   - A newline ('\n') increments the line and resets the column to 1.
//   - Any other character increments the column by 1.
//   - Carriage return ('\r') is treated as a regular character for column
//     tracking; the newline that usually follows will handle the line break.
void Lexer::advance() {
    if (pos_ < source_.size()) {
        if (source_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

// ---------------------------------------------------------------------------
// isAtEnd — Checks if we've consumed all input
// ---------------------------------------------------------------------------
bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

// ---------------------------------------------------------------------------
// skipWhitespaceAndComments — Skips spaces, tabs, newlines, and comments
// ---------------------------------------------------------------------------
// The lexer calls this before reading each token. Whitespace is not
// meaningful in our language (unlike Python), so we skip it.
//
// We also support single-line comments starting with "//".
// Everything from "//" to the end of the line is ignored.
//
// We use a loop because after skipping a comment, there may be more
// whitespace or another comment on the next line.
void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = currentChar();

        // Skip whitespace characters
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
            continue;
        }

        // Skip single-line comments: // ... until end of line
        if (c == '/' && peekNext() == '/') {
            // Skip the two '/' characters
            advance();
            advance();
            // Skip everything until newline or end of input
            while (!isAtEnd() && currentChar() != '\n') {
                advance();
            }
            // The newline itself will be skipped in the next iteration
            continue;
        }

        // Not whitespace or comment — stop skipping
        break;
    }
}

// ---------------------------------------------------------------------------
// readIdentifierOrKeyword — Reads a word (identifier or keyword)
// ---------------------------------------------------------------------------
// An identifier starts with a letter or underscore, followed by letters,
// digits, or underscores. Examples: "x", "total", "my_var", "_temp"
//
// After reading the full word, we check the keywords table.
// If it matches (e.g., "let", "if"), we return a keyword token.
// Otherwise, we return an IDENTIFIER token.
//
// This approach is called "maximal munch": we read as many valid
// characters as possible before deciding what the token is.
Token Lexer::readIdentifierOrKeyword() {
    int startLine = line_;
    int startColumn = column_;
    std::string word;

    // Read all alphanumeric characters and underscores
    while (!isAtEnd() && (std::isalnum(currentChar()) || currentChar() == '_')) {
        word += currentChar();
        advance();
    }

    // Check if the word is a keyword
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return Token(it->second, word, startLine, startColumn);
    }

    // It's a regular identifier
    return Token(TokenType::IDENTIFIER, word, startLine, startColumn);
}

// ---------------------------------------------------------------------------
// readInteger — Reads a sequence of digits
// ---------------------------------------------------------------------------
// An integer literal is one or more decimal digits: "0", "42", "1000".
// We don't handle negative numbers here — the parser will treat "-42" as
// the minus operator applied to the integer 42.
Token Lexer::readInteger() {
    int startLine = line_;
    int startColumn = column_;
    std::string number;

    while (!isAtEnd() && std::isdigit(currentChar())) {
        number += currentChar();
        advance();
    }

    return Token(TokenType::INTEGER, number, startLine, startColumn);
}

// ---------------------------------------------------------------------------
// readNextToken — Core dispatch: reads one token from the current position
// ---------------------------------------------------------------------------
// This function is called repeatedly by tokenize(). It:
//   1. Skips whitespace and comments.
//   2. Checks if we've reached the end of input (returns END_OF_FILE).
//   3. Looks at the current character to decide what kind of token to read.
//   4. For two-character operators, peeks at the next character.
//   5. Throws LexerError for unsupported characters.
Token Lexer::readNextToken() {
    // Step 1: Skip any whitespace and comments before the next token
    skipWhitespaceAndComments();

    // Step 2: Check for end of input
    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", line_, column_);
    }

    char c = currentChar();
    int tokenLine = line_;
    int tokenColumn = column_;

    // Step 3a: Identifiers and keywords start with a letter or underscore
    if (std::isalpha(c) || c == '_') {
        return readIdentifierOrKeyword();
    }

    // Step 3b: Integer literals start with a digit
    if (std::isdigit(c)) {
        return readInteger();
    }

    // Step 3c: Operators and delimiters
    switch (c) {
        case '+':
            advance();
            return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);

        case '-':
            advance();
            return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);

        case '*':
            advance();
            return Token(TokenType::STAR, "*", tokenLine, tokenColumn);

        case '/':
            // Note: "//" comments are already handled in skipWhitespaceAndComments.
            // If we get here, it's the division operator.
            advance();
            return Token(TokenType::SLASH, "/", tokenLine, tokenColumn);

        case '=':
            advance();
            // Check if it's "==" (comparison) or just "=" (assignment)
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::EQUAL_EQUAL, "==", tokenLine, tokenColumn);
            }
            return Token(TokenType::EQUAL, "=", tokenLine, tokenColumn);

        case '!':
            advance();
            // "!=" is the only valid use of '!' in our language
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::BANG_EQUAL, "!=", tokenLine, tokenColumn);
            }
            // Standalone '!' is not supported
            throw LexerError(
                "Unexpected character '!' at line " + std::to_string(tokenLine) +
                ", column " + std::to_string(tokenColumn) +
                ". Did you mean '!='?",
                tokenLine, tokenColumn
            );

        case '<':
            advance();
            // Check if it's "<=" or just "<"
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", tokenLine, tokenColumn);
            }
            return Token(TokenType::LESS, "<", tokenLine, tokenColumn);

        case '>':
            advance();
            // Check if it's ">=" or just ">"
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", tokenLine, tokenColumn);
            }
            return Token(TokenType::GREATER, ">", tokenLine, tokenColumn);

        case '(':
            advance();
            return Token(TokenType::LPAREN, "(", tokenLine, tokenColumn);

        case ')':
            advance();
            return Token(TokenType::RPAREN, ")", tokenLine, tokenColumn);

        case '{':
            advance();
            return Token(TokenType::LBRACE, "{", tokenLine, tokenColumn);

        case '}':
            advance();
            return Token(TokenType::RBRACE, "}", tokenLine, tokenColumn);

        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);

        default:
            // Unknown character — report error with position
            throw LexerError(
                "Unexpected character '" + std::string(1, c) +
                "' at line " + std::to_string(tokenLine) +
                ", column " + std::to_string(tokenColumn),
                tokenLine, tokenColumn
            );
    }
}

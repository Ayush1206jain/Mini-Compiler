// ============================================================================
// main.cpp — Command-line driver for the Mini Compiler lexer
// ============================================================================
//
// Usage:
//   mini_compiler <source_file>
//
// Example:
//   mini_compiler examples/day1_example.mc
//
// This program reads a source file, runs the lexer, and prints each token
// on its own line with its type, lexeme, line number, and column number.
// ============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"

int main(int argc, char* argv[]) {
    // Check that a filename was provided
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " examples/day1_example.mc" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    // Open the source file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return 1;
    }

    // Read the entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Run the lexer
    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        // Print a header
        std::cout << "Tokens from: " << filename << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        std::cout << "Line  Col   Type              Lexeme" << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        // Print each token
        for (const Token& token : tokens) {
            // Format: Line  Col   Type              Lexeme
            std::string typeStr = tokenTypeToString(token.type);

            // Pad the type string for alignment
            while (typeStr.size() < 18) {
                typeStr += ' ';
            }

            std::cout << token.line;
            // Pad line number
            std::string lineStr = std::to_string(token.line);
            for (size_t i = lineStr.size(); i < 6; i++) std::cout << ' ';

            std::cout << token.column;
            std::string colStr = std::to_string(token.column);
            for (size_t i = colStr.size(); i < 6; i++) std::cout << ' ';

            std::cout << typeStr;

            // Show lexeme (use quotes for clarity, show empty for EOF)
            if (token.type == TokenType::END_OF_FILE) {
                std::cout << "(end)";
            } else {
                std::cout << "\"" << token.lexeme << "\"";
            }

            std::cout << std::endl;
        }

        std::cout << std::string(60, '-') << std::endl;
        std::cout << "Total tokens: " << tokens.size() << std::endl;

    } catch (const LexerError& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

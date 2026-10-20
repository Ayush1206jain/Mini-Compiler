// ============================================================================
// main.cpp — Command-line driver for the Mini Compiler
// ============================================================================
//
// Usage:
//   mini_compiler <source_file> [mode]
//
// Modes:
//   (default)       Run final optimized IR through the interpreter
//   --tokens        Print token stream from lexer
//   --ir            Print original unoptimized Three-Address IR
//   --opt-ir        Print Three-Address IR after Day 4 (Constant Folding & Propagation)
//   --dce-ir        Print Three-Address IR after Day 5 (Dead-Code Elimination)
//   --asm           Print generated Pseudo-Assembly from final optimized IR
//   --stats         Print IR instruction-count analysis and optimization reduction
//   --compare       Compare unoptimized vs. optimized execution and semantic equivalence
//   --run-unopt     Execute the unoptimized IR
//   --all           Comprehensive dump of all stages, stats, assembly, and output
// ============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
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
#include "pseudo_assembly_generator.hpp"
#include "instruction_count_analyzer.hpp"
#include "pipeline_validator.hpp"

void printTokens(const std::vector<Token>& tokens, const std::string& filename) {
    std::cout << "Tokens from: " << filename << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Line  Col   Type              Lexeme" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const Token& token : tokens) {
        std::string typeStr = tokenTypeToString(token.type);
        while (typeStr.size() < 18) typeStr += ' ';

        std::cout << token.line;
        std::string lineStr = std::to_string(token.line);
        for (size_t i = lineStr.size(); i < 6; i++) std::cout << ' ';

        std::cout << token.column;
        std::string colStr = std::to_string(token.column);
        for (size_t i = colStr.size(); i < 6; i++) std::cout << ' ';

        std::cout << typeStr;
        if (token.type == TokenType::END_OF_FILE) {
            std::cout << "(end)";
        } else {
            std::cout << "\"" << token.lexeme << "\"";
        }
        std::cout << std::endl;
    }
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Total tokens: " << tokens.size() << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <source_file> [--tokens | --ir | --opt-ir | --dce-ir | --asm | --stats | --compare | --run-unopt | --all]"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " examples/day6_arithmetic.mc --all" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string mode = (argc >= 3) ? argv[2] : "";

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    try {
        // Fast-path: Comparison mode uses the dedicated PipelineValidator
        if (mode == "--compare") {
            ValidationReport report = PipelineValidator::validate(source);
            std::cout << report.toString();
            return report.passed ? 0 : 1;
        }

        // Stage 1: Lexer
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        if (mode == "--tokens") {
            printTokens(tokens, filename);
            return 0;
        }

        // Stage 2: Parser & AST
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        // Stage 3: IR Generation (Original IR)
        IRGenerator irGen;
        IRProgram origIR = irGen.generate(*ast);

        if (mode == "--ir") {
            std::cout << "=== Original Three-Address IR ===" << std::endl;
            std::cout << origIR.toString();
            return 0;
        }

        if (mode == "--run-unopt") {
            IRInterpreter interpreter;
            std::vector<int> output = interpreter.execute(origIR);
            for (int val : output) {
                std::cout << val << std::endl;
            }
            return 0;
        }

        // Stage 4: IR Optimization (Day 4: Constant Propagation & Folding)
        IROptimizer optimizer;
        IRProgram foldedIR = optimizer.optimize(origIR);

        if (mode == "--opt-ir") {
            std::cout << "=== Three-Address IR (After Constant Folding & Propagation) ===" << std::endl;
            std::cout << foldedIR.toString();
            return 0;
        }

        // Stage 5: Dead-Code Elimination (Day 5: DCE)
        DeadCodeElimination dce;
        IRProgram finalOptIR = dce.eliminate(foldedIR);

        if (mode == "--dce-ir") {
            std::cout << "=== Three-Address IR (After Dead-Code Elimination) ===" << std::endl;
            std::cout << finalOptIR.toString();
            return 0;
        }

        // Stage 6: Instruction Count Analysis (Day 6)
        if (mode == "--stats") {
            auto stats = InstructionCountAnalyzer::analyze(origIR, foldedIR, finalOptIR);
            std::cout << stats.toString();
            return 0;
        }

        // Stage 7: Pseudo-Assembly Code Generation (Day 5: Pseudo-Assembly)
        PseudoAssemblyGenerator asmGen;
        PseudoAssemblyProgram pasm = asmGen.generate(finalOptIR);

        if (mode == "--asm") {
            std::cout << "=== Generated Pseudo-Assembly ===" << std::endl;
            std::cout << pasm.toString();
            return 0;
        }

        if (mode == "--all") {
            printTokens(tokens, filename);
            std::cout << std::endl;
            std::cout << "=== 1. Original Three-Address IR ===" << std::endl;
            std::cout << origIR.toString();
            std::cout << std::endl;
            std::cout << "=== 2. IR After Constant Folding & Propagation (Day 4) ===" << std::endl;
            std::cout << foldedIR.toString();
            std::cout << std::endl;
            std::cout << "=== 3. IR After Dead-Code Elimination (Day 5) ===" << std::endl;
            std::cout << finalOptIR.toString();
            std::cout << std::endl;
            auto stats = InstructionCountAnalyzer::analyze(origIR, foldedIR, finalOptIR);
            std::cout << stats.toString();
            std::cout << std::endl;
            std::cout << "=== 4. Generated Pseudo-Assembly ===" << std::endl;
            std::cout << pasm.toString();
            std::cout << std::endl;
            std::cout << "=== 5. Program Output (Optimized IR Execution) ===" << std::endl;
        }

        // Stage 8: Execution via IR Interpreter (Executes final optimized IR)
        IRInterpreter interpreter;
        std::vector<int> output = interpreter.execute(finalOptIR);

        for (int val : output) {
            std::cout << val << std::endl;
        }

    } catch (const LexerError& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        return 1;
    } catch (const ParserError& e) {
        std::cerr << "Parser Error: " << e.what() << std::endl;
        return 1;
    } catch (const RuntimeError& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

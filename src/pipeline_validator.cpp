#include "pipeline_validator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "ir_generator.hpp"
#include "ir_interpreter.hpp"
#include "ir_optimizer.hpp"
#include "dead_code_elimination.hpp"
#include "pseudo_assembly_generator.hpp"
#include <sstream>

std::string executionCategoryToString(ExecutionCategory cat) {
    switch (cat) {
        case ExecutionCategory::SUCCESS:                return "SUCCESS";
        case ExecutionCategory::DIVISION_BY_ZERO:       return "DIVISION_BY_ZERO";
        case ExecutionCategory::UNINITIALIZED_VARIABLE: return "UNINITIALIZED_VARIABLE";
        case ExecutionCategory::STEP_LIMIT_EXCEEDED:    return "STEP_LIMIT_EXCEEDED";
        case ExecutionCategory::PARSER_ERROR:           return "PARSER_ERROR";
        case ExecutionCategory::LEXER_ERROR:            return "LEXER_ERROR";
        case ExecutionCategory::OTHER_RUNTIME_ERROR:    return "OTHER_RUNTIME_ERROR";
    }
    return "UNKNOWN";
}

ExecutionCategory PipelineValidator::classifyError(const std::string& errorMsg) {
    if (errorMsg.find("Division by zero") != std::string::npos ||
        (errorMsg.find("divide") != std::string::npos && errorMsg.find("0") != std::string::npos)) {
        return ExecutionCategory::DIVISION_BY_ZERO;
    }
    if (errorMsg.find("uninitialized") != std::string::npos ||
        errorMsg.find("Uninitialized") != std::string::npos) {
        return ExecutionCategory::UNINITIALIZED_VARIABLE;
    }
    if (errorMsg.find("limit exceeded") != std::string::npos ||
        errorMsg.find("step limit") != std::string::npos) {
        return ExecutionCategory::STEP_LIMIT_EXCEEDED;
    }
    return ExecutionCategory::OTHER_RUNTIME_ERROR;
}

ValidationReport PipelineValidator::validate(const std::string& sourceCode, size_t maxSteps) {
    ValidationReport report;

    // Step 1: Lexical Analysis
    std::vector<Token> tokens;
    try {
        Lexer lexer(sourceCode);
        tokens = lexer.tokenize();
    } catch (const LexerError& e) {
        report.unoptimized.completed = false;
        report.unoptimized.category = ExecutionCategory::LEXER_ERROR;
        report.unoptimized.errorMessage = e.what();

        report.optimized = report.unoptimized;
        report.categoriesMatch = true;
        report.outputsMatch = true;
        report.passed = true;
        return report;
    }

    // Step 2: Parsing & AST
    std::unique_ptr<ProgramNode> ast;
    try {
        Parser parser(tokens);
        ast = parser.parseProgram();
    } catch (const ParserError& e) {
        report.unoptimized.completed = false;
        report.unoptimized.category = ExecutionCategory::PARSER_ERROR;
        report.unoptimized.errorMessage = e.what();

        report.optimized = report.unoptimized;
        report.categoriesMatch = true;
        report.outputsMatch = true;
        report.passed = true;
        return report;
    }

    // Step 3: IR Generation (Original IR)
    try {
        IRGenerator irGen;
        report.originalIR = irGen.generate(*ast);
    } catch (const std::exception& e) {
        report.unoptimized.completed = false;
        report.unoptimized.category = ExecutionCategory::OTHER_RUNTIME_ERROR;
        report.unoptimized.errorMessage = e.what();
        report.mismatchReason = "IR Generation failed: " + std::string(e.what());
        return report;
    }

    // Step 4: Execute Unoptimized IR
    {
        IRInterpreter unoptInterp(maxSteps);
        try {
            report.unoptimized.output = unoptInterp.execute(report.originalIR);
            report.unoptimized.completed = true;
            report.unoptimized.category = ExecutionCategory::SUCCESS;
        } catch (const RuntimeError& e) {
            report.unoptimized.completed = false;
            report.unoptimized.category = classifyError(e.what());
            report.unoptimized.errorMessage = e.what();
        }
    }

    // Step 5: Optimization Pipeline (Day 4 Folding + Day 5 DCE)
    try {
        IROptimizer optimizer;
        report.foldedIR = optimizer.optimize(report.originalIR);

        DeadCodeElimination dce;
        report.finalOptimizedIR = dce.eliminate(report.foldedIR);
    } catch (const std::exception& e) {
        report.optimized.completed = false;
        report.optimized.category = ExecutionCategory::OTHER_RUNTIME_ERROR;
        report.optimized.errorMessage = e.what();
        report.mismatchReason = "Optimization pipeline crashed: " + std::string(e.what());
        return report;
    }

    // Step 6: Instruction Count Analysis
    report.instructionStats = InstructionCountAnalyzer::analyze(report.originalIR,
                                                               report.foldedIR,
                                                               report.finalOptimizedIR);

    // Step 7: Execute Optimized IR
    {
        IRInterpreter optInterp(maxSteps);
        try {
            report.optimized.output = optInterp.execute(report.finalOptimizedIR);
            report.optimized.completed = true;
            report.optimized.category = ExecutionCategory::SUCCESS;
        } catch (const RuntimeError& e) {
            report.optimized.completed = false;
            report.optimized.category = classifyError(e.what());
            report.optimized.errorMessage = e.what();
        }
    }

    // Step 8: Pseudo-Assembly Code Generation
    try {
        PseudoAssemblyGenerator asmGen;
        auto pasm = asmGen.generate(report.finalOptimizedIR);
        report.pseudoAssembly = pasm.toString();
        report.pseudoAssemblyGenerated = true;
    } catch (const std::exception& e) {
        report.pseudoAssemblyGenerated = false;
        report.mismatchReason = "Pseudo-assembly generation failed: " + std::string(e.what());
    }

    // Step 9: Compare Equivalence
    report.categoriesMatch = (report.unoptimized.category == report.optimized.category);
    report.outputsMatch = (report.unoptimized.output == report.optimized.output);

    if (!report.categoriesMatch) {
        report.mismatchReason = "Error category mismatch: unoptimized=" +
                                executionCategoryToString(report.unoptimized.category) +
                                ", optimized=" + executionCategoryToString(report.optimized.category);
    } else if (!report.outputsMatch) {
        report.mismatchReason = "Output mismatch between unoptimized and optimized IR execution.";
    }

    report.passed = (report.categoriesMatch && report.outputsMatch && report.pseudoAssemblyGenerated);
    return report;
}

std::string ValidationReport::toString() const {
    std::ostringstream oss;
    oss << "=== Execution Comparison (Unoptimized vs. Optimized) ===" << "\n";
    oss << "Unoptimized Status: " << executionCategoryToString(unoptimized.category)
        << (unoptimized.completed ? " (Completed)" : " (" + unoptimized.errorMessage + ")") << "\n";
    oss << "Optimized Status:   " << executionCategoryToString(optimized.category)
        << (optimized.completed ? " (Completed)" : " (" + optimized.errorMessage + ")") << "\n";

    oss << "Unoptimized Output: [";
    for (size_t i = 0; i < unoptimized.output.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << unoptimized.output[i];
    }
    oss << "]\n";

    oss << "Optimized Output:   [";
    for (size_t i = 0; i < optimized.output.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << optimized.output[i];
    }
    oss << "]\n";

    oss << "Semantic Equivalence: " << (passed ? "MATCH (Outputs and errors identical)" : "MISMATCH") << "\n";
    if (!mismatchReason.empty()) {
        oss << "Reason: " << mismatchReason << "\n";
    }

    oss << "Instruction Reduction: " << instructionStats.originalCount << " -> "
        << instructionStats.dceCount << " (" << instructionStats.percentageReduction << "%)\n";
    oss << "Pseudo-Assembly Generation: " << (pseudoAssemblyGenerated ? "SUCCESS" : "FAILED") << "\n";

    return oss.str();
}

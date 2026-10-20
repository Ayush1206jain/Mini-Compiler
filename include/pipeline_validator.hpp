#ifndef PIPELINE_VALIDATOR_HPP
#define PIPELINE_VALIDATOR_HPP

#include <string>
#include <vector>
#include "ir.hpp"
#include "instruction_count_analyzer.hpp"
#include "pseudo_assembly_generator.hpp"

// ============================================================================
// pipeline_validator.hpp — End-to-End Pipeline Validation and Comparison Helper
// ============================================================================
//
// Exercises the complete compiler pipeline and compares unoptimized execution
// with optimized execution to verify semantic equivalence and runtime safety.
//
// Pipeline:
//   Source -> Lexer -> Parser -> AST -> Original IR
//   Original IR -> IRInterpreter -> Unoptimized Output
//   Original IR (Copy) -> Constant Folding -> DCE -> Final Optimized IR
//   Final Optimized IR -> IRInterpreter -> Optimized Output
//   Final Optimized IR -> PseudoAssemblyGenerator -> Assembly Output
//
// Comparison Metrics:
//   - Output stream equality (order and integer values).
//   - Execution completion status (success vs error category).
//   - Instruction count metrics (original vs optimized).
//   - Pseudo-assembly generation verification.
// ============================================================================

enum class ExecutionCategory {
    SUCCESS,
    DIVISION_BY_ZERO,
    UNINITIALIZED_VARIABLE,
    STEP_LIMIT_EXCEEDED,
    PARSER_ERROR,
    LEXER_ERROR,
    OTHER_RUNTIME_ERROR
};

std::string executionCategoryToString(ExecutionCategory cat);

struct PathExecutionResult {
    bool completed = false;
    ExecutionCategory category = ExecutionCategory::SUCCESS;
    std::string errorMessage;
    std::vector<int> output;
};

struct ValidationReport {
    bool passed = false;                     // Semantic equivalence or matching error
    bool outputsMatch = false;               // Identical printed output vectors
    bool categoriesMatch = false;            // Identical completion or error category
    bool pseudoAssemblyGenerated = false;

    PathExecutionResult unoptimized;
    PathExecutionResult optimized;
    InstructionStats instructionStats;

    IRProgram originalIR;
    IRProgram foldedIR;
    IRProgram finalOptimizedIR;
    std::string pseudoAssembly;
    std::string mismatchReason;

    std::string toString() const;
};

class PipelineValidator {
public:
    // Runs source code through unoptimized and optimized paths and validates equivalence
    static ValidationReport validate(const std::string& sourceCode, size_t maxSteps = 100000);

    // Classifies exception error messages into stable categories
    static ExecutionCategory classifyError(const std::string& errorMsg);
};

#endif // PIPELINE_VALIDATOR_HPP

#ifndef IR_INTERPRETER_HPP
#define IR_INTERPRETER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "ir.hpp"

// ============================================================================
// ir_interpreter.hpp — Three-Address IR Interpreter
// ============================================================================
//
// Directly executes an IRProgram.
//
// Key features:
//   - Runtime symbol environment for variables and temporaries.
//   - Label pre-indexing (label -> instruction index) for O(1) jump dispatch.
//   - Error detection:
//       * Division by zero
//       * Uninitialized variable or temporary access
//       * Invalid jump label targets
//       * Instruction step limit (default 100,000) against infinite loops
//   - Captured output list for clean testing and console output for CLI.
// ============================================================================

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& message)
        : std::runtime_error(message) {}
};

class IRInterpreter {
public:
    explicit IRInterpreter(size_t maxSteps = 100000);

    // Executes the IR program and returns all printed outputs as strings
    std::vector<int> execute(const IRProgram& program);

    // Accessors for state inspection (useful for tests and debugging)
    int getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;
    const std::vector<int>& getOutput() const { return output_; }

private:
    size_t maxSteps_;
    std::unordered_map<std::string, int> env_;
    std::unordered_map<std::string, size_t> labelMap_;
    std::vector<int> output_;

    void buildLabelMap(const IRProgram& program);
    int readOperand(const std::string& name) const;
};

#endif // IR_INTERPRETER_HPP

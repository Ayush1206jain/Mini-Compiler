#ifndef IR_GENERATOR_HPP
#define IR_GENERATOR_HPP

#include <string>
#include <memory>
#include "ast.hpp"
#include "ir.hpp"

// ============================================================================
// ir_generator.hpp — AST to Three-Address IR Code Generator
// ============================================================================
//
// Traverses the Abstract Syntax Tree (AST) and generates a linear stream
// of Three-Address Code (TAC) instructions.
//
// Features:
//   - Deterministic temporary generation: t0, t1, t2, ...
//   - Deterministic label generation: L0, L1, L2, ...
//   - Respects operator precedence through post-order expression traversal.
//   - Generates control-flow jumps for if/else and while statements.
// ============================================================================

class IRGenerator {
public:
    IRGenerator();

    IRProgram generate(const ProgramNode& program);

private:
    IRProgram ir_;
    int tempCount_;
    int labelCount_;

    std::string newTemp();
    std::string newLabel();

    // Visitor / Generation methods
    void generateStatement(const StmtNode& stmt);
    void generateLet(const LetStmtNode& stmt);
    void generateAssign(const AssignStmtNode& stmt);
    void generatePrint(const PrintStmtNode& stmt);
    void generateIf(const IfStmtNode& stmt);
    void generateWhile(const WhileStmtNode& stmt);
    void generateBlock(const BlockStmtNode& block);

    // Expression generation returns the variable / temporary name holding the result
    std::string generateExpression(const ExprNode& expr);
    std::string generateIntegerLiteral(const IntegerLiteralNode& expr);
    std::string generateVariableExpr(const VariableExprNode& expr);
    std::string generateBinaryExpr(const BinaryExprNode& expr);
    std::string generateUnaryExpr(const UnaryExprNode& expr);
    std::string generateComparisonExpr(const ComparisonExprNode& expr);
};

#endif // IR_GENERATOR_HPP

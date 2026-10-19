#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>
#include "token.hpp"

// ============================================================================
// ast.hpp — Abstract Syntax Tree (AST) node definitions
// ============================================================================
//
// The AST is a tree representation of the syntactic structure of the source
// code. Each node corresponds to a construct in the grammar.
//
// Types of nodes:
//   - Expressions: produce values (e.g., IntegerLiteral, VariableExpr, BinaryExpr)
//   - Statements: execute actions (e.g., LetStmt, AssignStmt, PrintStmt, IfStmt, WhileStmt)
// ============================================================================

enum class ASTNodeType {
    // Expressions
    INTEGER_LITERAL,
    VARIABLE_EXPR,
    BINARY_EXPR,
    UNARY_EXPR,
    COMPARISON_EXPR,

    // Statements
    LET_STMT,
    ASSIGN_STMT,
    PRINT_STMT,
    IF_STMT,
    WHILE_STMT,
    BLOCK_STMT,
    PROGRAM
};

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
};

// Base class for expression nodes
class ExprNode : public ASTNode {};

// Base class for statement nodes
class StmtNode : public ASTNode {};

// --- Expression Nodes ---

// Integer literal, e.g. "42"
class IntegerLiteralNode : public ExprNode {
public:
    int value;
    int line;
    int column;

    IntegerLiteralNode(int val, int line, int col)
        : value(val), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::INTEGER_LITERAL; }
};

// Variable reference, e.g. "x" or "total"
class VariableExprNode : public ExprNode {
public:
    std::string name;
    int line;
    int column;

    VariableExprNode(const std::string& name, int line, int col)
        : name(name), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::VARIABLE_EXPR; }
};

// Binary arithmetic expression: left + right, left * right, etc.
class BinaryExprNode : public ExprNode {
public:
    TokenType op; // PLUS, MINUS, STAR, SLASH
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    int line;
    int column;

    BinaryExprNode(TokenType op, std::unique_ptr<ExprNode> left,
                   std::unique_ptr<ExprNode> right, int line, int col)
        : op(op), left(std::move(left)), right(std::move(right)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::BINARY_EXPR; }
};

// Unary expression: -operand
class UnaryExprNode : public ExprNode {
public:
    TokenType op; // MINUS
    std::unique_ptr<ExprNode> operand;
    int line;
    int column;

    UnaryExprNode(TokenType op, std::unique_ptr<ExprNode> operand, int line, int col)
        : op(op), operand(std::move(operand)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::UNARY_EXPR; }
};

// Comparison expression: left == right, left < right, etc.
class ComparisonExprNode : public ExprNode {
public:
    TokenType op; // EQUAL_EQUAL, BANG_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    int line;
    int column;

    ComparisonExprNode(TokenType op, std::unique_ptr<ExprNode> left,
                       std::unique_ptr<ExprNode> right, int line, int col)
        : op(op), left(std::move(left)), right(std::move(right)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::COMPARISON_EXPR; }
};

// --- Statement Nodes ---

// Variable declaration: "let x = expr;"
class LetStmtNode : public StmtNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> initializer;
    int line;
    int column;

    LetStmtNode(const std::string& name, std::unique_ptr<ExprNode> init, int line, int col)
        : name(name), initializer(std::move(init)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::LET_STMT; }
};

// Variable assignment: "x = expr;"
class AssignStmtNode : public StmtNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> value;
    int line;
    int column;

    AssignStmtNode(const std::string& name, std::unique_ptr<ExprNode> val, int line, int col)
        : name(name), value(std::move(val)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::ASSIGN_STMT; }
};

// Print statement: "print(expr);"
class PrintStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expression;
    int line;
    int column;

    PrintStmtNode(std::unique_ptr<ExprNode> expr, int line, int col)
        : expression(std::move(expr)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::PRINT_STMT; }
};

// Block of statements: "{ stmt1; stmt2; }"
class BlockStmtNode : public StmtNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;

    BlockStmtNode() = default;

    ASTNodeType getType() const override { return ASTNodeType::BLOCK_STMT; }
};

// If/Else statement: "if (cond) { ... } else { ... }"
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ComparisonExprNode> condition;
    std::unique_ptr<BlockStmtNode> thenBranch;
    std::unique_ptr<BlockStmtNode> elseBranch; // nullptr if no else
    int line;
    int column;

    IfStmtNode(std::unique_ptr<ComparisonExprNode> cond,
               std::unique_ptr<BlockStmtNode> thenBranch,
               std::unique_ptr<BlockStmtNode> elseBranch,
               int line, int col)
        : condition(std::move(cond)), thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::IF_STMT; }
};

// While statement: "while (cond) { ... }"
class WhileStmtNode : public StmtNode {
public:
    std::unique_ptr<ComparisonExprNode> condition;
    std::unique_ptr<BlockStmtNode> body;
    int line;
    int column;

    WhileStmtNode(std::unique_ptr<ComparisonExprNode> cond,
                  std::unique_ptr<BlockStmtNode> body,
                  int line, int col)
        : condition(std::move(cond)), body(std::move(body)), line(line), column(col) {}

    ASTNodeType getType() const override { return ASTNodeType::WHILE_STMT; }
};

// Program: list of top-level statements
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;

    ProgramNode() = default;

    ASTNodeType getType() const override { return ASTNodeType::PROGRAM; }
};

#endif // AST_HPP

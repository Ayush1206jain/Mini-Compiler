#include "ir_generator.hpp"
#include <stdexcept>

IRGenerator::IRGenerator() : tempCount_(0), labelCount_(0) {}

std::string IRGenerator::newTemp() {
    return "t" + std::to_string(tempCount_++);
}

std::string IRGenerator::newLabel() {
    return "L" + std::to_string(labelCount_++);
}

IRProgram IRGenerator::generate(const ProgramNode& program) {
    ir_ = IRProgram();
    tempCount_ = 0;
    labelCount_ = 0;

    for (const auto& stmt : program.statements) {
        generateStatement(*stmt);
    }

    return ir_;
}

void IRGenerator::generateStatement(const StmtNode& stmt) {
    switch (stmt.getType()) {
        case ASTNodeType::LET_STMT:
            generateLet(static_cast<const LetStmtNode&>(stmt));
            break;
        case ASTNodeType::ASSIGN_STMT:
            generateAssign(static_cast<const AssignStmtNode&>(stmt));
            break;
        case ASTNodeType::PRINT_STMT:
            generatePrint(static_cast<const PrintStmtNode&>(stmt));
            break;
        case ASTNodeType::IF_STMT:
            generateIf(static_cast<const IfStmtNode&>(stmt));
            break;
        case ASTNodeType::WHILE_STMT:
            generateWhile(static_cast<const WhileStmtNode&>(stmt));
            break;
        case ASTNodeType::BLOCK_STMT:
            generateBlock(static_cast<const BlockStmtNode&>(stmt));
            break;
        default:
            throw std::runtime_error("Unknown statement type in IR generation");
    }
}

void IRGenerator::generateLet(const LetStmtNode& stmt) {
    // If the initializer is a simple integer literal, we can load directly into the variable:
    // LOAD_CONST var, value
    if (stmt.initializer->getType() == ASTNodeType::INTEGER_LITERAL) {
        const auto& lit = static_cast<const IntegerLiteralNode&>(*stmt.initializer);
        ir_.addInstruction(IRInstruction::makeLoadConst(stmt.name, lit.value));
    } else {
        std::string exprTemp = generateExpression(*stmt.initializer);
        ir_.addInstruction(IRInstruction::makeMove(stmt.name, exprTemp));
    }
}

void IRGenerator::generateAssign(const AssignStmtNode& stmt) {
    if (stmt.value->getType() == ASTNodeType::INTEGER_LITERAL) {
        const auto& lit = static_cast<const IntegerLiteralNode&>(*stmt.value);
        ir_.addInstruction(IRInstruction::makeLoadConst(stmt.name, lit.value));
    } else {
        std::string exprTemp = generateExpression(*stmt.value);
        ir_.addInstruction(IRInstruction::makeMove(stmt.name, exprTemp));
    }
}

void IRGenerator::generatePrint(const PrintStmtNode& stmt) {
    std::string valName;
    if (stmt.expression->getType() == ASTNodeType::VARIABLE_EXPR) {
        valName = static_cast<const VariableExprNode&>(*stmt.expression).name;
    } else if (stmt.expression->getType() == ASTNodeType::INTEGER_LITERAL) {
        std::string t = newTemp();
        int v = static_cast<const IntegerLiteralNode&>(*stmt.expression).value;
        ir_.addInstruction(IRInstruction::makeLoadConst(t, v));
        valName = t;
    } else {
        valName = generateExpression(*stmt.expression);
    }
    ir_.addInstruction(IRInstruction::makePrint(valName));
}

void IRGenerator::generateIf(const IfStmtNode& stmt) {
    std::string condTemp = generateComparisonExpr(*stmt.condition);

    if (stmt.elseBranch) {
        std::string elseLabel = newLabel();
        std::string endLabel = newLabel();

        // If condition is false, jump to elseLabel
        ir_.addInstruction(IRInstruction::makeJumpIfFalse(condTemp, elseLabel));

        // Then branch
        generateBlock(*stmt.thenBranch);
        ir_.addInstruction(IRInstruction::makeJump(endLabel));

        // Else branch
        ir_.addInstruction(IRInstruction::makeLabel(elseLabel));
        generateBlock(*stmt.elseBranch);

        // End label
        ir_.addInstruction(IRInstruction::makeLabel(endLabel));
    } else {
        std::string endLabel = newLabel();

        // If condition is false, jump past then branch
        ir_.addInstruction(IRInstruction::makeJumpIfFalse(condTemp, endLabel));

        // Then branch
        generateBlock(*stmt.thenBranch);

        // End label
        ir_.addInstruction(IRInstruction::makeLabel(endLabel));
    }
}

void IRGenerator::generateWhile(const WhileStmtNode& stmt) {
    std::string startLabel = newLabel();
    std::string endLabel = newLabel();

    // Start of loop
    ir_.addInstruction(IRInstruction::makeLabel(startLabel));

    // Condition check
    std::string condTemp = generateComparisonExpr(*stmt.condition);
    ir_.addInstruction(IRInstruction::makeJumpIfFalse(condTemp, endLabel));

    // Loop body
    generateBlock(*stmt.body);

    // Jump back to start
    ir_.addInstruction(IRInstruction::makeJump(startLabel));

    // Exit label
    ir_.addInstruction(IRInstruction::makeLabel(endLabel));
}

void IRGenerator::generateBlock(const BlockStmtNode& block) {
    for (const auto& s : block.statements) {
        generateStatement(*s);
    }
}

std::string IRGenerator::generateExpression(const ExprNode& expr) {
    switch (expr.getType()) {
        case ASTNodeType::INTEGER_LITERAL:
            return generateIntegerLiteral(static_cast<const IntegerLiteralNode&>(expr));
        case ASTNodeType::VARIABLE_EXPR:
            return generateVariableExpr(static_cast<const VariableExprNode&>(expr));
        case ASTNodeType::BINARY_EXPR:
            return generateBinaryExpr(static_cast<const BinaryExprNode&>(expr));
        case ASTNodeType::UNARY_EXPR:
            return generateUnaryExpr(static_cast<const UnaryExprNode&>(expr));
        case ASTNodeType::COMPARISON_EXPR:
            return generateComparisonExpr(static_cast<const ComparisonExprNode&>(expr));
        default:
            throw std::runtime_error("Unknown expression node in IR generation");
    }
}

std::string IRGenerator::generateIntegerLiteral(const IntegerLiteralNode& expr) {
    std::string t = newTemp();
    ir_.addInstruction(IRInstruction::makeLoadConst(t, expr.value));
    return t;
}

std::string IRGenerator::generateVariableExpr(const VariableExprNode& expr) {
    return expr.name;
}

std::string IRGenerator::generateBinaryExpr(const BinaryExprNode& expr) {
    std::string leftTemp = generateExpression(*expr.left);
    std::string rightTemp = generateExpression(*expr.right);
    std::string dest = newTemp();

    IROpcode op;
    switch (expr.op) {
        case TokenType::PLUS:  op = IROpcode::ADD; break;
        case TokenType::MINUS: op = IROpcode::SUB; break;
        case TokenType::STAR:  op = IROpcode::MUL; break;
        case TokenType::SLASH: op = IROpcode::DIV; break;
        default:
            throw std::runtime_error("Unsupported binary operator in IR generation");
    }

    ir_.addInstruction(IRInstruction::makeBinary(op, dest, leftTemp, rightTemp));
    return dest;
}

std::string IRGenerator::generateUnaryExpr(const UnaryExprNode& expr) {
    // Unary minus: -x is equivalent to 0 - x
    std::string zeroTemp = newTemp();
    ir_.addInstruction(IRInstruction::makeLoadConst(zeroTemp, 0));
    std::string operandTemp = generateExpression(*expr.operand);
    std::string dest = newTemp();
    ir_.addInstruction(IRInstruction::makeBinary(IROpcode::SUB, dest, zeroTemp, operandTemp));
    return dest;
}

std::string IRGenerator::generateComparisonExpr(const ComparisonExprNode& expr) {
    std::string leftTemp = generateExpression(*expr.left);
    std::string rightTemp = generateExpression(*expr.right);
    std::string dest = newTemp();

    IROpcode op;
    switch (expr.op) {
        case TokenType::EQUAL_EQUAL:   op = IROpcode::CMP_EQ; break;
        case TokenType::BANG_EQUAL:    op = IROpcode::CMP_NE; break;
        case TokenType::LESS:          op = IROpcode::CMP_LT; break;
        case TokenType::LESS_EQUAL:    op = IROpcode::CMP_LE; break;
        case TokenType::GREATER:       op = IROpcode::CMP_GT; break;
        case TokenType::GREATER_EQUAL: op = IROpcode::CMP_GE; break;
        default:
            throw std::runtime_error("Unsupported comparison operator in IR generation");
    }

    ir_.addInstruction(IRInstruction::makeBinary(op, dest, leftTemp, rightTemp));
    return dest;
}

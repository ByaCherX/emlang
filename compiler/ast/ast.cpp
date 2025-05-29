#include "../include/ast.h"
#include <sstream>

namespace emlang {

// ASTNode
ASTNode::ASTNode(ASTNodeType type, size_t line, size_t column)
    : type(type), line(line), column(column) {}

// Expression
Expression::Expression(ASTNodeType type, size_t line, size_t column)
    : ASTNode(type, line, column) {}

// Statement
Statement::Statement(ASTNodeType type, size_t line, size_t column)
    : ASTNode(type, line, column) {}

// LiteralExpression
LiteralExpression::LiteralExpression(LiteralType type, const std::string& value, size_t line, size_t column)
    : Expression(ASTNodeType::LITERAL, line, column), literalType(type), value(value) {}

std::string LiteralExpression::toString() const {
    std::string typeStr;
    switch (literalType) {
        case LiteralType::NUMBER: typeStr = "NUMBER"; break;
        case LiteralType::STRING: typeStr = "STRING"; break;
        case LiteralType::BOOLEAN: typeStr = "BOOLEAN"; break;
        case LiteralType::NULL_LITERAL: typeStr = "NULL"; break;
    }
    return "Literal(" + typeStr + ": " + value + ")";
}

void LiteralExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IdentifierExpression
IdentifierExpression::IdentifierExpression(const std::string& name, size_t line, size_t column)
    : Expression(ASTNodeType::IDENTIFIER, line, column), name(name) {}

std::string IdentifierExpression::toString() const {
    return "Identifier(" + name + ")";
}

void IdentifierExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BinaryOpExpression
BinaryOpExpression::BinaryOpExpression(ExpressionPtr left, const std::string& op, ExpressionPtr right, size_t line, size_t column)
    : Expression(ASTNodeType::BINARY_OP, line, column), left(std::move(left)), operator_(op), right(std::move(right)) {}

std::string BinaryOpExpression::toString() const {
    return "BinaryOp(" + left->toString() + " " + operator_ + " " + right->toString() + ")";
}

void BinaryOpExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// UnaryOpExpression
UnaryOpExpression::UnaryOpExpression(const std::string& op, ExpressionPtr operand, size_t line, size_t column)
    : Expression(ASTNodeType::UNARY_OP, line, column), operator_(op), operand(std::move(operand)) {}

std::string UnaryOpExpression::toString() const {
    return "UnaryOp(" + operator_ + operand->toString() + ")";
}

void UnaryOpExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FunctionCallExpression
FunctionCallExpression::FunctionCallExpression(const std::string& name, std::vector<ExpressionPtr> args, size_t line, size_t column)
    : Expression(ASTNodeType::FUNCTION_CALL, line, column), functionName(name), arguments(std::move(args)) {}

std::string FunctionCallExpression::toString() const {
    std::stringstream ss;
    ss << "FunctionCall(" << functionName << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    ss << "))";
    return ss.str();
}

void FunctionCallExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// VariableDeclaration
VariableDeclaration::VariableDeclaration(const std::string& name, const std::string& type, ExpressionPtr init, bool isConst, size_t line, size_t column)
    : Statement(ASTNodeType::VARIABLE_DECLARATION, line, column), name(name), type(type), initializer(std::move(init)), isConstant(isConst) {}

std::string VariableDeclaration::toString() const {
    std::string result = (isConstant ? "const " : "let ") + name;
    if (!type.empty()) {
        result += ": " + type;
    }
    if (initializer) {
        result += " = " + initializer->toString();
    }
    return "VarDecl(" + result + ")";
}

void VariableDeclaration::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FunctionDeclaration
FunctionDeclaration::FunctionDeclaration(const std::string& name, std::vector<Parameter> params, const std::string& retType, StatementPtr body, size_t line, size_t column)
    : Statement(ASTNodeType::FUNCTION_DECLARATION, line, column), name(name), parameters(std::move(params)), returnType(retType), body(std::move(body)) {}

std::string FunctionDeclaration::toString() const {
    std::stringstream ss;
    ss << "FunctionDecl(" << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i].name << ": " << parameters[i].type;
    }
    ss << ")";
    if (!returnType.empty()) {
        ss << ": " << returnType;
    }
    ss << " " << body->toString() << ")";
    return ss.str();
}

void FunctionDeclaration::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BlockStatement
BlockStatement::BlockStatement(std::vector<StatementPtr> stmts, size_t line, size_t column)
    : Statement(ASTNodeType::BLOCK_STATEMENT, line, column), statements(std::move(stmts)) {}

std::string BlockStatement::toString() const {
    std::stringstream ss;
    ss << "Block(";
    for (size_t i = 0; i < statements.size(); ++i) {
        if (i > 0) ss << "; ";
        ss << statements[i]->toString();
    }
    ss << ")";
    return ss.str();
}

void BlockStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IfStatement
IfStatement::IfStatement(ExpressionPtr cond, StatementPtr then, StatementPtr else_, size_t line, size_t column)
    : Statement(ASTNodeType::IF_STATEMENT, line, column), condition(std::move(cond)), thenBranch(std::move(then)), elseBranch(std::move(else_)) {}

std::string IfStatement::toString() const {
    std::string result = "If(" + condition->toString() + " then " + thenBranch->toString();
    if (elseBranch) {
        result += " else " + elseBranch->toString();
    }
    result += ")";
    return result;
}

void IfStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// WhileStatement
WhileStatement::WhileStatement(ExpressionPtr cond, StatementPtr body, size_t line, size_t column)
    : Statement(ASTNodeType::WHILE_STATEMENT, line, column), condition(std::move(cond)), body(std::move(body)) {}

std::string WhileStatement::toString() const {
    return "While(" + condition->toString() + " " + body->toString() + ")";
}

void WhileStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ReturnStatement
ReturnStatement::ReturnStatement(ExpressionPtr val, size_t line, size_t column)
    : Statement(ASTNodeType::RETURN_STATEMENT, line, column), value(std::move(val)) {}

std::string ReturnStatement::toString() const {
    if (value) {
        return "Return(" + value->toString() + ")";
    }
    return "Return()";
}

void ReturnStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ExpressionStatement
ExpressionStatement::ExpressionStatement(ExpressionPtr expr, size_t line, size_t column)
    : Statement(ASTNodeType::EXPRESSION_STMT, line, column), expression(std::move(expr)) {}

std::string ExpressionStatement::toString() const {
    return "ExprStmt(" + expression->toString() + ")";
}

void ExpressionStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Program
Program::Program(std::vector<StatementPtr> stmts)
    : ASTNode(ASTNodeType::PROGRAM), statements(std::move(stmts)) {}

std::string Program::toString() const {
    std::stringstream ss;
    ss << "Program(";
    for (size_t i = 0; i < statements.size(); ++i) {
        if (i > 0) ss << "; ";
        ss << statements[i]->toString();
    }
    ss << ")";
    return ss.str();
}

void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace emlang

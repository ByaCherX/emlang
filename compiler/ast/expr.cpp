//===--- expr.cpp - Expression implementations ------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Expression class implementations for EMLang AST
//
// This file contains the implementation of all Expression-derived classes.
//===----------------------------------------------------------------------===//

#include "ast/expr.h"
#include <sstream>

namespace emlang {

// LiteralExpression
LiteralExpr::LiteralExpr(LiteralType type, const std::string& value, size_t line, size_t column)
    : Expression(NodeType::LITERAL_EXPR, line, column), literalType(type), value(value) {}

std::string LiteralExpr::toString() const {
    std::string typeStr;
    switch (literalType) {
        case LiteralType::INT: typeStr = "INT"; break;
        case LiteralType::FLOAT: typeStr = "FLOAT"; break;
        case LiteralType::STR: typeStr = "STR"; break;
        case LiteralType::CHAR: typeStr = "CHAR"; break;
        case LiteralType::BOOL: typeStr = "BOOL"; break;
        case LiteralType::NULL_LITERAL: typeStr = "NULL"; break;
    }
    return "Literal(" + typeStr + ": " + value + ")";
}

void LiteralExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IdentifierExpression
IdentifierExpr::IdentifierExpr(const std::string& name, size_t line, size_t column)
    : Expression(NodeType::IDENTIFIER_EXPR, line, column), name(name) {}

std::string IdentifierExpr::toString() const {
    return "Identifier(" + name + ")";
}

void IdentifierExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BinaryOpExpression
BinaryOpExpr::BinaryOpExpr(ExpressionPtr left, const BinOp& op, ExpressionPtr right, size_t line, size_t column)
    : Expression(NodeType::BINARY_EXPR, line, column), left(std::move(left)), operator_(op), right(std::move(right)) {}

std::string BinaryOpExpr::toString() const {
    return "BinaryOp(" + left->toString() + " " + binOpToString(operator_) + " " + right->toString() + ")";
}

void BinaryOpExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// UnaryOpExpression
UnaryOpExpr::UnaryOpExpr(const BinaryOpExpr::BinOp& op, ExpressionPtr operand, size_t line, size_t column)
    : Expression(NodeType::UNARY_EXPR, line, column), operator_(op), operand(std::move(operand)) {}

std::string UnaryOpExpr::toString() const {
    return "UnaryOp(" + binOpToString(operator_) + operand->toString() + ")";
}

void UnaryOpExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// AssignmentExpression
AssignmentExpr::AssignmentExpr(ExpressionPtr target, ExpressionPtr value, size_t line, size_t column)
    : Expression(NodeType::ASSIGNMENT_EXPR, line, column), target(std::move(target)), value(std::move(value)) {}

std::string AssignmentExpr::toString() const {
    return "Assignment(" + target->toString() + " = " + value->toString() + ")";
}

void AssignmentExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FunctionCallExpression
FunctionCallExpr::FunctionCallExpr(const std::string& name, std::vector<ExpressionPtr> args, size_t line, size_t column)
    : Expression(NodeType::FUNCTION_CALL, line, column), functionName(name), arguments(std::move(args)) {}

std::string FunctionCallExpr::toString() const {
    std::stringstream ss;
    ss << "FunctionCall(" << functionName << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    ss << "))";
    return ss.str();
}

void FunctionCallExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// MemberExpression
MemberExpr::MemberExpr(ExpressionPtr object, const std::string& memberName, bool isMethodCall, size_t line, size_t column)
    : Expression(NodeType::MEMBER_EXPR, line, column), object(std::move(object)), memberName(memberName), isMethodCall(isMethodCall) {}

std::string MemberExpr::toString() const {
    return "MemberAccess(" + object->toString() + "." + memberName + (isMethodCall ? "()" : "") + ")";
}

void MemberExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

#ifdef EMLANG_FEATURE_CASTING
// CastExpression
CastExpr::CastExpr(ExpressionPtr operand, const std::string& targetType, bool isExplicit, size_t line, size_t column)
    : Expression(NodeType::CAST_EXPR, line, column), operand(std::move(operand)), targetType(targetType), isExplicit(isExplicit) {}

std::string CastExpr::toString() const {
    return "Cast(" + operand->toString() + " as " + targetType + ")";
}

void CastExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
#endif // EMLANG_FEATURE_CASTING

// IndexExpression
IndexExpr::IndexExpr(ExpressionPtr array, ExpressionPtr index, size_t line, size_t column)
    : Expression(NodeType::INDEX_EXPR, line, column), array(std::move(array)), index(std::move(index)) {}

std::string IndexExpr::toString() const {
    return "Index(" + array->toString() + "[" + index->toString() + "])";
}

void IndexExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ArrayExpression
ArrayExpr::ArrayExpr(std::vector<ExpressionPtr> elements, size_t line, size_t column)
    : Expression(NodeType::ARRAY_EXPR, line, column), elements(std::move(elements)) {}

std::string ArrayExpr::toString() const {
    std::stringstream ss;
    ss << "Array([";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    ss << "])";
    return ss.str();
}

void ArrayExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ObjectExpression
ObjectExpr::ObjectExpr(std::vector<ObjectField> fields, size_t line, size_t column)
    : Expression(NodeType::OBJECT_EXPR, line, column), fields(std::move(fields)) {}

std::string ObjectExpr::toString() const {
    std::stringstream ss;
    ss << "Object({";
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << fields[i].key << ": " << fields[i].value->toString();
    }
    ss << "})";
    return ss.str();
}

void ObjectExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

#ifdef EMLANG_FEATURE_POINTERS
// DereferenceExpression
DereferenceExpr::DereferenceExpr(ExpressionPtr operand, size_t line, size_t column)
    : Expression(NodeType::DEREFERENCE, line, column), operand(std::move(operand)) {}

std::string DereferenceExpr::toString() const {
    return "Dereference(*" + operand->toString() + ")";
}

void DereferenceExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// AddressOfExpression
AddressOfExpr::AddressOfExpr(ExpressionPtr operand, size_t line, size_t column)
    : Expression(NodeType::ADDRESS_OF, line, column), operand(std::move(operand)) {}

std::string AddressOfExpr::toString() const {
    return "AddressOf(&" + operand->toString() + ")";
}

void AddressOfExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
#endif // EMLANG_FEATURE_POINTERS

// ========================================================
// Helper functions
// ========================================================
std::string binOpToString(BinaryOpExpr::BinOp op) {
    switch (op) {
        case BinaryOpExpr::BinOp::ADD: return "+";
        case BinaryOpExpr::BinOp::SUB: return "-";
        case BinaryOpExpr::BinOp::MUL: return "*";
        case BinaryOpExpr::BinOp::DIV: return "/";
        case BinaryOpExpr::BinOp::MOD: return "%";
        case BinaryOpExpr::BinOp::AND: return "&";
        case BinaryOpExpr::BinOp::OR: return "|";
        case BinaryOpExpr::BinOp::XOR: return "^";
        case BinaryOpExpr::BinOp::INV: return "~";
        case BinaryOpExpr::BinOp::SHL: return "<<";
        case BinaryOpExpr::BinOp::SHR: return ">>";
        case BinaryOpExpr::BinOp::EQ: return "==";
        case BinaryOpExpr::BinOp::NE: return "!=";
        case BinaryOpExpr::BinOp::LT: return "<";
        case BinaryOpExpr::BinOp::LE: return "<=";
        case BinaryOpExpr::BinOp::GT: return ">";
        case BinaryOpExpr::BinOp::GE: return ">=";
        case BinaryOpExpr::BinOp::LAND: return "&&";
        case BinaryOpExpr::BinOp::LOR: return "||";
        case BinaryOpExpr::BinOp::LNOT: return "!";
    }
    return "";
}

} // namespace emlang

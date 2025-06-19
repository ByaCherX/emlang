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
    : Expression(ASTNodeType::LITERAL, line, column), literalType(type), value(value) {}

std::string LiteralExpr::toString() const {
    std::string typeStr;
    switch (literalType) {
        case LiteralType::NUMBER: typeStr = "NUMBER"; break;
        case LiteralType::STRING: typeStr = "STRING"; break;
        case LiteralType::CHAR: typeStr = "CHAR"; break;
        case LiteralType::BOOLEAN: typeStr = "BOOLEAN"; break;
        case LiteralType::NULL_LITERAL: typeStr = "NULL"; break;
    }
    return "Literal(" + typeStr + ": " + value + ")";
}

void LiteralExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IdentifierExpression
IdentifierExpr::IdentifierExpr(const std::string& name, size_t line, size_t column)
    : Expression(ASTNodeType::IDENTIFIER, line, column), name(name) {}

std::string IdentifierExpr::toString() const {
    return "Identifier(" + name + ")";
}

void IdentifierExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BinaryOpExpression
BinaryOpExpr::BinaryOpExpr(ExpressionPtr left, const std::string& op, ExpressionPtr right, size_t line, size_t column)
    : Expression(ASTNodeType::BINARY_OP, line, column), left(std::move(left)), operator_(op), right(std::move(right)) {}

std::string BinaryOpExpr::toString() const {
    return "BinaryOp(" + left->toString() + " " + operator_ + " " + right->toString() + ")";
}

void BinaryOpExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// UnaryOpExpression
UnaryOpExpr::UnaryOpExpr(const std::string& op, ExpressionPtr operand, size_t line, size_t column)
    : Expression(ASTNodeType::UNARY_OP, line, column), operator_(op), operand(std::move(operand)) {}

std::string UnaryOpExpr::toString() const {
    return "UnaryOp(" + operator_ + operand->toString() + ")";
}

void UnaryOpExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// AssignmentExpression
AssignmentExpr::AssignmentExpr(ExpressionPtr target, ExpressionPtr value, size_t line, size_t column)
    : Expression(ASTNodeType::ASSIGNMENT, line, column), target(std::move(target)), value(std::move(value)) {}

std::string AssignmentExpr::toString() const {
    return "Assignment(" + target->toString() + " = " + value->toString() + ")";
}

void AssignmentExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FunctionCallExpression
FunctionCallExpr::FunctionCallExpr(const std::string& name, std::vector<ExpressionPtr> args, size_t line, size_t column)
    : Expression(ASTNodeType::FUNCTION_CALL, line, column), functionName(name), arguments(std::move(args)) {}

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

// DereferenceExpression
DereferenceExpr::DereferenceExpr(ExpressionPtr operand, size_t line, size_t column)
    : Expression(ASTNodeType::DEREFERENCE, line, column), operand(std::move(operand)) {}

std::string DereferenceExpr::toString() const {
    return "Dereference(*" + operand->toString() + ")";
}

void DereferenceExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// AddressOfExpression
AddressOfExpr::AddressOfExpr(ExpressionPtr operand, size_t line, size_t column)
    : Expression(ASTNodeType::ADDRESS_OF, line, column), operand(std::move(operand)) {}

std::string AddressOfExpr::toString() const {
    return "AddressOf(&" + operand->toString() + ")";
}

void AddressOfExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace emlang

//===--- stmt.cpp - Statement implementations -------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Statement class implementations for EMLang AST
//
// This file contains the implementation of all Statement-derived classes.
//===----------------------------------------------------------------------===//

#include "ast/stmt.h"
#include <sstream>

namespace emlang {

// BlockStatement
BlockStmt::BlockStmt(std::vector<StatementPtr> stmts, size_t line, size_t column)
    : Statement(ASTNodeType::BLOCK_STATEMENT, line, column), statements(std::move(stmts)) {}

std::string BlockStmt::toString() const {
    std::stringstream ss;
    ss << "Block(";
    for (size_t i = 0; i < statements.size(); ++i) {
        if (i > 0) ss << "; ";
        ss << statements[i]->toString();
    }
    ss << ")";
    return ss.str();
}

void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IfStatement
IfStmt::IfStmt(ExpressionPtr cond, StatementPtr then, StatementPtr else_, size_t line, size_t column)
    : Statement(ASTNodeType::IF_STATEMENT, line, column), 
      condition(std::move(cond)), 
      thenBranch(std::move(then)), 
      elseBranch(std::move(else_)) {}

std::string IfStmt::toString() const {
    std::string result = "If(" + condition->toString() + " then " + thenBranch->toString();
    if (elseBranch) {
        result += " else " + elseBranch->toString();
    }
    result += ")";
    return result;
}

void IfStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// WhileStatement
WhileStmt::WhileStmt(ExpressionPtr cond, StatementPtr body, size_t line, size_t column)
    : Statement(ASTNodeType::WHILE_STATEMENT, line, column), 
      condition(std::move(cond)), 
      body(std::move(body)) {}

std::string WhileStmt::toString() const {
    return "While(" + condition->toString() + " " + body->toString() + ")";
}

void WhileStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ReturnStatement
ReturnStmt::ReturnStmt(ExpressionPtr val, size_t line, size_t column)
    : Statement(ASTNodeType::RETURN_STATEMENT, line, column), value(std::move(val)) {}

std::string ReturnStmt::toString() const {
    if (value) {
        return "Return(" + value->toString() + ")";
    }
    return "Return()";
}

void ReturnStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ExpressionStatement
ExpressionStmt::ExpressionStmt(ExpressionPtr expr, size_t line, size_t column)
    : Statement(ASTNodeType::EXPRESSION_STMT, line, column), expression(std::move(expr)) {}

std::string ExpressionStmt::toString() const {
    return "ExprStmt(" + expression->toString() + ")";
}

void ExpressionStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace emlang

//===--- ast.cpp - AST Base Classes -----------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Base AST node implementations
//===----------------------------------------------------------------------===//

#include "ast/ast_base.h"
#include "ast/visitor.h"
#include <sstream>

namespace emlang {

// ASTNode base class implementation
ASTNode::ASTNode(NodeType type, size_t line, size_t column)
    : type(type), line(line), column(column) {}

// Expression base class implementation
Expression::Expression(NodeType type, size_t line, size_t column)
    : ASTNode(type, line, column) {}

// Statement base class implementation
Statement::Statement(NodeType type, size_t line, size_t column)
    : ASTNode(type, line, column) {}

// Program class implementation
Program::Program(std::vector<StatementPtr> stmts) : 
    ASTNode(NodeType::PROGRAM, 0, 0), 
    statements(std::move(stmts)) {}

std::string Program::toString() const {
    std::stringstream ss;
    ss << "Program {\n";
    for (const auto& stmt : statements) {
        if (stmt) {
            ss << "  " << stmt->toString() << "\n";
        }
    }
    ss << "}";
    return ss.str();
}

void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace emlang

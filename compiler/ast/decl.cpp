//===--- decl.cpp - Declaration implementations -----------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declaration class implementations for EMLang AST
//
// This file contains the implementation of all Declaration-derived classes.
//===----------------------------------------------------------------------===//

#include "ast/decl.h"
#include <sstream>

namespace emlang {

// VariableDeclaration
VariableDecl::VariableDecl(
    const std::string& name, 
    const std::string& type, 
    ExpressionPtr init, 
    bool isConst, 
    size_t line, 
    size_t column
) : Statement(NodeType::VARIABLE_DECL, line, column), 
    name(name), 
    type(type), 
    initializer(std::move(init)), 
    isConstant(isConst) {}

std::string VariableDecl::toString() const {
    std::string result = (isConstant ? "const " : "let ") + name;
    if (type.has_value() && !type->empty()) {
        result += ": " + type.value();
    }
    if (initializer) {
        result += " = " + initializer->toString();
    }
    return "VarDecl(" + result + ")";
}

void VariableDecl::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// FunctionDeclaration
FunctionDecl::FunctionDecl(
    const std::string& name, 
    std::vector<Parameter> params, 
    const std::string& retType, 
    StatementPtr body, 
    bool Extern,
    bool Async,
    bool Unsafe,
    size_t line, 
    size_t column
) : Statement(NodeType::FUNCTION_DECL, line, column), 
    name(name), 
    parameters(std::move(params)), 
    returnType(retType), 
    body(std::move(body)),
    isExtern(Extern),
    isAsync(Async),
    isUnsafe(Unsafe) {}

std::string FunctionDecl::toString() const {
    std::stringstream ss;
    ss << "FunctionDecl(" << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i].name << ": " << parameters[i].type;
    }
    ss << ")";
    if (!returnType->empty()) {
        ss << ": " << returnType.value();
    }
    ss << " " << body->toString() << ")";
    return ss.str();
}

void FunctionDecl::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ExternFunctionDeclaration
ExternFunctionDecl::ExternFunctionDecl(
    const std::string& name, 
    std::vector<Parameter> params, 
    const std::string& retType, 
    size_t line, 
    size_t column
) : Statement(NodeType::EXTERN_FN_DECL, line, column), 
    name(name), 
    parameters(std::move(params)), 
    returnType(retType) {}

std::string ExternFunctionDecl::toString() const {
    std::stringstream ss;
    ss << "ExternFunctionDecl(extern " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i].name << ": " << parameters[i].type;
    }
    ss << ")";
    if (!returnType.empty()) {
        ss << ": " << returnType;
    }
    ss << ")";
    return ss.str();
}

void ExternFunctionDecl::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace emlang

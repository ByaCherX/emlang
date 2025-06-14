//===--- decl.h - Declaration AST nodes -------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declaration AST node declarations
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_DECL_H
#define EM_LANG_AST_DECL_H

#pragma once

#include "ast_base.h"
#include "visitor.h"

namespace emlang {

/**
 * @class VariableDeclaration
 * @brief Represents variable declarations (let/const)
 */
class EMLANG_API VariableDecl : public Statement {
public:
    std::string name;           // Variable name
    std::string type;           // Variable type (optional)
    ExpressionPtr initializer;  // Optional initializer expression
    bool isConstant;            // true for const, false for let
    
    VariableDecl(const std::string& name, const std::string& type, ExpressionPtr init = nullptr, bool isConst = false, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator=(const VariableDecl&) = delete;
    
    // Enable move constructor and assignment operator
    VariableDecl(VariableDecl&&) = default;
    VariableDecl& operator=(VariableDecl&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class FunctionDeclaration
 * @brief Represents function declarations
 */
class EMLANG_API FunctionDecl : public Statement {
public:
    std::string name;                   // Function name
    std::vector<Parameter> parameters;  // Function parameters
    std::string returnType;             // Return type (optional)
    StatementPtr body;                  // Function body
    
    FunctionDecl(const std::string& name, std::vector<Parameter> params, const std::string& retType, StatementPtr body, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator=(const FunctionDecl&) = delete;
    
    // Enable move constructor and assignment operator
    FunctionDecl(FunctionDecl&&) = default;
    FunctionDecl& operator=(FunctionDecl&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ExternFunctionDeclaration
 * @brief Represents external function declarations
 */
class EMLANG_API ExternFunctionDecl : public Statement {
public:
    std::string name;                   // Function name
    std::vector<Parameter> parameters;  // Function parameters
    std::string returnType;             // Return type (optional)
    
    ExternFunctionDecl(const std::string& name, std::vector<Parameter> params, const std::string& retType, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

} // namespace emlang

#endif // EM_LANG_AST_DECL_H

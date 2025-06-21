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
#include <optional>

namespace emlang {

/**
 * @class VariableDeclaration
 * @brief Represents variable declarations (let/const)
 */
class VariableDecl : public Statement {
public:
    std::string name;                // Variable name
    std::optional<std::string> type; // Variable type (optional)
    ExpressionPtr initializer;       // Optional initializer expression
    bool isConstant;                 // true for const, false for let
    
    VariableDecl(
        const std::string& name, 
        const std::string& type, 
        ExpressionPtr init = nullptr, 
        bool isConst = false, 
        size_t line = 0, size_t column = 0);
    
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
 * IMPLEMENT: FunctionDecl -> FnDecl, combine with extern function decl
 */
class FunctionDecl : public Statement {
public:
    std::string name;                      // Function name
    std::vector<Parameter> parameters;     // Function parameters
    std::optional<std::string> returnType; // Return type (optional)
    StatementPtr body;                     // Function body
    bool isExtern;                         // true if this is an extern function declaration
    bool isAsync;                          // true if this is an async function declaration
    bool isUnsafe;                         // true if this is an unsafe function declaration
    std::optional<std::string> abi;        // ABI name for the function (optional)
    
    FunctionDecl(
        const std::string& name, 
        std::vector<Parameter> params, 
        const std::string& retType, 
        StatementPtr body, 
        bool isExtern = false, 
        bool isAsync = false, 
        bool isUnsafe = false, 
        size_t line = 0, size_t column = 0);
    
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
class ExternFunctionDecl : public Statement {
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

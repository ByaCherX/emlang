//===--- CGDecl.h - Declaration Code Generation ------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declaration code generation for EMLang
//
// This file contains the CGDecl class responsible for generating LLVM IR
// for all declaration AST nodes in EMLang.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CGDECL_H
#define EM_CODEGEN_CGDECL_H

#pragma once

#include <emlang_export.h>
#include "CGBase.h"
#include "context.h"
#include "value_map.h"
#include "codegen_error.h"
#include <memory>
#include <string>

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>


namespace emlang {

// Forward declarations for declaration AST nodes
class VariableDecl;
class FunctionDecl;
class ExternFunctionDecl;

namespace codegen {

/**
 * @class CGDecl
 * @brief Declaration code generator implementing visitor pattern
 * 
 * CGDecl handles the conversion of all declaration AST nodes to LLVM IR
 * by implementing the visitor pattern. It inherits from CGBase and overrides
 * only the declaration-related visitor methods.
 */
class CGDecl : public CGBase {
private:
    llvm::Function* currentFunction;
    std::string currentExpressionType; // Tracks the type of the current expression

public:
    /******************** Constructor ********************/

    /**
     * @brief Constructs a new CGDecl instance
     * @param contextManager Reference to the LLVM context manager
     * @param valueMap Reference to the value mapping system
     * @param errorReporter Reference to the error reporting system
     */
    CGDecl(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter);

    /**
     * @brief Default destructor
     */
    ~CGDecl() = default;

    /******************** Visitor ********************/

    // Declaration visitors - full implementations
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(ExternFunctionDecl& node) override;

    /******************** Utility Methods ********************/

    /**
     * @brief Gets the current function being processed
     * @return Current LLVM function
     */
    llvm::Function* getCurrentFunction() const { return currentFunction; }

    /**
     * @brief Sets the current function
     * @param function LLVM function to set as current
     */
    void setCurrentFunction(llvm::Function* function) { currentFunction = function; }

    /**
     * @brief Sets the current value and type
     * @param value LLVM value to set
     * @param type Expression type to set
     */
    void setCurrentValue(llvm::Value* value, const std::string& type) override;

private:
    /******************** Generation Methods ********************/

    /**
     * @brief Generates LLVM IR for a variable declaration
     * @param node The variable declaration AST node
     * @return Generated LLVM value (alloca instruction)
     */
    llvm::Value* generateVariableDecl(VariableDecl& node);

    /**
     * @brief Generates LLVM IR for a function declaration
     * @param node The function declaration AST node
     * @return Generated LLVM function
     */
    llvm::Function* generateFunctionDecl(FunctionDecl& node);

    /**
     * @brief Generates LLVM IR for an external function declaration
     * @param node The external function declaration AST node
     * @return Generated LLVM function
     */
    llvm::Function* generateExternFunctionDecl(ExternFunctionDecl& node);

    /******************** Helper Methods ********************/

    /**
     * @brief Creates function parameters and adds them to the value map
     * @param function LLVM function to process
     * @param node Function declaration AST node
     */
    void createFunctionParameters(llvm::Function* function, FunctionDecl& node);

    /**
     * @brief Validates function signature compatibility
     * @param node Function declaration AST node
     * @return true if valid, false otherwise
     */
    bool validateFunctionSignature(FunctionDecl& node);

    /**
     * @brief Validates external function declaration
     * @param node External function declaration AST node
     * @return true if valid, false otherwise
     */
    bool validateExternFunctionDecl(ExternFunctionDecl& node);
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_CGDECL_H

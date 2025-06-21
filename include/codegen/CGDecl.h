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
#include "context.h"
#include "value_map.h"
#include "codegen_error.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <memory>
#include <string>

namespace emlang {

// Forward declarations for declaration AST nodes
class VariableDecl;
class FunctionDecl;
class ExternFunctionDecl;

namespace codegen {

/**
 * @class CGDecl
 * @brief Code generator for declaration AST nodes
 * 
 * CGDecl handles the conversion of all declaration AST nodes to LLVM IR.
 * It manages variable declarations, function declarations, external function
 * declarations, and maintains proper symbol table integration.
 */
class EMLANG_API CGDecl {
private:
    ContextManager& contextManager;
    ValueMap& valueMap;
    CodegenErrorReporter& errorReporter;

    llvm::Value* currentValue;
    llvm::Function* currentFunction;

public:
    // ======================== CONSTRUCTION ========================

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

    // ======================== DECLARATION GENERATION ========================

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

    // ======================== UTILITY METHODS ========================

    /**
     * @brief Gets the current generated value
     * @return Current LLVM value
     */
    llvm::Value* getCurrentValue() const { return currentValue; }

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
     * @brief Sets the current value
     * @param value LLVM value to set
     */
    void setCurrentValue(llvm::Value* value) { currentValue = value; }

private:
    // ======================== HELPER METHODS ========================

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

    // ======================== ERROR HANDLING ========================

    /**
     * @brief Reports an error and sets currentValue to nullptr
     * @param message Error message
     */
    void error(const std::string& message);

    /**
     * @brief Reports a typed error with context
     * @param type Error type
     * @param message Error message
     * @param context Optional context
     */
    void error(CodegenErrorType type, const std::string& message, const std::string& context = "");
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_CGDECL_H

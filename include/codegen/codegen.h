//===--- codegen.h - Main CodeGen interface ---------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main code generation interface for EMLang
//
// This file contains the main CodeGenerator class that orchestrates
// the translation from EMLang AST to LLVM IR using modular components.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_H
#define EM_CODEGEN_H

#pragma once

#include <emlang_export.h>
#include "ast.h"
#include "context.h"
#include "value_map.h"
#include "codegen_error.h"
#include "builtins_integration.h"
#include "aot_compiler.h"
#include "CGExpr.h"
#include "CGDecl.h"
#include "CGStmt.h"
#include "CGBase.h"
// Backend includes temporarily disabled until implementation is complete  
// #include "aot_compiler.h"
// #include "jit/jit_engine.h"
#include <memory>
#include <string>

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Error.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif


namespace emlang {
namespace codegen {

// Forward declarations
std::string toString(llvm::Error err);

/**
 * @class CodeGenerator
 * @brief Main code generator that orchestrates LLVM IR generation
 * 
 * CodeGenerator is the primary class responsible for coordinating EMLang's
 * Abstract Syntax Tree conversion to LLVM Intermediate Representation. It uses
 * modular components for expression, declaration, and statement generation,
 * along with value mapping, LLVM context management, and error handling.
 */
class EMLANG_API CodeGenerator {
private:
    std::unique_ptr<codegen::ContextManager> contextManager;
    std::unique_ptr<codegen::ValueMap> valueMap;
    std::unique_ptr<CodegenErrorReporter> errorReporter;

    // Code generation components
    std::unique_ptr<CGExpr> exprGenerator;
    std::unique_ptr<CGDecl> declGenerator;
    std::unique_ptr<CGStmt> stmtGenerator;
    std::unique_ptr<CGBase> programGenerator;    
      
    // Backend components
    std::unique_ptr<AOTCompiler> aotBackend;
    // JIT backend still experimental
    // std::unique_ptr<jit::JITEngine> jitBackend;

    llvm::Value* currentValue;
    std::string currentExpressionType;

    llvm::Function* currentFunction;

public:
    /******************** CONSTRUCTION AND LIFECYCLE ********************/

    /**
     * @brief Constructs a new CodeGenerator instance
     * @param moduleName Name for the LLVM module
     * @param optLevel Optimization level to apply
     */
    CodeGenerator(const std::string& moduleName);

    /**
     * @brief Default destructor
     */
    ~CodeGenerator() = default;

    /******************** PRIMARY CODE GENERATION ********************/

    /**
     * @brief Generates LLVM IR from an EMLang program AST
     * @param program Root AST node representing the entire program
     */
    void generateIR(Program& program);

    /**
     * @brief Prints the generated LLVM IR to standard output
     *
     * The output includes all functions, global variables, type definitions,
     * and metadata in LLVM's textual IR format.
     */
    void printIR() const;

    /******************** EXECUTION ********************/
    // JIT not implemented yet  
    
    /******************** BACKEND MANAGEMENT ********************/
    // Backend functionality temporarily disabled until implementation is complete

    /**
     * @brief Compiles using AOT backend (temporarily disabled)
     */
    bool compileAOT(const std::string& outputPath);

    /** @brief Initializes AOT backend */
    bool initializeAOTBackend();

    /******************** COMPONENT ACCESS ********************/

    /** @brief Gets the expression generator */
    CGExpr& getExprGenerator() { return *exprGenerator; }

    /** @brief Gets the declaration generator */
    CGDecl& getDeclGenerator() { return *declGenerator; }

    /** @brief Gets the statement generator */
    CGStmt& getStmtGenerator() { return *stmtGenerator; }

    /** @brief Gets the context manager */
    ContextManager& getContextManager() { return *contextManager; }

    /** @brief Gets the value map */
    ValueMap& getValueMap() { return *valueMap; }

private:
    /******************** ERROR HANDLING ********************/

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

    /**
     * @brief Gets the error reporter
     * @return Reference to the error reporter
     */
    CodegenErrorReporter& getErrorReporter() { return *errorReporter; }

    /**
     * @brief Checks if any errors occurred during code generation
     * @return true if errors exist, false otherwise
     */
    bool hasErrors() const;

    /******************** UTILITY METHODS ********************/
    std::string toString(llvm::Error err);
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_H

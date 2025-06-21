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
#include "value_map.h"
#include "context.h"
#include "codegen_error.h"
#include "builtins_integration.h"
#include "CGExpr.h"
#include "CGDecl.h"
#include "CGStmt.h"
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

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif


namespace emlang {
namespace codegen {

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

    llvm::Value* currentValue;
    std::string currentExpressionType;

    llvm::Function* currentFunction;

public:
    // ======================== CONSTRUCTION AND LIFECYCLE ========================

    /**
     * @brief Constructs a new CodeGenerator instance
     * @param moduleName Name for the LLVM module
     * @param optLevel Optimization level to apply
     */
    CodeGenerator(const std::string& moduleName, codegen::OptimizationLevel optLevel = codegen::OptimizationLevel::None);

    /**
     * @brief Default destructor
     */
    ~CodeGenerator() = default;

    // ======================== PRIMARY CODE GENERATION ========================

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

    // ======================== EXECUTION ========================

    /**
     * @brief JIT compiles and executes the main function
     * @return Exit code returned by the main function
     */
    int executeMain();

    /**
     * @brief Generates code object file from the LLVM IR
     * @param filename Path to the output object file (typically .o extension)
     *
     * Compiles the LLVM IR to native machine code and writes it as an
     * object file suitable for linking. The process includes:
     * - Target machine configuration
     * - Instruction selection and scheduling
     * - Register allocation
     * - Assembly generation
     * - Object file format generation
     *
     * The generated object files can be linked with standard system linkers
     * to create executable programs or shared libraries.
     */
    void writeCodeToFile(const std::string& filename, bool emitLLVM = false);

    // ======================== ERROR HANDLING ========================

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

    // ======================== COMPONENT ACCESS ========================

    /**
     * @brief Gets the expression generator
     * @return Reference to the expression generator
     */
    CGExpr& getExprGenerator() { return *exprGenerator; }

    /**
     * @brief Gets the declaration generator
     * @return Reference to the declaration generator
     */
    CGDecl& getDeclGenerator() { return *declGenerator; }

    /**
     * @brief Gets the statement generator
     * @return Reference to the statement generator
     */
    CGStmt& getStmtGenerator() { return *stmtGenerator; }

    /**
     * @brief Gets the context manager
     * @return Reference to the context manager
     */
    ContextManager& getContextManager() { return *contextManager; }

    /**
     * @brief Gets the value map
     * @return Reference to the value map
     */
    ValueMap& getValueMap() { return *valueMap; }

private:
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

#endif // EM_CODEGEN_H

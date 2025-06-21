//===--- CGStmt.h - Statement Code Generation -------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Statement code generation for EMLang
//
// This file contains the CGStmt class responsible for generating LLVM IR
// for all statement AST nodes in EMLang.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CGSTMT_H
#define EM_CODEGEN_CGSTMT_H

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
#include <llvm/IR/BasicBlock.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <memory>
#include <string>

namespace emlang {

// Forward declarations for statement AST nodes
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class ExpressionStmt;

namespace codegen {

// Forward declaration for expression generator
class CGExpr;

/**
 * @class CGStmt
 * @brief Code generator for statement AST nodes
 * 
 * CGStmt handles the conversion of all statement AST nodes to LLVM IR.
 * It manages control flow, block statements, conditional statements,
 * loops, and return statements while maintaining proper basic block
 * organization.
 */
class EMLANG_API CGStmt {
private:
    ContextManager& contextManager;
    ValueMap& valueMap;
    CodegenErrorReporter& errorReporter;
    CGExpr& exprGenerator;

    llvm::Value* currentValue;
    llvm::BasicBlock* currentBlock;

public:
    // ======================== CONSTRUCTION ========================

    /**
     * @brief Constructs a new CGStmt instance
     * @param contextManager Reference to the LLVM context manager
     * @param valueMap Reference to the value mapping system
     * @param errorReporter Reference to the error reporting system
     * @param exprGenerator Reference to the expression generator
     */
    CGStmt(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter, CGExpr& exprGenerator);

    /**
     * @brief Default destructor
     */
    ~CGStmt() = default;

    // ======================== STATEMENT GENERATION ========================

    /**
     * @brief Generates LLVM IR for a block statement
     * @param node The block statement AST node
     * @return Generated LLVM basic block
     */
    llvm::BasicBlock* generateBlock(BlockStmt& node);

    /**
     * @brief Generates LLVM IR for an if statement
     * @param node The if statement AST node
     * @return Last generated LLVM basic block
     */
    llvm::BasicBlock* generateIf(IfStmt& node);

    /**
     * @brief Generates LLVM IR for a while loop statement
     * @param node The while statement AST node
     * @return Last generated LLVM basic block
     */
    llvm::BasicBlock* generateWhile(WhileStmt& node);

    /**
     * @brief Generates LLVM IR for a for loop statement
     * @param node The for statement AST node
     * @return Last generated LLVM basic block
     */
    llvm::BasicBlock* generateFor(ForStmt& node);

    /**
     * @brief Generates LLVM IR for a return statement
     * @param node The return statement AST node
     * @return Generated LLVM value (return instruction)
     */
    llvm::Value* generateReturn(ReturnStmt& node);

    /**
     * @brief Generates LLVM IR for an expression statement
     * @param node The expression statement AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateExpressionStmt(ExpressionStmt& node);

    // ======================== UTILITY METHODS ========================

    /**
     * @brief Gets the current generated value
     * @return Current LLVM value
     */
    llvm::Value* getCurrentValue() const { return currentValue; }

    /**
     * @brief Gets the current basic block
     * @return Current LLVM basic block
     */
    llvm::BasicBlock* getCurrentBlock() const { return currentBlock; }

    /**
     * @brief Sets the current basic block
     * @param block LLVM basic block to set as current
     */
    void setCurrentBlock(llvm::BasicBlock* block) { currentBlock = block; }

    /**
     * @brief Sets the current value
     * @param value LLVM value to set
     */
    void setCurrentValue(llvm::Value* value) { currentValue = value; }

private:
    // ======================== HELPER METHODS ========================

    /**
     * @brief Creates a new basic block in the current function
     * @param name Name for the basic block
     * @return Created LLVM basic block
     */
    llvm::BasicBlock* createBasicBlock(const std::string& name);

    /**
     * @brief Ensures the current basic block is properly terminated
     * @param block Basic block to check and terminate if needed
     */
    void ensureBlockTermination(llvm::BasicBlock* block);

    /**
     * @brief Validates control flow for conditional statements
     * @param conditionValue LLVM value representing the condition
     * @return true if valid, false otherwise
     */
    bool validateCondition(llvm::Value* conditionValue);

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

#endif // EM_CODEGEN_CGSTMT_H

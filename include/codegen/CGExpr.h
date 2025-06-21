//===--- CGExpr.h - Expression Code Generation ------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Expression code generation for EMLang
//
// This file contains the CGExpr class responsible for generating LLVM IR
// for all expression AST nodes in EMLang.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CGEXPR_H
#define EM_CODEGEN_CGEXPR_H

#pragma once

#include <emlang_export.h>
#include "context.h"
#include "value_map.h"
#include "codegen_error.h"
#include <llvm/IR/Value.h>
#include <memory>
#include <string>

namespace emlang {

// Forward declarations for expression AST nodes
class LiteralExpr;
class IdentifierExpr;
class BinaryOpExpr;
class UnaryOpExpr;
class AssignmentExpr;
class FunctionCallExpr;
class MemberExpr;
class CastExpr;
class IndexExpr;
class ArrayExpr;
class ObjectExpr;
class DereferenceExpr;
class AddressOfExpr;

namespace codegen {

/**
 * @class CGExpr
 * @brief Code generator for expression AST nodes
 * 
 * CGExpr handles the conversion of all expression AST nodes to LLVM IR.
 * It manages value generation, type inference, and expression evaluation
 * while maintaining proper integration with the broader code generation context.
 */
class EMLANG_API CGExpr {
private:
    ContextManager& contextManager;
    ValueMap& valueMap;
    CodegenErrorReporter& errorReporter;

    llvm::Value* currentValue;
    std::string currentExpressionType;

public:
    // ======================== CONSTRUCTION ========================

    /**
     * @brief Constructs a new CGExpr instance
     * @param contextManager Reference to the LLVM context manager
     * @param valueMap Reference to the value mapping system
     * @param errorReporter Reference to the error reporting system
     */
    CGExpr(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter);

    /**
     * @brief Default destructor
     */
    ~CGExpr() = default;

    // ======================== EXPRESSION GENERATION ========================

    /**
     * @brief Generates LLVM IR for a literal expression
     * @param node The literal expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateLiteral(LiteralExpr& node);

    /**
     * @brief Generates LLVM IR for an identifier expression
     * @param node The identifier expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateIdentifier(IdentifierExpr& node);

    /**
     * @brief Generates LLVM IR for a binary operation expression
     * @param node The binary operation expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateBinaryOp(BinaryOpExpr& node);

    /**
     * @brief Generates LLVM IR for a unary operation expression
     * @param node The unary operation expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateUnaryOp(UnaryOpExpr& node);

    /**
     * @brief Generates LLVM IR for an assignment expression
     * @param node The assignment expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateAssignment(AssignmentExpr& node);

    /**
     * @brief Generates LLVM IR for a function call expression
     * @param node The function call expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateFunctionCall(FunctionCallExpr& node);

    /**
     * @brief Generates LLVM IR for a member access expression
     * @param node The member expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateMember(MemberExpr& node);

#ifdef EMLANG_FEATURE_CASTING
    /**
     * @brief Generates LLVM IR for a cast expression
     * @param node The cast expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateCast(CastExpr& node);
#endif // EMLANG_FEATURE_CASTING

    /**
     * @brief Generates LLVM IR for an index expression
     * @param node The index expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateIndex(IndexExpr& node);

    /**
     * @brief Generates LLVM IR for an array expression
     * @param node The array expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateArray(ArrayExpr& node);

    /**
     * @brief Generates LLVM IR for an object expression
     * @param node The object expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateObject(ObjectExpr& node);

#ifdef EMLANG_FEATURE_POINTERS
    /**
     * @brief Generates LLVM IR for a dereference expression
     * @param node The dereference expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateDereference(DereferenceExpr& node);

    /**
     * @brief Generates LLVM IR for an address-of expression
     * @param node The address-of expression AST node
     * @return Generated LLVM value
     */
    llvm::Value* generateAddressOf(AddressOfExpr& node);
#endif // EMLANG_FEATURE_POINTERS

    // ======================== UTILITY METHODS ========================

    /**
     * @brief Gets the current generated value
     * @return Current LLVM value
     */
    llvm::Value* getCurrentValue() const { return currentValue; }

    /**
     * @brief Gets the current expression type
     * @return Current expression type as string
     */
    const std::string& getCurrentExpressionType() const { return currentExpressionType; }

    /**
     * @brief Sets the current value and type
     * @param value LLVM value to set
     * @param type Expression type to set
     */
    void setCurrentValue(llvm::Value* value, const std::string& type = "");

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

#endif // EM_CODEGEN_CGEXPR_H

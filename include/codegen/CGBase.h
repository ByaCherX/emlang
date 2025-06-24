//===--- CGBase.h - Base Code Generation Visitor -------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Base code generation visitor for EMLang
//
// This file contains the CGBase class that serves as the base visitor
// for all code generation components in EMLang.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CGBASE_H
#define EM_CODEGEN_CGBASE_H
#pragma once

#include <emlang_export.h>
#include "ast.h"
#include "context.h"
#include "value_map.h"
#include "codegen_error.h"
#include <memory>
#include <string>

#include <llvm/IR/Value.h>

namespace emlang {
namespace codegen {

// Forward declarations
class CGExpr;
class CGDecl; 
class CGStmt;

/**
 * @class CGBase
 * @brief Base visitor class for code generation components
 * 
 * CGBase serves as the abstract base class for all code generation visitors.
 * It provides common infrastructure and enforces the visitor pattern while
 * allowing specialized implementations for different AST node categories.
 */
class CGBase : public ASTVisitor {
protected:
    codegen::ContextManager& contextManager;
    codegen::ValueMap& valueMap;
    CodegenErrorReporter& errorReporter;

    llvm::Value* currentValue;

    // References to specialized visitors (optional, can be null)
    CGExpr* exprVisitor;
    CGDecl* declVisitor;
    CGStmt* stmtVisitor;

public:
    /******************** Constructor ********************/
    
    /**
     * @brief Constructs a new CGBase instance
     * @param contextManager Reference to the LLVM context manager
     * @param valueMap Reference to the value mapping system
     * @param errorReporter Reference to the error reporting system
     */
    CGBase(codegen::ContextManager& contextManager,
           codegen::ValueMap& valueMap,
           CodegenErrorReporter& errorReporter);

    /**
     * @brief Constructs a new CGBase instance with specialized visitors
     * @param contextManager Reference to the LLVM context manager
     * @param valueMap Reference to the value mapping system
     * @param errorReporter Reference to the error reporting system
     * @param exprVisitor Expression visitor (can be null)
     * @param declVisitor Declaration visitor (can be null)
     * @param stmtVisitor Statement visitor (can be null)
     */
    CGBase(codegen::ContextManager& contextManager,
           codegen::ValueMap& valueMap,
           CodegenErrorReporter& errorReporter,
           CGExpr* exprVisitor,
           CGDecl* declVisitor,
           CGStmt* stmtVisitor);

    /**
     * @brief Virtual destructor
     */
    virtual ~CGBase() = default;

    /******************** Utility Methods ********************/

    /**
     * @brief Gets the current generated value
     * @return Current LLVM value
     */
    virtual llvm::Value* getCurrentValue() const;

    /**
     * @brief Sets the current value
     * @param value LLVM value to set
     */
    virtual void setCurrentValue(llvm::Value* value);

    /**
     * @brief Sets the current value and type
     * @param value LLVM value to set
     * @param type Expression type to set
     */
    virtual void setCurrentValue(llvm::Value* value, const std::string& type);

    /******************** Error Handling ********************/
    
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

    // ========================================================
    // AST Visitor Method Declarations (Pure Virtual)
    // ========================================================
      // Program visitor - must be implemented by concrete classes
    void visit(Program& node) override;

    // Expression visitors - default empty implementations for non-expression visitors
    void visit(LiteralExpr& node) override {}
    void visit(IdentifierExpr& node) override {}
    void visit(BinaryOpExpr& node) override {}
    void visit(UnaryOpExpr& node) override {}
    void visit(AssignmentExpr& node) override {}
    void visit(FunctionCallExpr& node) override {}
    void visit(MemberExpr& node) override {}
#ifdef EMLANG_FEATURE_CASTING
    void visit(CastExpr& node) override {}
#endif
    void visit(IndexExpr& node) override {}
    void visit(ArrayExpr& node) override {}
    void visit(ObjectExpr& node) override {}
#ifdef EMLANG_FEATURE_POINTERS
    void visit(DereferenceExpr& node) override {}
    void visit(AddressOfExpr& node) override {}
#endif

    // Declaration visitors - default empty implementations for non-declaration visitors
    void visit(VariableDecl& node) override {}
    void visit(FunctionDecl& node) override {}
    void visit(ExternFunctionDecl& node) override {}
    
    // Statement visitors - default empty implementations for non-statement visitors
    void visit(BlockStmt& node) override {}
    void visit(IfStmt& node) override {}
    void visit(WhileStmt& node) override {}
    void visit(ForStmt& node) override {}
    void visit(ReturnStmt& node) override {}
    void visit(ExpressionStmt& node) override {}
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_CGBASE_H

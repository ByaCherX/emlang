//===--- CGExpr.h - Expression Code Generation Visitor -------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Expression code generation visitor for EMLang
//
// This file contains the CGExpr class responsible for generating LLVM IR
// for all expression AST nodes in EMLang using the visitor pattern.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CGEXPR_H
#define EM_CODEGEN_CGEXPR_H

#pragma once

#include <emlang_export.h>
#include "CGBase.h"
#include <memory>
#include <string>

namespace emlang {
namespace codegen {

/**
 * @class CGExpr
 * @brief Expression code generation visitor
 * 
 * CGExpr specializes in generating LLVM IR for expression AST nodes.
 * It implements the visitor pattern and handles value generation,
 * type inference, and expression evaluation.
 */
class CGExpr : public CGBase {
private:
    std::string currentExpressionType;

public:
    /******************** Construction ********************/

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

    /******************** Visitor ********************/

    // Program visitor - delegates to base class or ignores
    void visit(Program& node) override;

    // Expression visitors - full implementations
    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryOpExpr& node) override;
    void visit(UnaryOpExpr& node) override;
    void visit(AssignmentExpr& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(MemberExpr& node) override;
#ifdef EMLANG_FEATURE_CASTING
    void visit(CastExpr& node) override;
#endif
    void visit(IndexExpr& node) override;
    void visit(ArrayExpr& node) override;
    void visit(ObjectExpr& node) override;
#ifdef EMLANG_FEATURE_POINTERS
    void visit(DereferenceExpr& node) override;
    void visit(AddressOfExpr& node) override;
#endif

    // Declaration and Statement visitors - empty implementations (not handled)
    // (inherited empty implementations from CGBase)

    /******************** Utility Methods ********************/

    /**
     * @brief Sets the current value and expression type
     * @param value LLVM value to set
     * @param type Expression type as string
     */
    void setCurrentValue(llvm::Value* value, const std::string& type = "") override;

    /**
     * @brief Gets the current expression type
     * @return Current expression type as string
     */
    const std::string& getCurrentExpressionType() const { return currentExpressionType; }
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_CGEXPR_H

//===--- dumper.h - AST Debug Dumper ----------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// AST debug dumper with colored output
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_DUMPER_H
#define EM_LANG_AST_DUMPER_H

#pragma once

#include "visitor.h"
#include <string>

namespace emlang {

// Forward declarations
class ASTNode;

/// AST dumper for debug output with colored syntax highlighting
class EMLANG_API ASTDumper : public ASTVisitor {
public:
    explicit ASTDumper(bool useColors = true);
    virtual ~ASTDumper() = default;

    /// Dump an AST node to stdout with colored output
    void dump(const ASTNode& node);    
    
    // Visitor interface implementation - matches visitor.h exactly
    void visit(Program& node) override;
    
    // Expression visitors
    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryOpExpr& node) override;
    void visit(UnaryOpExpr& node) override;
    void visit(AssignmentExpr& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(MemberExpr& node) override;
#ifdef EMLANG_FEATURE_CASTING
    void visit(CastExpr& node) override;
#endif // EMLANG_FEATURE_CASTING
    void visit(IndexExpr& node) override;
    void visit(ArrayExpr& node) override;
    void visit(ObjectExpr& node) override;
#ifdef EMLANG_FEATURE_POINTERS
    void visit(DereferenceExpr& node) override;
    void visit(AddressOfExpr& node) override;
#endif

    // Declaration visitors
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(ExternFunctionDecl& node) override;
    
    // Statement visitors
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ForStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(ExpressionStmt& node) override;

private:
    int indent_;
    bool useColors_;

    std::string getIndent() const;
    std::string colorize(const std::string& text, const char* color) const;
    std::string formatNodeHeader(const std::string& type, const ASTNode& node) const;
};

} // namespace emlang

#endif // EM_LANG_AST_DUMPER_H

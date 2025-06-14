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
    
    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryOpExpr& node) override;
    void visit(UnaryOpExpr& node) override;
    void visit(AssignmentExpr& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(DereferenceExpr& node) override;
    void visit(AddressOfExpr& node) override;

    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(ExternFunctionDecl& node) override;
    
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
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

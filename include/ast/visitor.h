//===--- visitor.h - AST Visitor Pattern Interface --------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Visitor pattern interface for AST traversal
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_VISITOR_H
#define EM_LANG_AST_VISITOR_H

#pragma once

#include <emlang_export.h>

namespace emlang {

// Forward declarations for all AST node types
class Program;
class LiteralExpr;
class IdentifierExpr;
class BinaryOpExpr;
class UnaryOpExpr;
class AssignmentExpr;
class FunctionCallExpr;
class DereferenceExpr;
class AddressOfExpr;
class MemberExpr;
class CastExpr;
class IndexExpr;
class ArrayExpr;
class ObjectExpr;
class VariableDecl;
class FunctionDecl;
class ExternFunctionDecl;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class ExpressionStmt;

/**
 * @class ASTVisitor
 * @brief Abstract base class for AST visitors implementing the Visitor pattern
 * 
 * The Visitor pattern allows operations to be performed on AST nodes without
 * modifying the node classes themselves. This enables different compiler phases
 * (semantic analysis, code generation, optimization) to be implemented as
 * separate visitors.
 */
class EMLANG_API ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Visit methods for all AST node types
    virtual void visit(Program& node) = 0;

    // Expression visitors
    virtual void visit(LiteralExpr& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(BinaryOpExpr& node) = 0;
    virtual void visit(UnaryOpExpr& node) = 0;
    virtual void visit(AssignmentExpr& node) = 0;
    virtual void visit(FunctionCallExpr& node) = 0;
    virtual void visit(MemberExpr& node) = 0;
#ifdef EMLANG_FEATURE_CASTING
    virtual void visit(CastExpr& node) = 0;
#endif
    virtual void visit(IndexExpr& node) = 0;
    virtual void visit(ArrayExpr& node) = 0;
    virtual void visit(ObjectExpr& node) = 0;
#ifdef EMLANG_FEATURE_POINTERS
    virtual void visit(DereferenceExpr& node) = 0;
    virtual void visit(AddressOfExpr& node) = 0;
#endif

    // Declaration visitors
    virtual void visit(VariableDecl& node) = 0;
    virtual void visit(FunctionDecl& node) = 0;
    virtual void visit(ExternFunctionDecl& node) = 0;
    
    // Statement visitors
    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(ExpressionStmt& node) = 0;
};

} // namespace emlang

#endif // EM_LANG_AST_VISITOR_H

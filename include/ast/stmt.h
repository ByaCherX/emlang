//===--- stmt.h - Statement AST nodes ---------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Statement AST node declarations
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_STMT_H
#define EM_LANG_AST_STMT_H

#pragma once

#include "ast_base.h"
#include "visitor.h"

namespace emlang {

/**
 * @class BlockStatement
 * @brief Represents a block/compound statement containing multiple statements
 */
class EMLANG_API BlockStmt : public Statement {
public:
    std::vector<StatementPtr> statements;   // Statements in this block
    
    BlockStmt(std::vector<StatementPtr> stmts, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    BlockStmt(const BlockStmt&) = delete;
    BlockStmt& operator=(const BlockStmt&) = delete;
    
    // Enable move constructor and assignment operator
    BlockStmt(BlockStmt&&) = default;
    BlockStmt& operator=(BlockStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class IfStatement
 * @brief Represents conditional statements
 */
class EMLANG_API IfStmt : public Statement {
public:
    ExpressionPtr condition;    // Condition expression
    StatementPtr thenBranch;    // Statement to execute if condition is true
    StatementPtr elseBranch;    // Optional else branch
    
    IfStmt(ExpressionPtr cond, StatementPtr then, StatementPtr else_ = nullptr, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    IfStmt(const IfStmt&) = delete;
    IfStmt& operator=(const IfStmt&) = delete;
    
    // Enable move constructor and assignment operator
    IfStmt(IfStmt&&) = default;
    IfStmt& operator=(IfStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class WhileStatement
 * @brief Represents while loop statements
 */
class EMLANG_API WhileStmt : public Statement {
public:
    ExpressionPtr condition;    // Loop condition
    StatementPtr body;          // Loop body
    
    WhileStmt(ExpressionPtr cond, StatementPtr body, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    WhileStmt(const WhileStmt&) = delete;
    WhileStmt& operator=(const WhileStmt&) = delete;
    
    // Enable move constructor and assignment operator
    WhileStmt(WhileStmt&&) = default;
    WhileStmt& operator=(WhileStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ForStatement
 * @brief Represents for loop statements
 */
class EMLANG_API ForStmt : public Statement {
public:
    StatementPtr initializer;   // Loop initialization (optional)
    ExpressionPtr condition;    // Loop condition (optional)
    ExpressionPtr increment;    // Loop increment (optional)
    StatementPtr body;          // Loop body
    
    ForStmt(StatementPtr init, ExpressionPtr cond, ExpressionPtr incr, StatementPtr body, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    ForStmt(const ForStmt&) = delete;
    ForStmt& operator=(const ForStmt&) = delete;
    
    // Enable move constructor and assignment operator
    ForStmt(ForStmt&&) = default;
    ForStmt& operator=(ForStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ReturnStatement
 * @brief Represents return statements
 */
class EMLANG_API ReturnStmt : public Statement {
public:
    ExpressionPtr value;        // Optional return value
    
    ReturnStmt(ExpressionPtr val = nullptr, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    ReturnStmt(const ReturnStmt&) = delete;
    ReturnStmt& operator=(const ReturnStmt&) = delete;
    
    // Enable move constructor and assignment operator
    ReturnStmt(ReturnStmt&&) = default;
    ReturnStmt& operator=(ReturnStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ExpressionStatement
 * @brief Represents expressions used as statements
 */
class EMLANG_API ExpressionStmt : public Statement {
public:
    ExpressionPtr expression;   // The expression
    
    ExpressionStmt(ExpressionPtr expr, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    ExpressionStmt(const ExpressionStmt&) = delete;
    ExpressionStmt& operator=(const ExpressionStmt&) = delete;
    
    // Enable move constructor and assignment operator
    ExpressionStmt(ExpressionStmt&&) = default;
    ExpressionStmt& operator=(ExpressionStmt&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

} // namespace emlang

#endif // EM_LANG_AST_STMT_H

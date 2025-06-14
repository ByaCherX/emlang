//===--- expr.h - Expression AST nodes --------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Expression AST node declarations
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_EXPR_H
#define EM_LANG_AST_EXPR_H

#pragma once

#include "ast_base.h"
#include "visitor.h"

namespace emlang {

/**
 * @class LiteralExpression
 * @brief Represents literal values (numbers, strings, booleans, etc.)
 */
class EMLANG_API LiteralExpr : public Expression {
public:
    LiteralType literalType;    // Type of the literal
    std::string value;          // String representation of the value
    
    LiteralExpr(LiteralType type, const std::string& value, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class IdentifierExpression
 * @brief Represents identifier references (variables, functions)
 */
class EMLANG_API IdentifierExpr : public Expression {
public:
    std::string name;    // Name of the identifier
    
    IdentifierExpr(const std::string& name, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class BinaryOpExpression
 * @brief Represents binary operations (arithmetic, logical, comparison)
 */
class EMLANG_API BinaryOpExpr : public Expression {
public:
    ExpressionPtr left;         // Left operand
    std::string operator_;      // Operator symbol
    ExpressionPtr right;        // Right operand
    
    BinaryOpExpr(ExpressionPtr left, const std::string& op, ExpressionPtr right, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    BinaryOpExpr(const BinaryOpExpr&) = delete;
    BinaryOpExpr& operator=(const BinaryOpExpr&) = delete;
    
    // Enable move constructor and assignment operator
    BinaryOpExpr(BinaryOpExpr&&) = default;
    BinaryOpExpr& operator=(BinaryOpExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class UnaryOpExpression
 * @brief Represents unary operations (negation, logical NOT, etc.)
 */
class EMLANG_API UnaryOpExpr : public Expression {
public:
    std::string operator_;      // Operator symbol
    ExpressionPtr operand;      // Operand expression
    
    UnaryOpExpr(const std::string& op, ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    UnaryOpExpr(const UnaryOpExpr&) = delete;
    UnaryOpExpr& operator=(const UnaryOpExpr&) = delete;
    
    // Enable move constructor and assignment operator
    UnaryOpExpr(UnaryOpExpr&&) = default;
    UnaryOpExpr& operator=(UnaryOpExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class AssignmentExpression
 * @brief Represents assignment operations
 */
class EMLANG_API AssignmentExpr : public Expression {
public:
    ExpressionPtr target;       // Left-hand side (target)
    ExpressionPtr value;        // Right-hand side (value)
    
    AssignmentExpr(ExpressionPtr target, ExpressionPtr value, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    AssignmentExpr(const AssignmentExpr&) = delete;
    AssignmentExpr& operator=(const AssignmentExpr&) = delete;
    
    // Enable move constructor and assignment operator
    AssignmentExpr(AssignmentExpr&&) = default;
    AssignmentExpr& operator=(AssignmentExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class FunctionCallExpression
 * @brief Represents function call expressions
 */
class EMLANG_API FunctionCallExpr : public Expression {
public:
    std::string functionName;               // Name of the function being called
    std::vector<ExpressionPtr> arguments;   // Function arguments
    
    FunctionCallExpr(const std::string& name, std::vector<ExpressionPtr> args, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    FunctionCallExpr(const FunctionCallExpr&) = delete;
    FunctionCallExpr& operator=(const FunctionCallExpr&) = delete;
    
    // Enable move constructor and assignment operator
    FunctionCallExpr(FunctionCallExpr&&) = default;
    FunctionCallExpr& operator=(FunctionCallExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class DereferenceExpression
 * @brief Represents pointer dereference operations (*ptr)
 */
class EMLANG_API DereferenceExpr : public Expression {
public:
    ExpressionPtr operand;      // Expression being dereferenced
    
    DereferenceExpr(ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    DereferenceExpr(const DereferenceExpr&) = delete;
    DereferenceExpr& operator=(const DereferenceExpr&) = delete;
    
    // Enable move constructor and assignment operator
    DereferenceExpr(DereferenceExpr&&) = default;
    DereferenceExpr& operator=(DereferenceExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class AddressOfExpression
 * @brief Represents address-of operations (&var)
 */
class EMLANG_API AddressOfExpr : public Expression {
public:
    ExpressionPtr operand;      // Expression whose address is taken
    
    AddressOfExpr(ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    AddressOfExpr(const AddressOfExpr&) = delete;
    AddressOfExpr& operator=(const AddressOfExpr&) = delete;
    
    // Enable move constructor and assignment operator
    AddressOfExpr(AddressOfExpr&&) = default;
    AddressOfExpr& operator=(AddressOfExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

} // namespace emlang

#endif // EM_LANG_AST_EXPR_H

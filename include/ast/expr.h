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
class LiteralExpr : public Expression {
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
class IdentifierExpr : public Expression {
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
class BinaryOpExpr : public Expression {
public:
    enum class BinOp {
        ADD, SUB, MUL, DIV, MOD,
        AND, OR, XOR, INV, SHL, SHR,
        EQ, NE, LT, LE, GT, GE,
        LAND, LOR, LNOT,
    };
    ExpressionPtr left;     // Left operand
    BinOp operator_;        // Operator symbol
    ExpressionPtr right;    // Right operand
    
    BinaryOpExpr(ExpressionPtr left, const BinOp& op, ExpressionPtr right, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class UnaryOpExpression
 * @brief Represents unary operations (negation, logical NOT, etc.)
 */
class UnaryOpExpr : public Expression {
public:
    BinaryOpExpr::BinOp operator_;  // Operator symbol
    ExpressionPtr operand;          // Operand expression
    
    UnaryOpExpr(const BinaryOpExpr::BinOp& op, ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class AssignmentExpression
 * @brief Represents assignment operations
 */
class AssignmentExpr : public Expression {
public:
    ExpressionPtr target;       // Left-hand side (target)
    ExpressionPtr value;        // Right-hand side (value)
    
    AssignmentExpr(ExpressionPtr target, ExpressionPtr value, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class FunctionCallExpression
 * @brief Represents function call expressions
 * IMPLEMENT: FunctionCallExpr -> CallExpr
 */
class FunctionCallExpr : public Expression {
public:
    //ExpressionPtr callee;                 // The function being called
    std::string functionName;               // Name of the function being called
    std::vector<ExpressionPtr> arguments;   // Function arguments
    //std::vector<> generic_args;           // Generic arguments (if any)
    
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
 * @class MemberExpression
 * @brief Represents member access operations (obj.member)
 */
class MemberExpr : public Expression {
public:
    ExpressionPtr object;       // Object being accessed
    std::string memberName;     // Name of the member
    bool isMethodCall;          // true if this is a method call
    
    MemberExpr(
        ExpressionPtr object, 
        const std::string& memberName, 
        bool isMethodCall = false, 
        size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class CastExpression
 * @brief Represents type casting operations (expr as Type or cast<Type>(expr))
 */
#ifdef EMLANG_FEATURE_CASTING
class CastExpr : public Expression {
public:
    ExpressionPtr operand;      // Expression being cast
    std::string targetType;     // Target type name
    bool isExplicit;            // true for explicit cast, false for implicit
    
    CastExpr(ExpressionPtr operand, const std::string& targetType, bool isExplicit = true, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    CastExpr(const CastExpr&) = delete;
    CastExpr& operator=(const CastExpr&) = delete;
    
    // Enable move constructor and assignment operator
    CastExpr(CastExpr&&) = default;
    CastExpr& operator=(CastExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};
#endif // EMLANG_FEATURE_CASTING

/**
 * @class IndexExpression
 * @brief Represents array indexing operations (arr[index])
 */
class IndexExpr : public Expression {
public:
    ExpressionPtr array;        // Array expression
    ExpressionPtr index;        // Index expression
    
    IndexExpr(ExpressionPtr array, ExpressionPtr index, size_t line = 0, size_t column = 0);

    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ArrayExpression
 * @brief Represents array literals ([1, 2, 3])
 */
class ArrayExpr : public Expression {
public:
    std::vector<ExpressionPtr> elements;    // Array elements
    
    ArrayExpr(std::vector<ExpressionPtr> elements, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    ArrayExpr(const ArrayExpr&) = delete;
    ArrayExpr& operator=(const ArrayExpr&) = delete;
    
    // Enable move constructor and assignment operator
    ArrayExpr(ArrayExpr&&) = default;
    ArrayExpr& operator=(ArrayExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

/**
 * @struct ObjectField
 * @brief Represents a field in an object literal
 */
struct ObjectField {
    std::string key;            // Field key
    ExpressionPtr value;        // Field value
    
    ObjectField(const std::string& key, ExpressionPtr value) : key(key), value(std::move(value)) {}
    
    // Delete copy constructor and assignment operator
    ObjectField(const ObjectField&) = delete;
    ObjectField& operator=(const ObjectField&) = delete;
    
    // Enable move constructor and assignment operator
    ObjectField(ObjectField&&) = default;
    ObjectField& operator=(ObjectField&&) = default;
};

/**
 * @class ObjectExpression
 * @brief Represents object literals ({key: value})
 */
class ObjectExpr : public Expression {
public:
    std::vector<ObjectField> fields;    // Object fields
    
    ObjectExpr(std::vector<ObjectField> fields, size_t line = 0, size_t column = 0);
    
    // Delete copy constructor and assignment operator
    ObjectExpr(const ObjectExpr&) = delete;
    ObjectExpr& operator=(const ObjectExpr&) = delete;
    
    // Enable move constructor and assignment operator
    ObjectExpr(ObjectExpr&&) = default;
    ObjectExpr& operator=(ObjectExpr&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

#ifdef EMLANG_FEATURE_POINTERS
/**
 * @class DereferenceExpression
 * @brief Represents pointer dereference operations (*ptr)
 */
class DereferenceExpr : public Expression {
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
 * 
 * IMPORTANT: Experimental Feature
 */
class AddressOfExpr : public Expression {
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
#endif // EMLANG_FEATURE_POINTERS

/****************************************
* Helper functions for expression creation
****************************************/
/* BinOp to string method */
std::string binOpToString(BinaryOpExpr::BinOp op);

} // namespace emlang

#endif // EM_LANG_AST_EXPR_H

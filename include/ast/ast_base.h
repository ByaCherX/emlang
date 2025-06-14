//===--- ast_base.h - Base AST classes --------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Base AST classes and visitor pattern interface
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_BASE_H
#define EM_LANG_AST_BASE_H

#pragma once

#include <emlang_export.h>
#include <memory>
#include <vector>
#include <string>

namespace emlang {

// Forward declarations
class ASTVisitor;
class Expression;
class Statement;

// Smart pointer type aliases for cleaner code
using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;

/**
 * @enum ASTNodeType
 * @brief Enumeration of all AST node types for runtime type identification
 */
enum class ASTNodeType {
    // Base types
    PROGRAM,
    
    // Expression types
    LITERAL,
    IDENTIFIER,
    BINARY_OP,
    UNARY_OP,
    ASSIGNMENT,
    FUNCTION_CALL,
    DEREFERENCE,
    ADDRESS_OF,
    
    // Statement types
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    EXTERN_FUNCTION_DECLARATION,
    BLOCK_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION_STMT
};

/**
 * @enum LiteralType
 * @brief Types of literal values supported by the language
 */
enum class LiteralType {
    NUMBER,         // Numeric literals (42, 3.14)
    STRING,         // String literals ("hello")
    CHAR,           // Character literals ('c')
    BOOLEAN,        // Boolean literals (true, false)
    NULL_LITERAL    // Null literal
};

/**
 * @struct Parameter
 * @brief Represents a function parameter
 */
struct Parameter {
    std::string name;    // Parameter name
    std::string type;    // Parameter type
    
    Parameter(const std::string& n, const std::string& t) : name(n), type(t) {}
};

/**
 * @class ASTNode
 * @brief Base class for all AST nodes
 */
class EMLANG_API ASTNode {
public:
    ASTNodeType type;    // Runtime type identification
    size_t line;         // Source code line number (1-based)
    size_t column;       // Source code column number (1-based)
    
    ASTNode(ASTNodeType type, size_t line = 0, size_t column = 0);
    virtual ~ASTNode() = default;
    
    virtual std::string toString() const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
};

/**
 * @class Expression
 * @brief Base class for all expression nodes
 */
class EMLANG_API Expression : public ASTNode {
public:
    Expression(ASTNodeType type, size_t line = 0, size_t column = 0);
    virtual ~Expression() = default;
};

/**
 * @class Statement
 * @brief Base class for all statement nodes
 */
class EMLANG_API Statement : public ASTNode {
public:
    Statement(ASTNodeType type, size_t line = 0, size_t column = 0);
    virtual ~Statement() = default;
};

/**
 * @class Program
 * @brief Root AST node representing a complete program
 */
class EMLANG_API Program : public ASTNode {
public:
    std::vector<StatementPtr> statements;
    
    explicit Program(std::vector<StatementPtr> stmts);
    
    // Delete copy constructor and assignment operator
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    
    // Enable move constructor and assignment operator
    Program(Program&&) = default;
    Program& operator=(Program&&) = default;
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

} // namespace emlang

#endif // EM_LANG_AST_BASE_H

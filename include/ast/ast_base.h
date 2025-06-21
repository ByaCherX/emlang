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
 * @enum NodeType
 * @brief Enumeration of all AST node types for runtime type identification
 */
enum class NodeType {
    // Base types
    PROGRAM,         // Root node representing the entire program
    
    // Expression
    LITERAL_EXPR,    // Literal values (numbers, strings, booleans, etc.)
    IDENTIFIER_EXPR, // Identifier references (variables, functions)
    BINARY_EXPR,     // Binary operations (e.g., addition, subtraction)
    UNARY_EXPR,      // Unary operations (e.g., negation, logical NOT)    
    ASSIGNMENT_EXPR, // Assignment (x = expr)
    FUNCTION_CALL,   // Function call expressions (func(args))
    MEMBER_EXPR,     // Member access (obj.member)
#ifdef EMLANG_FEATURE_CASTING
    CAST_EXPR,       // Type casting (cast<type>(expr) or foo as <type>)
#endif
    INDEX_EXPR,      // Array indexing (arr[index])
    ARRAY_EXPR,      // Array literals ([1, 2, 3])
    OBJECT_EXPR,     // Object literals ({key: value})
#ifdef EMLANG_FEATURE_POINTERS
    DEREFERENCE,     // Dereference operator (*ptr)
    ADDRESS_OF,      // Address-of operator (&var)
#endif
    
    // Statement
    IF_STMT,         // If statements (if (condition) { ... })
    SWITCH_STMT,     // Switch statements (switch (expr) { ... })
    WHILE_STMT,      // While loops (while (condition) { ... })
    FOR_STMT,        // For loops (for (init; condition; increment) { ... })
    RETURN_STMT,     // Return statements (return expr;)
    BLOCK_STMT,      // Block statements ({ ... })
    EXPRESSION_STMT, // Expression statements (expr;)

    // Declarations
    VARIABLE_DECL,   // Variable declarations (let x: int32 = 42;)
    FUNCTION_DECL,   // Function declarations (function name(param1: type1): returnType { ... })
    EXTERN_FN_DECL,  // External function declarations (extern function name(param1: type1): returnType;)
#ifdef EMLANG_FEATURE_IMPORTS
    IMPORT_DECL,      // Import declaration (DO NOT IMPLEMENT YET)
#endif
};

/**
 * @enum LiteralType
 * @brief Types of literal values supported by the language
 */
enum class LiteralType {
    INT,            // Integer literals (42)
    FLOAT,          // Floating-point literals (3.14)
    STR,            // String literals ("hello")
    CHAR,           // Character literals ('c')
    BOOL,           // Boolean literals (true, false)
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
class ASTNode {
public:
    NodeType type;     // Runtime type identification
    size_t line;       // Source code line number (1-based)
    size_t column;     // Source code column number (1-based)
    
    ASTNode(NodeType type, size_t line = 0, size_t column = 0);
    virtual ~ASTNode() = default;
    
    virtual std::string toString() const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
};

/**
 * @class Expression
 * @brief Base class for all expression nodes
 */
class Expression : public ASTNode {
public:
    Expression(NodeType type, size_t line = 0, size_t column = 0);
    virtual ~Expression() = default;
};

/**
 * @class Statement
 * @brief Base class for all statement nodes
 */
class Statement : public ASTNode {
public:
    Statement(NodeType type, size_t line = 0, size_t column = 0);
    virtual ~Statement() = default;
};

/**
 * @class Program
 * @brief Root AST node representing a complete program
 */
class Program : public ASTNode {
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

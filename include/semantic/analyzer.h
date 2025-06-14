//===--- analyzer.h - Sem Analyzer ------------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # Semantic analyzer for the EMLang programming language
//
// This file contains the semantic analysis implementation for EMLang, 
// which is responsible for performing type checking, symbol resolution, 
// and semantic validation of the Abstract Syntax Tree (AST) produced 
// by the parser. The semantic analyzer ensures that the program follows 
// EMLang's type system and semantic rules before code generation.
// 
// Key responsibilities:
// - Symbol table management and scope resolution
// - Type checking and type inference
// - Function signature validation
// - Variable usage and assignment validation
// - Control flow analysis
// - Error detection and reporting
// 
// The semantic analyzer implements the Visitor pattern to traverse the AST and perform
// analysis on each node type. It maintains symbol tables for different scopes and tracks
// type information throughout the analysis process.
// 
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_ANALYZER_H
#define EM_LANG_ANALYZER_H
#pragma once

#include "semantic/semantic_core.h"
#include "ast.h"
#include "ast/visitor.h"
#include <string>
#include <memory>

namespace emlang {

/**
 * @class Analyzer
 * @brief Performs semantic analysis on EMLang AST using the Visitor pattern
 * 
 * The SemanticAnalyzer is responsible for ensuring that an EMLang program is
 * semantically correct before code generation. It performs comprehensive analysis
 * including type checking, symbol resolution, and semantic validation.,
 * 
 * **Analysis Phases:**
 * 1. **Symbol Collection**: First pass to collect all function and global declarations
 * 2. **Type Checking**: Validate all expressions have compatible types
 * 3. **Symbol Resolution**: Ensure all identifiers reference valid symbols
 * 4. **Control Flow Analysis**: Validate return statements and reachability
 * 5. **Semantic Validation**: Check language-specific semantic rules
 * 
 * **Type System Features:**
 * - Static type checking with type inference
 * - Primitive types (int8, int16, int32, int64, isize, uint8, uint16, uint32, uint64, usize, float, double, bool, char, str)
 * - Pointer types with arbitrary levels of indirection
 * - Function types with parameter and return type checking
 * - Type compatibility and implicit conversion rules
 * 
 * **Usage Example:**
 * @code
 * Analyzer analyzer;
 * bool success = analyzer.analyze(program);
 * if (!success) {
 *     // Handle semantic errors
 * }
 * @endcode
 */
class EMLANG_API Analyzer : public ASTVisitor {
private:
    std::vector<std::unique_ptr<Scope>> scopes;     // Stack of active scopes
    Scope* currentScope;                            // Currently active scope
    std::string currentFunctionReturnType;          // Return type of current function being analyzed
    std::string currentExpressionType;              // Type of currently analyzed expression
    bool hasErrors;                                 // Flag indicating if semantic errors were found

    // ======================== SCOPE MANAGEMENT METHODS ========================
    
    /**
     * @brief Creates and enters a new scope
     */
    void enterScope();
    
    /**
     * @brief Exits the current scope and returns to parent
     */
    void exitScope();
    
    /**
     * @brief Determines the type of an expression through type inference
     * @param expr The expression to analyze
     * @return String representing the inferred type
     */
    std::string getExpressionType(Expression& expr);
    
    // ======================== ERROR REPORTING METHODS ========================
    
    /**
     * @brief Reports a semantic error with location information
     * @param message Descriptive error message
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     */
    void error(const std::string& message, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Reports a semantic warning with location information
     * @param message Descriptive warning message
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     */
    void warning(const std::string& message, size_t line = 0, size_t column = 0);
    
public:
    /**
     * @brief Constructs a new Analyzer
     */
    Analyzer();
    
    /**
     * @brief Default destructor
     */
    ~Analyzer() = default;

    // Delete copy operations (unique_ptr is not copyable)
    Analyzer(const Analyzer&) = delete;
    Analyzer& operator=(const Analyzer&) = delete;
    
    // Default move operations
    Analyzer(Analyzer&&) = default;
    Analyzer& operator=(Analyzer&&) = default;
    
    // ======================== MAIN ANALYSIS INTERFACE ========================
    
    /**
     * @brief Performs complete semantic analysis on a program
     * @param program The Program AST node to analyze
     * @return true if analysis succeeded without errors, false if errors found
     */
    bool analyze(Program& program);
    
    /**
     * @brief Checks if any semantic errors were encountered during analysis
     * @return true if errors were found, false if analysis was clean
     */
    bool hasSemanticErrors() const;

    /**
     * @brief Registers built-in functions in the global scope
     */
    void registerBuiltinFunctions();
    
    // ======================== AST VISITOR METHODS ========================
    
    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryOpExpr& node) override;
    void visit(UnaryOpExpr& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(DereferenceExpr& node) override;
    void visit(AddressOfExpr& node) override;
    void visit(AssignmentExpr& node) override;
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(ExternFunctionDecl& node) override;
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(ExpressionStmt& node) override;
    void visit(Program& node) override;
};

} // namespace emlang

#endif // EM_LANG_ANALYZER_H

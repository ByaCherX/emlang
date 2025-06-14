//===--- semantic_core.h - Sem core interface -------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Semantic core interface for the EMLang programming language
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_SEMANTIC_H
#define EM_LANG_SEMANTIC_H

#pragma once

#include <emlang_export.h>
#include "builtins.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace emlang {

/**
 * @struct Symbol
 * @brief Represents a symbol (variable, function, etc.) in the symbol table
 * 
 * A Symbol contains all the information needed to track an identifier throughout
 * the compilation process. This includes its name, type, properties, and source
 * location for error reporting.
 * 
 * The symbol system supports:
 * - Variables (mutable and immutable)
 * - Functions with parameter and return types
 * - Type information for semantic checking
 * - Source location tracking for error reporting
 * - Scope-aware symbol resolution
 */
struct EMLANG_API Symbol {
    std::string name;       // The identifier name as it appears in source code
    std::string type;       // Type string (e.g., "int32", "str", "bool", "int32*")
    bool isConstant;        // true for 'const' declarations, false for 'let'
    bool isFunction;        // true if this symbol represents a function
    size_t line;            // Line number where symbol was declared (1-based)
    size_t column;          // Column number where symbol was declared (1-based)
    
    /**
     * @brief Constructs a new Symbol with the specified properties
     * @param name The identifier name
     * @param type The type string (e.g., "int32", "str", "bool")
     * @param isConst Whether this is a constant (immutable) symbol
     * @param isFunc Whether this symbol represents a function
     * @param line Source line number for error reporting
     * @param column Source column number for error reporting
     */
    Symbol(
        const std::string& name, 
        const std::string& type, 
        bool isConst = false, 
        bool isFunc = false, 
        size_t line = 0, 
        size_t column = 0
    )
        : name(name), type(type), isConstant(isConst), isFunction(isFunc), line(line), column(column) {}
};

/**
 * @class Scope
 * @brief Represents a lexical scope containing symbols and supporting nested scoping
 * 
 * The Scope class implements a hierarchical symbol table that supports EMLang's
 * lexical scoping rules. Each scope can contain symbols and has an optional parent
 * scope for nested scope resolution.
 * 
 * **Scoping Rules:**
 * - Inner scopes can access symbols from outer scopes
 * - Inner scopes can shadow symbols from outer scopes
 * - Symbol lookup proceeds from innermost to outermost scope
 * - Each scope maintains its own symbol table
 * 
 * **Scope Types:**
 * - Global scope (top-level declarations)
 * - Function scope (function parameters and local variables)
 * - Block scope (variables declared within blocks)
 * 
 * **Usage Example:**
 * @code
 * Scope globalScope;                           // Global scope
 * Scope functionScope(&globalScope);           // Function scope with global parent
 * functionScope.define("x", "int32");          // Define local variable
 * Symbol* symbol = functionScope.lookup("x");  // Lookup in current + parent scopes
 * @endcode
 */
class EMLANG_API Scope {
private:
    std::map<std::string, std::unique_ptr<Symbol>> symbols;  // Symbol table for this scope
    Scope* parent;                                           // Parent scope (nullptr for global)
    
public:
    /**
     * @brief Constructs a new Scope with an optional parent scope
     * @param parent Pointer to parent scope (nullptr for global scope)
     * 
     * Creates a new scope that can contain symbols. If a parent is provided,
     * symbol lookups will search the parent scope if not found locally.
     */
    explicit Scope(Scope* parent = nullptr);
    
    // ======================== SYMBOL OPERATIONS ========================
    
    /**
     * @brief Defines a new symbol in this scope
     * @param name The symbol name (identifier)
     * @param type The symbol type (e.g., "int32", "str", "bool")
     * @param isConst Whether the symbol is constant (immutable)
     * @param isFunc Whether the symbol represents a function
     * @param line Source line number for error reporting
     * @param column Source column number for error reporting
     * @return true if symbol was successfully defined, false if already exists
     * 
     * This method adds a new symbol to the current scope. It will fail if a symbol
     * with the same name already exists in this scope (but allows shadowing of
     * symbols from parent scopes).
     * 
     * The method is used during:
     * - Variable declarations (let, const)
     * - Function declarations
     * - Parameter declarations
     * - Block-scoped symbol introduction
     */
    bool define(
        const std::string& name,
        const std::string& type,
        bool isConst = false,
        bool isFunc = false,
        size_t line = 0,
        size_t column = 0
    );
    
    /**
     * @brief Looks up a symbol by name in this scope and parent scopes
     * @param name The symbol name to search for
     * @return Pointer to Symbol if found, nullptr if not found
     * 
     * This method performs symbol resolution by searching:
     * 1. Current scope first
     * 2. Parent scope if not found locally
     * 3. Continues up the scope chain until found or global scope reached
     * 
     * This implements EMLang's lexical scoping rules where inner scopes
     * can access symbols from outer scopes.
     */
    Symbol* lookup(const std::string& name);
    
    /**
     * @brief Checks if a symbol exists in this scope or any parent scope
     * @param name The symbol name to check
     * @return true if symbol exists anywhere in scope chain, false otherwise
     * 
     * This is a convenience method that returns boolean result for symbol
     * existence checking without returning the actual symbol.
     */
    bool exists(const std::string& name);
    
    /**
     * @brief Checks if a symbol exists in this scope only (not parent scopes)
     * @param name The symbol name to check
     * @return true if symbol exists in current scope, false otherwise
     * 
     * This method checks only the current scope, ignoring parent scopes.
     * It's used to detect redefinition errors and to implement shadowing rules.
     */
    bool existsInCurrentScope(const std::string& name);
    
    // ======================== SCOPE OPERATIONS ========================
    
    /**
     * @brief Returns the parent scope of this scope
     * @return Pointer to parent scope, or nullptr if this is the global scope
     * 
     * This method provides access to the parent scope for scope chain
     * navigation and cleanup operations.
     */
    Scope* getParent() const;

    // Default destructor
    ~Scope() = default;

    // Delete copy operations (unique_ptr is not copyable)
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    
    // Default move operations
    Scope(Scope&&) = default;
    Scope& operator=(Scope&&) = default;
};

} // namespace emlang

#endif // EM_LANG_SEMANTIC_H
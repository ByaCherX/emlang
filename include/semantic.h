//===--- semantic.h - Sem interface ------------------------- ---*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # Semantic analyzer for the EMLang programming language
//
// This file contains the semantic analysis implementation for EMLang, which is responsible for
// performing type checking, symbol resolution, and semantic validation of the Abstract Syntax Tree (AST)
// produced by the parser. The semantic analyzer ensures that the program follows EMLang's type system
// and semantic rules before code generation.
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

#ifndef EM_LANG_SEMANTIC_H
#define EM_LANG_SEMANTIC_H

#pragma once

#include "ast.h"
#include "builtins.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

// DLL Export/Import Macros for Windows
#ifdef _WIN32
    #ifdef EMLANG_EXPORTS
        #define EMLANG_API __declspec(dllexport)
    #elif defined(EMLANG_DLL)
        #define EMLANG_API __declspec(dllimport)
    #else
        #define EMLANG_API
    #endif
#else
    #define EMLANG_API
#endif


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

/**
 * @class SemanticAnalyzer
 * @brief Performs semantic analysis on EMLang AST using the Visitor pattern
 * 
 * The SemanticAnalyzer is responsible for ensuring that an EMLang program is
 * semantically correct before code generation. It performs comprehensive analysis
 * including type checking, symbol resolution, and semantic validation.
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
 * **Error Handling:**
 * - Precise error location reporting
 * - Multiple error detection in single analysis pass
 * - Detailed error messages with context
 * - Warning generation for suspicious but legal code
 * 
 * **Usage Example:**
 * @code
 * SemanticAnalyzer analyzer;
 * bool success = analyzer.analyze(program);
 * if (!success) {
 *     // Handle semantic errors
 * }
 * @endcode
 */
class EMLANG_API SemanticAnalyzer : public ASTVisitor {
private:
    std::vector<std::unique_ptr<Scope>> scopes;     // Stack of active scopes
    Scope* currentScope;                            // Currently active scope
    std::string currentFunctionReturnType;          // Return type of current function being analyzed
    bool hasErrors;                                 // Flag indicating if semantic errors were found

    // ======================== TYPE CHECKING METHODS ========================
    
    /**
     * @brief Determines the type of an expression through type inference
     * @param expr The expression to analyze
     * @return String representing the inferred type
     * 
     * This method performs type inference on expressions by:
     * - Analyzing literal values (numbers, strings, booleans)
     * - Looking up identifier types in symbol tables
     * - Computing result types for binary operations
     * - Validating function call return types
     * - Handling pointer arithmetic and dereferencing
     * 
     * The method implements EMLang's type inference rules and ensures
     * that all expressions have well-defined types.
     */
    std::string getExpressionType(Expression& expr);
    
    /**
     * @brief Checks if two types are compatible for assignment or comparison
     * @param expected The expected/target type
     * @param actual The actual/source type
     * @return true if types are compatible, false otherwise
     * 
     * This method implements EMLang's type compatibility rules:
     * - Exact type matches are always compatible
     * - Numeric types have specific conversion rules
     * - Pointer types must match exactly (no implicit conversions)
     * - String and character types have specific compatibility rules
     * 
     * Used for validating assignments, function arguments, and comparisons.
     */
    bool isCompatibleType(const std::string& expected, const std::string& actual);
    
    /**
     * @brief Checks if a type is a numeric type (integer or floating-point)
     * @param type The type string to check
     * @return true if type is numeric, false otherwise
     * 
     * Numeric types in EMLang include:
     * - Signed integers: int8, int16, int32, int64, isize
     * - Unsigned integers: uint8, uint16, uint32, uint64, usize
     * - Floating-point: float, double
     * 
     * Used for validating arithmetic operations and numeric comparisons.
     */
    bool isNumericType(const std::string& type);
    
    /**
     * @brief Checks if a type is the boolean type
     * @param type The type string to check
     * @return true if type is bool, false otherwise
     * 
     * Used for validating logical operations, conditions, and boolean expressions.
     */
    bool isBooleanType(const std::string& type);
    
    /**
     * @brief Checks if a type is a pointer type
     * @param type The type string to check
     * @return true if type is a pointer (ends with '*'), false otherwise
     * 
     * Pointer types in EMLang are denoted by '*' suffix (e.g., "int32*", "char*").
     * This method is used for validating pointer operations like dereferencing
     * and address-of operations.
     */
    bool isPointerType(const std::string& type);
    
    /**
     * @brief Extracts the base type from a pointer type
     * @param pointerType The pointer type (e.g., "int32*")
     * @return The base type (e.g., "int32")
     * 
     * This method removes one level of pointer indirection from a type.
     * For example: "int32*" → "int32", "char**" → "char*"
     * 
     * Used for dereferencing operations and pointer arithmetic.
     */
    std::string getPointerBaseType(const std::string& pointerType);
    
    /**
     * @brief Creates a pointer type from a base type
     * @param baseType The base type (e.g., "int32")
     * @return The pointer type (e.g., "int32*")
     * 
     * This method adds one level of pointer indirection to a type.
     * Used for address-of operations and pointer type construction.
     */
    std::string makePointerType(const std::string& baseType);
    
    /**
     * @brief Checks if a type is the string type
     * @param type The type string to check
     * @return true if type is str, false otherwise
     * 
     * Used for validating string operations and string literal assignments.
     */
    bool isStringType(const std::string& type);
    
    // ======================== ADVANCED TYPE CHECKING HELPERS ========================
    
    /**
     * @brief Checks if a type is a signed integer type
     * @param type The type string to check
     * @return true if type is signed integer, false otherwise
     * 
     * Signed integer types: int8, int16, int32, int64, isize
     * Used for type promotion and arithmetic operation validation.
     */
    bool isSignedInteger(const std::string& type);
    
    /**
     * @brief Checks if a type is an unsigned integer type
     * @param type The type string to check
     * @return true if type is unsigned integer, false otherwise
     * 
     * Unsigned integer types: uint8, uint16, uint32, uint64, usize
     * Used for type promotion and arithmetic operation validation.
     */
    bool isUnsignedInteger(const std::string& type);
    
    /**
     * @brief Checks if a type is a floating-point type
     * @param type The type string to check
     * @return true if type is floating-point, false otherwise
     * 
     * Floating-point types: float, double
     * Used for arithmetic operation validation and type promotion.
     */
    bool isFloatingPoint(const std::string& type);
    
    /**
     * @brief Checks if a type is the character type
     * @param type The type string to check
     * @return true if type is char, false otherwise
     * 
     * Used for character literal validation and character operations.
     */
    bool isCharType(const std::string& type);
    
    /**
     * @brief Checks if a type is the unit type (void equivalent)
     * @param type The type string to check
     * @return true if type represents no value, false otherwise
     * 
     * Used for validating function return types and void expressions.
     */
    bool isUnitType(const std::string& type);
    
    /**
     * @brief Checks if a type is a built-in primitive type
     * @param type The type string to check
     * @return true if type is primitive, false otherwise
     * 
     * Primitive types include all numeric types, bool, char, and str.
     * Used for distinguishing between built-in and user-defined types.
     */
    bool isPrimitiveType(const std::string& type);
    
    /**
     * @brief Checks if a type can be implicitly converted to another type
     * @param from The source type
     * @param to The target type
     * @return true if implicit conversion is allowed, false otherwise
     * 
     * EMLang's implicit conversion rules:
     * - Numeric widening conversions (int8 → int16 → int32 → int64)
     * - Integer to floating-point conversions
     * - Specific compatibility rules for each type combination
     * 
     * Used for assignment validation and function argument checking.
     */
    bool canImplicitlyConvert(const std::string& from, const std::string& to);
    
    /**
     * @brief Determines the common type for binary operations
     * @param type1 First operand type
     * @param type2 Second operand type
     * @return The result type for the operation
     * 
     * This method implements type promotion rules for binary operations:
     * - If both types are the same, return that type
     * - For numeric types, promote to the wider type
     * - For mixed signed/unsigned, apply specific promotion rules
     * - For incompatible types, return empty string (error)
     * 
     * Used for determining result types of arithmetic and comparison operations.
     */
    std::string getCommonType(const std::string& type1, const std::string& type2);
    
    // ======================== SCOPE MANAGEMENT METHODS ========================
    
    /**
     * @brief Creates and enters a new scope
     * 
     * This method:
     * - Creates a new Scope with current scope as parent
     * - Pushes the new scope onto the scope stack
     * - Updates currentScope to point to the new scope
     * 
     * Called when entering:
     * - Function bodies
     * - Block statements
     * - Control flow statement bodies
     */
    void enterScope();
    
    /**
     * @brief Exits the current scope and returns to parent
     * 
     * This method:
     * - Pops the current scope from the scope stack
     * - Updates currentScope to point to the parent scope
     * - Cleans up scope-specific state
     * 
     * Called when exiting:
     * - Function bodies
     * - Block statements
     * - Control flow statement bodies
     */
    void exitScope();
    
    // ======================== ERROR REPORTING METHODS ========================
    
    /**
     * @brief Reports a semantic error with location information
     * @param message Descriptive error message
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     * 
     * This method:
     * - Records that an error occurred (sets hasErrors flag)
     * - Formats and outputs the error message with location
     * - Provides context for debugging and IDE integration
     * - Continues analysis to find additional errors
     * 
     * Used for all semantic errors including:
     * - Type mismatches
     * - Undefined symbols
     * - Redefinition errors
     * - Invalid operations
     */
    void error(const std::string& message, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Reports a semantic warning with location information
     * @param message Descriptive warning message
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     * 
     * This method reports suspicious but legal code patterns:
     * - Unused variables
     * - Unreachable code
     * - Potentially unintended operations
     * - Style violations
     * 
     * Warnings don't prevent compilation but help improve code quality.
     */
    void warning(const std::string& message, size_t line = 0, size_t column = 0);
    
public:
    /**
     * @brief Constructs a new SemanticAnalyzer
     * 
     * Initializes the analyzer with:
     * - Empty scope stack
     * - Global scope as initial scope
     * - No current function context
     * - Error state reset
     */
    SemanticAnalyzer();
    
    /**
     * @brief Default destructor
     */
    ~SemanticAnalyzer() = default;

    // Delete copy operations (unique_ptr is not copyable)
    SemanticAnalyzer(const SemanticAnalyzer&) = delete;
    SemanticAnalyzer& operator=(const SemanticAnalyzer&) = delete;
    
    // Default move operations
    SemanticAnalyzer(SemanticAnalyzer&&) = default;
    SemanticAnalyzer& operator=(SemanticAnalyzer&&) = default;
    
    // ======================== MAIN ANALYSIS INTERFACE ========================
    
    /**
     * @brief Performs complete semantic analysis on a program
     * @param program The Program AST node to analyze
     * @return true if analysis succeeded without errors, false if errors found
     * 
     * This is the main entry point for semantic analysis. It:
     * - Sets up the global scope
     * - Visits all top-level declarations
     * - Performs type checking on all expressions
     * - Validates all semantic rules
     * - Reports errors and warnings
     * 
     * The method returns false if any semantic errors were found,
     * indicating that the program should not proceed to code generation.
     */
    bool analyze(Program& program);
    
    /**
     * @brief Checks if any semantic errors were encountered during analysis
     * @return true if errors were found, false if analysis was clean
     * 
     * This method can be called after analyze() to check the error state
     * without needing to store the return value of analyze().
     */
    bool hasSemanticErrors() const;
    
    // ======================== AST VISITOR METHODS ========================
    // These methods implement the Visitor pattern for AST traversal
    
    /**
     * @brief Visits a literal expression (numbers, strings, booleans, characters)
     * @param node The LiteralExpression AST node
     * 
     * This method:
     * - Determines the type of the literal based on its value and format
     * - Validates that the literal is well-formed
     * - Sets the expression type for parent expressions
     * 
     * Handles all literal types: numeric, string, boolean, and character literals.
     */
    void visit(LiteralExpression& node) override;
    
    /**
     * @brief Visits an identifier expression (variable/function references)
     * @param node The IdentifierExpression AST node
     * 
     * This method:
     * - Looks up the identifier in the symbol table
     * - Reports undefined symbol errors if not found
     * - Sets the expression type based on symbol type
     * - Validates that the symbol is used appropriately
     */
    void visit(IdentifierExpression& node) override;
    
    /**
     * @brief Visits a binary operation expression (+, -, *, /, ==, !=, etc.)
     * @param node The BinaryOpExpression AST node
     * 
     * This method:
     * - Recursively analyzes left and right operands
     * - Validates that the operation is valid for the operand types
     * - Performs type checking and promotion as needed
     * - Determines the result type of the operation
     * - Reports type mismatch errors
     */
    void visit(BinaryOpExpression& node) override;
    
    /**
     * @brief Visits a unary operation expression (-, !, &)
     * @param node The UnaryOpExpression AST node
     * 
     * This method:
     * - Recursively analyzes the operand
     * - Validates that the operation is valid for the operand type
     * - Determines the result type (e.g., & produces pointer type)
     * - Reports invalid operation errors
     */
    void visit(UnaryOpExpression& node) override;
    
    /**
     * @brief Visits a function call expression
     * @param node The FunctionCallExpression AST node
     * 
     * This method:
     * - Looks up the function in the symbol table
     * - Validates that the symbol is actually a function
     * - Checks argument count against parameter count
     * - Performs type checking on each argument
     * - Sets the expression type to the function's return type
     */
    void visit(FunctionCallExpression& node) override;
    
    /**
     * @brief Visits a pointer dereference expression (*ptr)
     * @param node The DereferenceExpression AST node
     * 
     * This method:
     * - Recursively analyzes the pointer expression
     * - Validates that the expression has pointer type
     * - Determines the result type by removing one level of indirection
     * - Reports invalid dereference errors
     */
    void visit(DereferenceExpression& node) override;
    
    /**
     * @brief Visits an address-of expression (&var)
     * @param node The AddressOfExpression AST node
     * 
     * This method:
     * - Recursively analyzes the target expression
     * - Validates that the target is addressable (lvalue)
     * - Determines the result type by adding pointer indirection
     * - Reports invalid address-of errors
     */
    void visit(AddressOfExpression& node) override;
    
    /**
     * @brief Visits a variable declaration (let/const statements)
     * @param node The VariableDeclaration AST node
     * 
     * This method:
     * - Validates that the variable name is not already defined in current scope
     * - Analyzes the initializer expression if present
     * - Performs type checking between declared type and initializer
     * - Adds the variable symbol to the current scope
     * - Handles type inference for variables without explicit types
     */
    void visit(VariableDeclaration& node) override;
    
    /**
     * @brief Visits a function declaration
     * @param node The FunctionDeclaration AST node
     * 
     * This method:
     * - Validates that the function name is not already defined
     * - Creates a new scope for the function body
     * - Adds parameter symbols to the function scope
     * - Sets current function context for return type checking
     * - Analyzes the function body
     * - Validates return statements against declared return type
     */
    void visit(FunctionDeclaration& node) override;
    
    /**
     * @brief Visits an external function declaration
     * @param node The ExternFunctionDeclaration AST node
     * 
     * This method:
     * - Validates that the external function name is not already defined
     * - Validates parameter types for correctness
     * - Validates return type for correctness
     * - Adds the external function symbol to the current scope
     * - Ensures external functions don't conflict with regular functions
     * - No function body analysis (external functions have no body)
     */
    void visit(ExternFunctionDeclaration& node) override;
    
    /**
     * @brief Visits a block statement (compound statement)
     * @param node The BlockStatement AST node
     * 
     * This method:
     * - Creates a new scope for the block
     * - Recursively analyzes all statements in the block
     * - Exits the block scope when done
     * - Handles nested scoping correctly
     */
    void visit(BlockStatement& node) override;
    
    /**
     * @brief Visits an if statement (conditional statement)
     * @param node The IfStatement AST node
     * 
     * This method:
     * - Analyzes the condition expression
     * - Validates that the condition has boolean type
     * - Recursively analyzes the then-statement
     * - Recursively analyzes the else-statement if present
     * - Reports type errors for non-boolean conditions
     */
    void visit(IfStatement& node) override;
    
    /**
     * @brief Visits a while loop statement
     * @param node The WhileStatement AST node
     * 
     * This method:
     * - Analyzes the condition expression
     * - Validates that the condition has boolean type
     * - Recursively analyzes the loop body
     * - Reports type errors for non-boolean conditions
     */
    void visit(WhileStatement& node) override;
    
    /**
     * @brief Visits a return statement
     * @param node The ReturnStatement AST node
     * 
     * This method:
     * - Validates that we're currently inside a function
     * - Analyzes the return value expression if present
     * - Validates that the return type matches the function's declared return type
     * - Handles void returns (no return value)
     * - Reports return type mismatch errors
     */
    void visit(ReturnStatement& node) override;
    
    /**
     * @brief Visits an expression statement
     * @param node The ExpressionStatement AST node
     * 
     * This method:
     * - Recursively analyzes the contained expression
     * - Validates that the expression is well-formed
     * - Does not require any specific return type
     */
    void visit(ExpressionStatement& node) override;
    
    /**
     * @brief Visits the root program node
     * @param node The Program AST node
     * 
     * This method:
     * - Sets up the global scope
     * - Recursively analyzes all top-level statements
     * - Performs global semantic validation
     * - Reports any program-level semantic errors
     */
    void visit(Program& node) override;
    
    // ======================== BUILT-IN FUNCTION SUPPORT ========================
    
    /**
     * @brief Registers built-in functions in the global scope
     * 
     * This method:
     * - Gets built-in functions from builtins.cpp
     * - Registers each built-in function as a symbol in global scope
     * - Sets appropriate function types and signatures
     * - Enables automatic built-in function resolution
     */
    void registerBuiltinFunctions();
    
private:
    // Current expression type (used during type checking)
    std::string currentExpressionType;  // Holds the type of the currently analyzed expression
};

} // namespace emlang

#endif // EM_LANG_SEMANTIC_H
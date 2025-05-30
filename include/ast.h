//===--- ast.h - AST interface ----------------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # Abstract Syntax Tree (AST) definitions for the EMLang programming language
//
// This file contains the complete AST node hierarchy for EMLang. The AST represents
// the syntactic structure of a program after parsing, providing a tree-based
// representation that can be easily traversed and analyzed by subsequent compiler phases.
// 
// **Design Principles:**
// - Hierarchical node structure with clear inheritance relationships
// - Visitor pattern support for extensible traversal and analysis
// - Position tracking for accurate error reporting and debugging
// - Type-safe node classification using enums
// - Memory-efficient design using smart pointers
// 
// **AST Structure:**
// ```
// ASTNode (base)
// ├── Expression
// │   ├── LiteralExpression (numbers, strings, booleans, etc.)
// │   ├── IdentifierExpression (variable/function references)
// │   ├── BinaryOpExpression (arithmetic, logical, comparison)
// │   ├── UnaryOpExpression (negation, logical NOT, address-of)
// │   ├── FunctionCallExpression (function invocations)
// │   ├── DereferenceExpression (pointer dereferencing)
// │   └── AddressOfExpression (address-of operations)
// ├── Statement
// │   ├── VariableDeclaration (let/const statements)
// │   ├── FunctionDeclaration (function definitions)
// │   ├── BlockStatement (compound statements)
// │   ├── IfStatement (conditional statements)
// │   ├── WhileStatement (loop statements)
// │   ├── ReturnStatement (function returns)
// │   └── ExpressionStatement (expression used as statement)
// └── Program (root node containing all top-level statements)
// ```
// 
// **Visitor Pattern:**
// The AST implements the Visitor pattern to allow extensible traversal without
// modifying the node classes. This enables different compiler phases (semantic
// analysis, code generation, optimization) to operate on the same AST structure.
//
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_H
#define EM_LANG_AST_H

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace emlang {

// Forward declarations for all AST node types
// This allows for circular references and reduces compilation dependencies
class ASTNode;           // Base class for all AST nodes
class Expression;        // Base class for all expressions
class Statement;         // Base class for all statements
class ASTVisitor;        // Visitor interface for AST traversal

// Expression node types
class LiteralExpression;        // Literal values (42, "hello", true, 'c')
class IdentifierExpression;     // Variable and function names
class BinaryOpExpression;       // Binary operations (+, -, *, /, ==, etc.)
class UnaryOpExpression;        // Unary operations (-, !, &)
class FunctionCallExpression;   // Function call expressions
class DereferenceExpression;    // Pointer dereference (*ptr)
class AddressOfExpression;      // Address-of operations (&var)

// Statement node types
class VariableDeclaration;      // Variable declarations (let/const)
class FunctionDeclaration;      // Function declarations
class BlockStatement;           // Block/compound statements
class IfStatement;              // Conditional statements
class WhileStatement;           // While loop statements
class ReturnStatement;          // Return statements
class ExpressionStatement;      // Expression used as statement

// Program node type
class Program;                  // Root node representing entire program

// Type aliases for smart pointers to improve code readability and type safety
using ASTNodePtr = std::unique_ptr<ASTNode>;        // Generic AST node pointer
using ExpressionPtr = std::unique_ptr<Expression>;  // Expression node pointer
using StatementPtr = std::unique_ptr<Statement>;    // Statement node pointer

/**
 * @enum ASTNodeType
 * @brief Enumeration of all possible AST node types for runtime type identification
 * 
 * This enum provides a way to identify the specific type of an AST node at runtime
 * without using expensive dynamic_cast operations. Each node type has a unique
 * identifier that can be used for:
 * - Fast type checking during traversal
 * - Switch-based dispatch in algorithms
 * - Debugging and logging
 * - Serialization and deserialization
 * 
 * The types are organized into logical groups:
 * - Expressions: Nodes that evaluate to values
 * - Statements: Nodes that represent actions or declarations
 * - Program: The root node representing the entire program
 */
enum class ASTNodeType {
    // ======================== EXPRESSION TYPES ========================
    // These nodes represent computations that produce values
    
    LITERAL,           // Literal values: numbers, strings, booleans, characters, null
    IDENTIFIER,        // Variable and function name references
    BINARY_OP,         // Binary operations: +, -, *, /, ==, !=, <, >, &&, ||, etc.
    UNARY_OP,          // Unary operations: -, !, & (address-of)
    FUNCTION_CALL,     // Function call expressions with arguments
    ARRAY_ACCESS,      // Array element access (future extension)
    DEREFERENCE,       // Pointer dereference operation (*ptr)
    ADDRESS_OF,        // Address-of operation (&var)
    
    // ======================== STATEMENT TYPES ========================
    // These nodes represent actions, declarations, and control flow
    
    EXPRESSION_STMT,       // Expression used as a statement
    VARIABLE_DECLARATION,  // Variable declarations (let/const)
    FUNCTION_DECLARATION,  // Function definitions with parameters and body
    IF_STATEMENT,          // Conditional statements (if/else)
    WHILE_STATEMENT,       // While loop statements
    FOR_STATEMENT,         // For loop statements (future extension)
    RETURN_STATEMENT,      // Return statements with optional value
    BLOCK_STATEMENT,       // Block/compound statements ({ ... })
    
    // ======================== PROGRAM TYPE ========================
    // Root node type
    
    PROGRAM = 0xFF               // Root node containing all top-level statements
};

/**
 * @class ASTNode
 * @brief Base class for all nodes in the Abstract Syntax Tree
 * 
 * ASTNode provides the common interface and functionality shared by all nodes
 * in the AST. It establishes the foundation for the entire node hierarchy and
 * supports the Visitor pattern for extensible traversal.
 * 
 * **Key Features:**
 * - Type identification through ASTNodeType enum
 * - Source position tracking for error reporting
 * - Virtual destructor for proper polymorphic cleanup
 * - Pure virtual methods enforcing interface contracts
 * - Support for the Visitor pattern
 * 
 * **Design Rationale:**
 * The abstract base class design allows for uniform treatment of all AST nodes
 * while preserving type safety and enabling polymorphic operations. The position
 * tracking enables precise error reporting and debugging capabilities.
 */
class ASTNode {
public:
    ASTNodeType type;    // Runtime type identifier for this node
    size_t line;         // Source line number where this construct appears (1-based)
    size_t column;       // Source column number where this construct starts (1-based)
    
    /**
     * @brief Constructs a new ASTNode with type and position information
     * @param type The specific node type from ASTNodeType enum
     * @param line Source line number (1-based indexing, 0 for unknown)
     * @param column Source column number (1-based indexing, 0 for unknown)
     * 
     * The position information is crucial for:
     * - Error reporting with precise source locations
     * - IDE integration for syntax highlighting and navigation
     * - Debugging and development tools
     * - Source-to-source transformations
     */
    ASTNode(ASTNodeType type, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes
     * 
     * This virtual destructor is essential for proper polymorphic destruction
     * when deleting AST nodes through base class pointers. It ensures that
     * derived class destructors are called correctly.
     */
    virtual ~ASTNode() = default;
    
    /**
     * @brief Converts the AST node to a human-readable string representation
     * @return String representation of the node and its structure
     * 
     * This pure virtual method must be implemented by all derived classes to
     * provide a textual representation of the node. Used for:
     * - Debugging and logging
     * - AST visualization tools
     * - Testing and verification
     * - Pretty-printing source code
     * 
     * The string representation should include the node type and relevant
     * information about its contents in a hierarchical format.
     */
    virtual std::string toString() const = 0;
    
    /**
     * @brief Accepts a visitor for AST traversal using the Visitor pattern
     * @param visitor The visitor object that will process this node
     * 
     * This method implements the Visitor pattern by calling the appropriate
     * visit method on the visitor object. Each derived class implements this
     * method to call visitor.visit(*this) with the correct static type.
     * 
     * The Visitor pattern enables:
     * - Extensible operations without modifying AST classes
     * - Type-safe dispatch to visitor methods
     * - Separation of concerns between data structure and algorithms
     * - Multiple analysis passes over the same AST
     */
    virtual void accept(class ASTVisitor& visitor) = 0;
};

/**
 * @class Expression
 * @brief Base class for all expression nodes in the AST
 * 
 * Expression represents any construct that evaluates to a value. This includes
 * literals, variables, function calls, arithmetic operations, and other
 * computations. Expressions are the building blocks of statements and can be
 * composed to form complex computations.
 * 
 * **Expression Categories:**
 * - **Literals**: Direct values (42, "hello", true)
 * - **Identifiers**: References to variables and functions
 * - **Operations**: Arithmetic, logical, and comparison operations
 * - **Calls**: Function and method invocations
 * - **Pointer operations**: Dereferencing and address-of
 * 
 * **Type System Integration:**
 * During semantic analysis, each expression is assigned a type that represents
 * the kind of value it produces. This type information is used for:
 * - Type checking and validation
 * - Code generation decisions
 * - Optimization opportunities
 * - Runtime behavior determination
 */
class Expression : public ASTNode {
public:
    /**
     * @brief Constructs a new Expression node
     * @param type The specific expression node type
     * @param line Source line number
     * @param column Source column number
     * 
     * Initializes the base ASTNode with the provided type and position information.
     * All expression types must call this constructor to properly initialize
     * the base class.
     */
    Expression(ASTNodeType type, size_t line = 0, size_t column = 0);
};

/**
 * @class Statement
 * @brief Base class for all statement nodes in the AST
 * 
 * Statement represents any construct that performs an action or makes a
 * declaration. Unlike expressions, statements do not evaluate to values
 * but instead cause side effects, control program flow, or introduce
 * new bindings into the environment.
 * 
 * **Statement Categories:**
 * - **Declarations**: Variable and function declarations
 * - **Control Flow**: If statements, loops, returns
 * - **Compound**: Block statements containing multiple statements
 * - **Expression Statements**: Expressions used for their side effects
 * 
 * **Execution Semantics:**
 * Statements are executed sequentially within their containing scope.
 * Some statements (like control flow constructs) may alter the normal
 * sequential execution order by jumping to different parts of the program
 * or repeating certain sections.
 */
class Statement : public ASTNode {
public:
    /**
     * @brief Constructs a new Statement node
     * @param type The specific statement node type
     * @param line Source line number
     * @param column Source column number
     * 
     * Initializes the base ASTNode with the provided type and position information.
     * All statement types must call this constructor to properly initialize
     * the base class.
     */
    Statement(ASTNodeType type, size_t line = 0, size_t column = 0);
};

/**
 * @class LiteralExpression
 * @brief Represents literal values in the source code
 * 
 * LiteralExpression handles all forms of literal values that can appear directly
 * in the source code. These are compile-time constants that represent specific
 * values of various types. The literal system supports EMLang's complete set
 * of literal syntaxes and provides type information for semantic analysis.
 * 
 * **Supported Literal Types:**
 * - **Numbers**: Integer and floating-point literals (42, 3.14, 0xFF, 1e10)
 * - **Strings**: String literals with escape sequences ("hello", "line\\n")
 * - **Characters**: Single character literals ('a', '\\n', '\\u1234')
 * - **Booleans**: Boolean literals (true, false)
 * - **Null**: Null pointer literal (null)
 * 
 * **Type Inference:**
 * The semantic analyzer determines the specific type of each literal based on
 * its format and value. For example:
 * - 42 → int32 (default integer type)
 * - 42.0 → double (default floating-point type)
 * - "hello" → str (string type)
 * - 'c' → char (character type)
 * - true → bool (boolean type)
 */
class LiteralExpression : public Expression {
public:    
    /**
     * @enum LiteralType
     * @brief Categorizes the different kinds of literal values
     * 
     * This enum distinguishes between different categories of literals to
     * enable proper type checking and code generation. Each category may
     * have multiple concrete types (e.g., NUMBER includes integers and floats).
     */
    enum class LiteralType {
        NUMBER,        // Numeric literals (integers and floating-point)
        STRING,        // String literals with escape sequence support
        CHAR,          // Character literals (single Unicode code points)
        BOOLEAN,       // Boolean literals (true/false)
        NULL_LITERAL   // Null pointer literal
    };
    
    LiteralType literalType;    // The category of this literal
    std::string value;          // The literal value as it appears in source (or processed form)
    
    /**
     * @brief Constructs a new LiteralExpression
     * @param type The category of literal (NUMBER, STRING, etc.)
     * @param value The literal value as a string
     * @param line Source line number
     * @param column Source column number
     * 
     * The value string contains:
     * - For numbers: The numeric value (possibly in different bases)
     * - For strings: The processed string content (escape sequences resolved)
     * - For characters: The character value (escape sequences resolved)
     * - For booleans: "true" or "false"
     * - For null: "null"
     */
    LiteralExpression(LiteralType type, const std::string& value, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this literal
     * @return Formatted string showing the literal type and value
     * 
     * Example outputs:
     * - "LiteralExpression(NUMBER: 42)"
     * - "LiteralExpression(STRING: \"hello\")"
     * - "LiteralExpression(BOOLEAN: true)"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this literal expression
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class IdentifierExpression
 * @brief Represents references to variables, functions, and other named entities
 * 
 * IdentifierExpression represents the use of a name to reference a previously
 * declared entity. This includes variables, function names, and any other
 * identifiers that can be looked up in the symbol table.
 * 
 * **Symbol Resolution:**
 * During semantic analysis, each identifier is resolved to its declaration
 * through symbol table lookup. This process:
 * - Finds the declaration that the identifier refers to
 * - Determines the type of the referenced entity
 * - Validates that the identifier is accessible in the current scope
 * - Reports errors for undefined or inaccessible identifiers
 * 
 * **Usage Contexts:**
 * - Variable references in expressions
 * - Function names in call expressions
 * - Parameter names in function bodies
 * - Any named entity reference
 */
class IdentifierExpression : public Expression {
public:
    std::string name;    // The identifier name as it appears in source code
    
    /**
     * @brief Constructs a new IdentifierExpression
     * @param name The identifier name
     * @param line Source line number
     * @param column Source column number
     * 
     * The name should exactly match how the identifier appears in the source
     * code, preserving case sensitivity and any other lexical properties.
     */
    IdentifierExpression(const std::string& name, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this identifier
     * @return Formatted string showing the identifier name
     * 
     * Example output: "IdentifierExpression(myVariable)"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this identifier expression
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class BinaryOpExpression
 * @brief Represents binary operations between two expressions
 * 
 * BinaryOpExpression handles all forms of binary operations in EMLang, including
 * arithmetic, logical, comparison, and assignment operations. Binary operations
 * take two operands and produce a result based on the operation type.
 * 
 * **Operation Categories:**
 * - **Arithmetic**: +, -, *, /, % (mathematical operations)
 * - **Comparison**: ==, !=, <, >, <=, >= (relational operations)
 * - **Logical**: &&, || (boolean operations with short-circuit evaluation)
 * - **Assignment**: = (assignment operations)
 * - **Bitwise**: &, |, ^, <<, >> (bit manipulation operations)
 * 
 * **Type Checking:**
 * The semantic analyzer validates that:
 * - Both operands have compatible types for the operation
 * - The operation is valid for the operand types
 * - Type promotion rules are applied correctly
 * - The result type is correctly inferred
 * 
 * **Precedence and Associativity:**
 * The parser handles operator precedence during AST construction, so the
 * binary operation tree reflects the correct evaluation order.
 */
class BinaryOpExpression : public Expression {
public:
    ExpressionPtr left;        // Left operand expression
    std::string operator_;     // Operator symbol (+, -, *, ==, etc.)
    ExpressionPtr right;       // Right operand expression
    
    /**
     * @brief Constructs a new BinaryOpExpression
     * @param left Left operand expression (takes ownership)
     * @param op Operator symbol as string
     * @param right Right operand expression (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The binary operation takes ownership of both operand expressions through
     * unique_ptr, ensuring proper memory management and preventing dangling pointers.
     */
    BinaryOpExpression(ExpressionPtr left, const std::string& op, ExpressionPtr right, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this binary operation
     * @return Formatted string showing the operation structure
     * 
     * Example output: "BinaryOpExpression(left + right)" where left and right
     * are recursively formatted representations of the operand expressions.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this binary operation
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     * The visitor typically processes the operands recursively before
     * handling the operation itself.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class UnaryOpExpression
 * @brief Represents unary operations on a single expression
 * 
 * UnaryOpExpression handles prefix unary operations that take a single operand
 * and produce a result. These operations are applied before their operand is
 * evaluated in the expression hierarchy.
 * 
 * **Supported Operations:**
 * - **Arithmetic Negation**: - (produces the negative of a numeric value)
 * - **Logical Negation**: ! (produces the logical NOT of a boolean value)
 * - **Address-Of**: & (produces a pointer to the operand's memory location)
 * 
 * **Type Checking:**
 * Each unary operation has specific type requirements:
 * - Arithmetic negation requires numeric types
 * - Logical negation requires boolean types
 * - Address-of requires lvalue expressions (addressable locations)
 * 
 * **Result Types:**
 * - Arithmetic negation preserves the operand type
 * - Logical negation always produces bool type
 * - Address-of produces a pointer type to the operand type
 */
class UnaryOpExpression : public Expression {
public:
    std::string operator_;     // Operator symbol (-, !, &)
    ExpressionPtr operand;     // The expression being operated on
    
    /**
     * @brief Constructs a new UnaryOpExpression
     * @param op Operator symbol as string
     * @param operand Expression to operate on (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The unary operation takes ownership of the operand expression through
     * unique_ptr, ensuring proper memory management.
     */
    UnaryOpExpression(const std::string& op, ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this unary operation
     * @return Formatted string showing the operation structure
     * 
     * Example output: "UnaryOpExpression(-operand)" where operand is the
     * recursively formatted representation of the operand expression.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this unary operation
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class FunctionCallExpression
 * @brief Represents function call expressions with arguments
 * 
 * FunctionCallExpression represents the invocation of a function with a list
 * of argument expressions. This includes both user-defined functions and any
 * built-in functions that might be added to the language.
 * 
 * **Function Resolution:**
 * During semantic analysis, the function name is resolved to:
 * - A function declaration in the symbol table
 * - Parameter type information for argument checking
 * - Return type information for expression typing
 * 
 * **Argument Validation:**
 * The semantic analyzer ensures that:
 * - The number of arguments matches the function's parameter count
 * - Each argument type is compatible with the corresponding parameter type
 * - All required arguments are provided
 * - No extra arguments are provided
 * 
 * **Type System Integration:**
 * The call expression's type is determined by the function's declared return
 * type, allowing it to be used in any context where that type is expected.
 */
class FunctionCallExpression : public Expression {
public:
    std::string functionName;               // Name of the function being called
    std::vector<ExpressionPtr> arguments;   // Argument expressions in call order
    
    /**
     * @brief Constructs a new FunctionCallExpression
     * @param name Function name to call
     * @param args Vector of argument expressions (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The function call takes ownership of all argument expressions through
     * the vector of unique_ptr, ensuring proper memory management.
     */
    FunctionCallExpression(const std::string& name, std::vector<ExpressionPtr> args, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this function call
     * @return Formatted string showing the call structure
     * 
     * Example output: "FunctionCallExpression(myFunc(arg1, arg2))" where
     * arg1 and arg2 are recursively formatted argument expressions.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this function call
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class DereferenceExpression
 * @brief Represents pointer dereference operations (*ptr)
 * 
 * DereferenceExpression represents the operation of accessing the value that
 * a pointer points to. This is a fundamental operation in pointer-based
 * programming and is essential for working with dynamic memory and references.
 * 
 * **Type Checking:**
 * The semantic analyzer ensures that:
 * - The operand has a pointer type (ends with *)
 * - The pointer is not null (when possible to determine)
 * - The resulting type is the pointed-to type (removes one level of *)
 * 
 * **Safety Considerations:**
 * Dereferencing requires careful validation:
 * - Null pointer dereference detection
 * - Bounds checking for array pointers
 * - Lifetime analysis for dangling pointers
 * 
 * **Example Usage:**
 * ```
 * let ptr: int32* = &x;
 * let value: int32 = *ptr;  // DereferenceExpression
 * ```
 */
class DereferenceExpression : public Expression {
public:
    ExpressionPtr operand;     // Expression that should evaluate to a pointer
    
    /**
     * @brief Constructs a new DereferenceExpression
     * @param operand Expression to dereference (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The dereference operation takes ownership of the operand expression.
     */
    DereferenceExpression(ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this dereference operation
     * @return Formatted string showing the dereference structure
     * 
     * Example output: "DereferenceExpression(*operand)" where operand is
     * the recursively formatted operand expression.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this dereference operation
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class AddressOfExpression
 * @brief Represents address-of operations (&var)
 * 
 * AddressOfExpression represents the operation of obtaining the memory address
 * of a variable or other addressable expression. This creates a pointer to the
 * operand and is fundamental for pointer-based programming.
 * 
 * **Addressability Requirements:**
 * Only certain expressions can have their address taken:
 * - Variables (both local and global)
 * - Array elements
 * - Structure/object fields
 * - Dereferenced pointers
 * 
 * **Type Transformation:**
 * The address-of operation transforms the operand type by adding one level
 * of pointer indirection:
 * - int32 → int32*
 * - char → char*
 * - MyType → MyType*
 * 
 * **Example Usage:**
 * ```
 * let x: int32 = 42;
 * let ptr: int32* = &x;  // AddressOfExpression
 * ```
 */
class AddressOfExpression : public Expression {
public:
    ExpressionPtr operand;     // Expression whose address is being taken
    
    /**
     * @brief Constructs a new AddressOfExpression
     * @param operand Expression to take address of (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The address-of operation takes ownership of the operand expression.
     */
    AddressOfExpression(ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this address-of operation
     * @return Formatted string showing the address-of structure
     * 
     * Example output: "AddressOfExpression(&operand)" where operand is
     * the recursively formatted operand expression.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this address-of operation
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class VariableDeclaration
 * @brief Represents variable declaration statements (let/const)
 * 
 * VariableDeclaration handles the declaration of new variables in the program,
 * including both mutable (let) and immutable (const) variables. This statement
 * introduces a new binding into the current scope and optionally initializes
 * the variable with a value.
 * 
 * **Declaration Syntax:**
 * - `let variableName: type = initializer;` (mutable variable)
 * - `const variableName: type = initializer;` (immutable variable)
 * - `let variableName: type;` (uninitialized mutable variable)
 * 
 * **Semantic Rules:**
 * - Variable names must be unique within their scope
 * - Const variables must have an initializer
 * - The initializer type must be compatible with the declared type
 * - Type annotations are required in EMLang (no type inference for declarations)
 * 
 * **Scope Integration:**
 * Variable declarations introduce new symbols into the symbol table for the
 * current scope. The semantic analyzer ensures that:
 * - No duplicate names exist in the same scope
 * - The declared type is valid
 * - Initializer compatibility is maintained
 * - Const variables are never modified after declaration
 */
class VariableDeclaration : public Statement {
public:
    std::string name;           // Variable name as declared
    std::string type;           // Explicit type annotation (e.g., "int32", "str", "bool")
    ExpressionPtr initializer;  // Optional initialization expression (nullptr if none)
    bool isConstant;           // true for const declarations, false for let declarations
    
    /**
     * @brief Constructs a new VariableDeclaration
     * @param name Variable name
     * @param type Type annotation string
     * @param init Initializer expression (can be nullptr for uninitialized)
     * @param isConst true for const variables, false for let variables
     * @param line Source line number
     * @param column Source column number
     * 
     * For const variables, init should not be nullptr as const variables
     * require initialization. The semantic analyzer will enforce this rule.
     */
    VariableDeclaration(const std::string& name, const std::string& type, ExpressionPtr init, bool isConst, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this variable declaration
     * @return Formatted string showing the declaration structure
     * 
     * Example outputs:
     * - "VariableDeclaration(let x: int32 = 42)"
     * - "VariableDeclaration(const PI: double = 3.14159)"
     * - "VariableDeclaration(let buffer: str)"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this variable declaration
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class FunctionDeclaration
 * @brief Represents function declaration statements
 * 
 * FunctionDeclaration handles the declaration of functions, which are named
 * code blocks that can accept parameters and return values. Functions are
 * first-class entities in EMLang and can be called from expressions.
 * 
 * **Function Syntax:**
 * ```
 * function functionName(param1: type1, param2: type2): returnType {
 *     // function body
 *     return expression;
 * }
 * ```
 * 
 * **Components:**
 * - **Name**: Unique identifier for the function
 * - **Parameters**: Typed parameter list for input values
 * - **Return Type**: Type of value the function returns
 * - **Body**: Block statement containing the function implementation
 * 
 * **Semantic Analysis:**
 * The semantic analyzer validates:
 * - Function name uniqueness in the current scope
 * - Parameter name uniqueness within the function
 * - Return statement compatibility with declared return type
 * - All execution paths return a value (for non-void functions)
 * - No use of undefined variables in the function body
 * 
 * **Symbol Table Integration:**
 * Functions are added to the symbol table with their signature information,
 * enabling type checking of function calls and overload resolution.
 */
class FunctionDeclaration : public Statement {
public:
    /**
     * @struct Parameter
     * @brief Represents a function parameter with name and type
     * 
     * Each parameter has a name (used within the function body) and a type
     * (used for argument validation during function calls).
     */
    struct Parameter {
        std::string name;    // Parameter name for use in function body
        std::string type;    // Parameter type for type checking
    };
    
    std::string name;                      // Function name
    std::vector<Parameter> parameters;     // Function parameters in declaration order
    std::string returnType;                // Return type annotation
    StatementPtr body;                     // Function body (typically a BlockStatement)
    
    /**
     * @brief Constructs a new FunctionDeclaration
     * @param name Function name
     * @param params Vector of parameters with names and types
     * @param retType Return type annotation string
     * @param body Function body statement (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The function declaration takes ownership of the body statement, which
     * typically contains the implementation logic.
     */
    FunctionDeclaration(const std::string& name, std::vector<Parameter> params, const std::string& retType, StatementPtr body, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this function declaration
     * @return Formatted string showing the function signature and body
     * 
     * Example output: "FunctionDeclaration(add(x: int32, y: int32): int32 { ... })"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this function declaration
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class BlockStatement
 * @brief Represents compound statements containing multiple statements
 * 
 * BlockStatement represents a sequence of statements enclosed in braces that
 * are executed sequentially. Blocks create their own lexical scope, meaning
 * variables declared within a block are only accessible within that block
 * and its nested blocks.
 * 
 * **Scope Semantics:**
 * - Creates a new lexical scope for contained statements
 * - Variables declared in the block are not accessible outside it
 * - Can access variables from enclosing scopes
 * - Supports nested blocks with proper scoping rules
 * 
 * **Execution Model:**
 * Statements within the block are executed in sequential order from first
 * to last. Control flow statements (if, while, return) may alter this
 * normal execution order.
 * 
 * **Usage Contexts:**
 * - Function bodies
 * - Conditional statement branches (if/else)
 * - Loop bodies (while)
 * - Standalone compound statements
 * 
 * **Example:**
 * ```
 * {
 *     let x: int32 = 10;
 *     let y: int32 = 20;
 *     print(x + y);
 * }
 * ```
 */
class BlockStatement : public Statement {
public:
    std::vector<StatementPtr> statements;    // Statements contained in this block
    
    /**
     * @brief Constructs a new BlockStatement
     * @param stmts Vector of statements to include in the block (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The block statement takes ownership of all contained statements through
     * the vector of unique_ptr, ensuring proper memory management.
     */
    BlockStatement(std::vector<StatementPtr> stmts, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this block statement
     * @return Formatted string showing the block structure
     * 
     * Example output: "BlockStatement({ statement1; statement2; statement3; })"
     * where each statement is recursively formatted.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this block statement
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     * The visitor typically processes each contained statement in order.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class IfStatement
 * @brief Represents conditional statements (if/else)
 * 
 * IfStatement implements conditional execution based on a boolean expression.
 * It supports both simple if statements and if-else statements with optional
 * else branches. This is a fundamental control flow construct that allows
 * programs to make decisions based on runtime conditions.
 * 
 * **Conditional Logic:**
 * - Evaluates a condition expression to determine execution path
 * - Executes the then-branch if condition is true
 * - Executes the else-branch (if present) if condition is false
 * - Supports nested if statements for complex decision logic
 * 
 * **Type Requirements:**
 * - Condition expression must evaluate to boolean type
 * - Then and else branches can be any statement type
 * - Common to use block statements for multi-statement branches
 * 
 * **Syntax Forms:**
 * ```
 * if (condition) thenStatement
 * 
 * if (condition) thenStatement else elseStatement
 * 
 * if (condition) {
 *     // multiple statements
 * } else {
 *     // multiple statements
 * }
 * ```
 * 
 * **Control Flow:**
 * The if statement alters normal sequential execution by potentially
 * skipping the then-branch or else-branch based on the condition value.
 */
class IfStatement : public Statement {
public:
    ExpressionPtr condition;     // Boolean expression determining which branch to execute
    StatementPtr thenBranch;     // Statement executed when condition is true
    StatementPtr elseBranch;     // Statement executed when condition is false (can be nullptr)
    
    /**
     * @brief Constructs a new IfStatement
     * @param cond Condition expression (takes ownership)
     * @param then Then-branch statement (takes ownership)
     * @param else_ Else-branch statement (takes ownership, can be nullptr)
     * @param line Source line number
     * @param column Source column number
     * 
     * The else-branch can be nullptr for simple if statements without else.
     * Both branches typically contain block statements for multi-statement bodies.
     */
    IfStatement(ExpressionPtr cond, StatementPtr then, StatementPtr else_, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this if statement
     * @return Formatted string showing the conditional structure
     * 
     * Example outputs:
     * - "IfStatement(if (condition) thenBranch)"
     * - "IfStatement(if (condition) thenBranch else elseBranch)"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this if statement
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class WhileStatement
 * @brief Represents while loop statements for iterative execution
 * 
 * WhileStatement implements conditional repetition by repeatedly executing
 * a body statement as long as a condition expression evaluates to true.
 * This is a fundamental control flow construct for implementing loops
 * and iterative algorithms.
 * 
 * **Loop Semantics:**
 * - Evaluates condition before each iteration (pre-test loop)
 * - Executes body statement if condition is true
 * - Re-evaluates condition after each iteration
 * - Exits loop when condition becomes false
 * - May never execute body if condition is initially false
 * 
 * **Type Requirements:**
 * - Condition expression must evaluate to boolean type
 * - Body can be any statement type (commonly a block statement)
 * 
 * **Control Flow:**
 * The while loop creates a backward jump in the control flow graph,
 * potentially executing the same code multiple times. This requires
 * careful analysis for:
 * - Loop termination guarantees
 * - Variable modification tracking
 * - Optimization opportunities
 * 
 * **Example:**
 * ```
 * let i: int32 = 0;
 * while (i < 10) {
 *     print(i);
 *     i = i + 1;
 * }
 * ```
 */
class WhileStatement : public Statement {
public:
    ExpressionPtr condition;     // Boolean expression controlling loop execution
    StatementPtr body;          // Statement executed repeatedly while condition is true
    
    /**
     * @brief Constructs a new WhileStatement
     * @param cond Condition expression (takes ownership)
     * @param body Loop body statement (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The condition is evaluated before each iteration, and the body
     * is executed only when the condition evaluates to true.
     */
    WhileStatement(ExpressionPtr cond, StatementPtr body, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this while statement
     * @return Formatted string showing the loop structure
     * 
     * Example output: "WhileStatement(while (condition) body)"
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this while statement
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ReturnStatement
 * @brief Represents return statements for function exit with optional value
 * 
 * ReturnStatement implements function termination and value return. It
 * immediately exits the current function and optionally provides a return
 * value to the caller. This is essential for function-based programming
 * and control flow management.
 * 
 * **Return Semantics:**
 * - Immediately terminates execution of the current function
 * - Returns control to the function's caller
 * - Optionally provides a return value
 * - Can appear anywhere within a function body
 * - Must be compatible with function's declared return type
 * 
 * **Type Checking:**
 * The semantic analyzer ensures that:
 * - Return statements only appear within function bodies
 * - Return value type matches function's declared return type
 * - Void functions don't return values
 * - Non-void functions return appropriate values
 * - All execution paths in non-void functions have returns
 * 
 * **Control Flow Impact:**
 * Return statements create early exits from functions, which affects:
 * - Dead code analysis (code after unconditional returns)
 * - Path analysis for ensuring all paths return values
 * - Resource cleanup and scope management
 * 
 * **Examples:**
 * ```
 * return;              // Void return (no value)
 * return 42;           // Return with integer value
 * return x + y;        // Return with expression value
 * ```
 */
class ReturnStatement : public Statement {
public:
    ExpressionPtr value;    // Return value expression (nullptr for void returns)
    
    /**
     * @brief Constructs a new ReturnStatement
     * @param val Return value expression (takes ownership, can be nullptr for void)
     * @param line Source line number
     * @param column Source column number
     * 
     * For void functions, val should be nullptr. For non-void functions,
     * val should contain an expression that evaluates to the correct return type.
     */
    ReturnStatement(ExpressionPtr val, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this return statement
     * @return Formatted string showing the return structure
     * 
     * Example outputs:
     * - "ReturnStatement(return;)" for void returns
     * - "ReturnStatement(return value)" for value returns
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this return statement
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ExpressionStatement
 * @brief Represents expressions used as statements for their side effects
 * 
 * ExpressionStatement wraps an expression to use it as a statement. This
 * allows expressions that produce side effects (like function calls or
 * assignments) to be used in statement contexts where their return value
 * is ignored.
 * 
 * **Use Cases:**
 * - Function calls where the return value is not needed
 * - Assignment expressions in languages that treat assignment as expression
 * - Any expression executed for its side effects rather than its value
 * - Method calls that modify object state
 * 
 * **Side Effects:**
 * Common side effects that make expressions useful as statements:
 * - I/O operations (print, read)
 * - Memory allocation/deallocation
 * - Global variable modification
 * - Function calls that modify external state
 * - System calls and library function invocations
 * 
 * **Type Considerations:**
 * While the expression has a type and produces a value, that value is
 * discarded when used as a statement. The semantic analyzer may warn
 * about unused values from pure expressions without side effects.
 * 
 * **Examples:**
 * ```
 * print("Hello, World!");     // Function call statement
 * x = y + z;                  // Assignment statement (if supported)
 * myFunction();               // Function call with ignored return value
 * ```
 */
class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;    // Expression being executed as a statement
    
    /**
     * @brief Constructs a new ExpressionStatement
     * @param expr Expression to execute (takes ownership)
     * @param line Source line number
     * @param column Source column number
     * 
     * The expression is evaluated for its side effects, and any return
     * value is discarded.
     */
    ExpressionStatement(ExpressionPtr expr, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Returns a string representation of this expression statement
     * @return Formatted string showing the statement structure
     * 
     * Example output: "ExpressionStatement(expression;)" where expression
     * is the recursively formatted contained expression.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this expression statement
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class Program
 * @brief Root node representing an entire EMLang program
 * 
 * Program is the root node of the AST that contains all top-level statements
 * in an EMLang source file. It represents the complete program structure and
 * serves as the entry point for all AST operations including semantic analysis,
 * code generation, and program execution.
 * 
 * **Program Structure:**
 * An EMLang program consists of a sequence of top-level statements that can include:
 * - Global variable declarations
 * - Function declarations
 * - Import/module statements (future extension)
 * - Type declarations (future extension)
 * 
 * **Execution Model:**
 * - Top-level statements are conceptually executed in order
 * - Variable declarations create global bindings
 * - Function declarations register functions in the global scope
 * - Program execution typically starts with a main function
 * 
 * **Semantic Analysis Entry Point:**
 * The Program node serves as the starting point for semantic analysis,
 * which processes all top-level declarations to build the global symbol
 * table and validate the program structure.
 * 
 * **Compilation Pipeline:**
 * 1. Lexical analysis produces tokens
 * 2. Parser creates Program AST from tokens
 * 3. Semantic analyzer validates Program and children
 * 4. Code generator traverses Program to emit target code
 * 
 * **Example Program Structure:**
 * ```
 * // Global variable declaration
 * let globalVar: int32 = 100;
 * 
 * // Function declaration
 * function main(): int32 {
 *     return 0;
 * }
 * ```
 */
class Program : public ASTNode {
public:
    std::vector<StatementPtr> statements;    // Top-level statements in the program
    
    /**
     * @brief Constructs a new Program node
     * @param stmts Vector of top-level statements (takes ownership)
     * 
     * The program takes ownership of all top-level statements, which typically
     * include variable declarations, function declarations, and other global
     * constructs.
     * 
     * Note: No line/column parameters as Program represents the entire file.
     */
    Program(std::vector<StatementPtr> stmts);
    
    /**
     * @brief Returns a string representation of this program
     * @return Formatted string showing the complete program structure
     * 
     * Example output: "Program(statement1; statement2; function main() { ... })"
     * where each statement is recursively formatted.
     */
    std::string toString() const override;
    
    /**
     * @brief Accepts a visitor for processing this program
     * @param visitor The visitor to accept
     * 
     * Calls visitor.visit(*this) to enable type-safe visitor dispatch.
     * This is typically the entry point for AST traversal operations.
     */
    void accept(ASTVisitor& visitor) override;
};

/**
 * @class ASTVisitor
 * @brief Abstract visitor interface for traversing and processing AST nodes
 * 
 * ASTVisitor implements the Visitor design pattern to enable extensible
 * operations on the AST without modifying the node classes themselves. This
 * pattern is essential for the compiler architecture as it allows different
 * phases (semantic analysis, code generation, optimization) to operate on
 * the same AST structure with different behaviors.
 * 
 * **Visitor Pattern Benefits:**
 * - **Extensibility**: Add new operations without modifying existing AST classes
 * - **Type Safety**: Compile-time dispatch to appropriate visit methods
 * - **Separation of Concerns**: Keep data structure separate from algorithms
 * - **Multiple Passes**: Different visitors can traverse the same AST
 * - **Maintainability**: Centralize related operations in single visitor classes
 * 
 * **Usage Pattern:**
 * 1. Create a concrete visitor class inheriting from ASTVisitor
 * 2. Implement visit methods for relevant node types
 * 3. Call accept() on AST nodes with the visitor instance
 * 4. The accept() method dispatches to the correct visit() method
 * 
 * **Compiler Phase Integration:**
 * Different compiler phases implement specific visitors:
 * - **SemanticAnalyzer**: Validates types and builds symbol tables
 * - **CodeGenerator**: Emits target machine code or bytecode
 * - **Optimizer**: Performs AST transformations for performance
 * - **PrettyPrinter**: Formats source code from AST
 * - **DebugInfoGenerator**: Extracts debugging information
 * 
 * **Implementation Requirements:**
 * - All visit methods must be implemented (pure virtual)
 * - Visit methods should handle the specific node type appropriately
 * - Recursive traversal is typically implemented within visit methods
 * - Visitors may maintain state between node visits
 * 
 * **Example Visitor:**
 * ```cpp
 * class TypeChecker : public ASTVisitor {
 * private:
 *     std::map<std::string, Type> symbolTable;
 * 
 * public:
 *     void visit(VariableDeclaration& node) override {
 *         // Validate type and add to symbol table
 *         symbolTable[node.name] = resolveType(node.type);
 *     }
 *     
 *     void visit(BinaryOpExpression& node) override {
 *         // Check operand types and validate operation
 *         node.left->accept(*this);
 *         node.right->accept(*this);
 *         validateBinaryOperation(node);
 *     }
 *     // ... other visit methods
 * };
 * ```
 */
class ASTVisitor {
public:
    /**
     * @brief Virtual destructor for proper polymorphic cleanup
     * 
     * Ensures that derived visitor classes are properly destroyed when
     * deleted through base class pointers.
     */
    virtual ~ASTVisitor() = default;
    
    // ======================== EXPRESSION VISITORS ========================
    // These methods handle expression nodes that produce values
    
    /**
     * @brief Visits a literal expression node
     * @param node The literal expression to process
     * 
     * Typically handles literal value processing, type assignment,
     * and constant folding opportunities.
     */
    virtual void visit(LiteralExpression& node) = 0;
    
    /**
     * @brief Visits an identifier expression node
     * @param node The identifier expression to process
     * 
     * Typically handles symbol lookup, type resolution, and
     * scope validation for variable/function references.
     */
    virtual void visit(IdentifierExpression& node) = 0;
    
    /**
     * @brief Visits a binary operation expression node
     * @param node The binary operation to process
     * 
     * Typically handles operand processing, type checking,
     * operator validation, and result type inference.
     */
    virtual void visit(BinaryOpExpression& node) = 0;
    
    /**
     * @brief Visits a unary operation expression node
     * @param node The unary operation to process
     * 
     * Typically handles operand processing, type checking,
     * and result type determination.
     */
    virtual void visit(UnaryOpExpression& node) = 0;
    
    /**
     * @brief Visits a function call expression node
     * @param node The function call to process
     * 
     * Typically handles function resolution, argument validation,
     * parameter matching, and return type determination.
     */
    virtual void visit(FunctionCallExpression& node) = 0;
    
    /**
     * @brief Visits a pointer dereference expression node
     * @param node The dereference operation to process
     * 
     * Typically handles pointer type validation, null checking,
     * and pointed-to type resolution.
     */
    virtual void visit(DereferenceExpression& node) = 0;
    
    /**
     * @brief Visits an address-of expression node
     * @param node The address-of operation to process
     * 
     * Typically handles addressability validation and
     * pointer type creation.
     */
    virtual void visit(AddressOfExpression& node) = 0;
    
    // ======================== STATEMENT VISITORS ========================
    // These methods handle statement nodes that perform actions
    
    /**
     * @brief Visits a variable declaration statement node
     * @param node The variable declaration to process
     * 
     * Typically handles symbol table updates, type validation,
     * initializer checking, and scope management.
     */
    virtual void visit(VariableDeclaration& node) = 0;
    
    /**
     * @brief Visits a function declaration statement node
     * @param node The function declaration to process
     * 
     * Typically handles function signature registration,
     * parameter validation, and body analysis.
     */
    virtual void visit(FunctionDeclaration& node) = 0;
    
    /**
     * @brief Visits a block statement node
     * @param node The block statement to process
     * 
     * Typically handles scope creation, sequential statement
     * processing, and scope cleanup.
     */
    virtual void visit(BlockStatement& node) = 0;
    
    /**
     * @brief Visits an if statement node
     * @param node The if statement to process
     * 
     * Typically handles condition validation, branch analysis,
     * and control flow tracking.
     */
    virtual void visit(IfStatement& node) = 0;
    
    /**
     * @brief Visits a while statement node
     * @param node The while statement to process
     * 
     * Typically handles condition validation, loop body analysis,
     * and termination checking.
     */
    virtual void visit(WhileStatement& node) = 0;
    
    /**
     * @brief Visits a return statement node
     * @param node The return statement to process
     * 
     * Typically handles return type validation, function context
     * checking, and control flow analysis.
     */
    virtual void visit(ReturnStatement& node) = 0;
    
    /**
     * @brief Visits an expression statement node
     * @param node The expression statement to process
     * 
     * Typically handles expression evaluation and side effect
     * analysis while discarding the result value.
     */
    virtual void visit(ExpressionStatement& node) = 0;
    
    // ======================== PROGRAM VISITOR ========================
    // This method handles the root node
    
    /**
     * @brief Visits a program node (root of AST)
     * @param node The program to process
     * 
     * Typically handles global scope initialization, top-level
     * statement processing, and overall program validation.
     */
    virtual void visit(Program& node) = 0;
};

} // namespace emlang

#endif // EM_LANG_AST_H
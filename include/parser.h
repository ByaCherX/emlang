//===--- parser.h - Parser interface ------------------------ ---*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # Syntax analyzer for the EMLang programming language
//
// This file contains the parser implementation for EMLang, which is responsible for
// analyzing the sequence of tokens produced by the lexer and constructing an
// Abstract Syntax Tree (AST) that represents the program's structure.
// 
// The parser implements a recursive descent parsing algorithm with operator precedence
// handling. It processes tokens according to EMLang's grammar rules and creates
// appropriate AST nodes for each syntactic construct.
// 
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_PARSER_H
#define EM_LANG_PARSER_H

#pragma once

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

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>


namespace emlang {

/**
 * @class Parser
 * @brief Recursive descent parser for EMLang source code
 * 
 * The Parser class takes a sequence of tokens from the lexer and builds an Abstract
 * Syntax Tree (AST) according to EMLang's grammar. It implements a recursive descent
 * parser with the following characteristics:
 * 
 * **Parsing Strategy:**
 * - Top-down recursive descent for statements and declarations
 * - Operator precedence climbing for expressions
 * - Lookahead for disambiguation
 * - Error recovery through synchronization points
 * 
 * **Grammar Hierarchy:**
 * ```
 * Program -> Statement*
 * Statement -> VarDecl | FuncDecl | IfStmt | WhileStmt | ForStmt | 
 *              ReturnStmt | BlockStmt | ExprStmt
 * Expression -> LogicalOr
 * LogicalOr -> LogicalAnd ('||' LogicalAnd)*
 * LogicalAnd -> Equality ('&&' Equality)*
 * Equality -> Comparison (('==' | '!=') Comparison)*
 * Comparison -> Term (('>' | '>=' | '<' | '<=') Term)*
 * Term -> Factor (('+' | '-') Factor)*
 * Factor -> Unary (('*' | '/' | '%') Unary)*
 * Unary -> ('!' | '-' | '&') Unary | Call
 * Call -> Primary ('(' Arguments? ')')*
 * Primary -> NUMBER | STRING | CHAR | IDENTIFIER | '(' Expression ')'
 * ```
 * 
 * **Error Handling:**
 * The parser uses panic-mode error recovery, where it:
 * 1. Reports the error with precise location information
 * 2. Synchronizes to a known good state (statement boundaries)
 * 3. Continues parsing to find additional errors
 * 
 * **Usage Example:**
 * @code
 * Lexer lexer("let x: int32 = 42;");
 * std::vector<Token> tokens = lexer.tokenize();
 * Parser parser(tokens);
 * std::unique_ptr<Program> ast = parser.parse();
 * @endcode
 */
class EMLANG_API Parser {
private:
    std::vector<Token> tokens;   // Complete token sequence from lexer
    size_t current;              // Index of current token being processed (0-based)
    
    // ======================== TOKEN NAVIGATION METHODS ========================
    
    /**
     * @brief Returns a reference to the current token being processed
     * @return Reference to the token at the current position
     * 
     * This method provides access to the token currently being examined by the parser.
     * It's used throughout the parsing process to check token types and extract values.
     * The method assumes that current < tokens.size() and may throw if called at EOF.
     */
    Token& currentToken();
    
    /**
     * @brief Returns a reference to a token at a specified offset from current position
     * @param offset Number of positions to look ahead (default: 1)
     * @return Reference to the token at current + offset position
     * 
     * This method implements lookahead functionality, allowing the parser to examine
     * upcoming tokens without consuming them. This is essential for:
     * - Disambiguating grammar constructs
     * - Making parsing decisions based on context
     * - Implementing backtracking where necessary
     * 
     * Example: peekToken(1) returns the next token, peekToken(2) returns the token after that
     */
    Token& peekToken(size_t offset = 1);
    
    /**
     * @brief Checks if the parser has reached the end of the token stream
     * @return true if no more tokens to process, false otherwise
     * 
     * This method is used to detect when parsing should terminate. It's called
     * before attempting to access tokens to prevent out-of-bounds errors.
     * The method typically checks for EOF_TOKEN or position >= tokens.size().
     */
    bool isAtEnd();
    
    /**
     * @brief Checks if the current token is of the specified type
     * @param type The TokenType to check against
     * @return true if current token matches the type, false otherwise
     * 
     * This is a non-consuming check that allows the parser to make decisions
     * based on the current token type without advancing the position.
     * Used extensively in conditional parsing logic.
     */
    bool check(TokenType type);
    
    /**
     * @brief Checks if current token matches type and consumes it if so
     * @param type The TokenType to match against
     * @return true if token was matched and consumed, false otherwise
     * 
     * This method combines checking and consuming in one operation:
     * - If current token matches the type: advance position and return true
     * - If current token doesn't match: leave position unchanged and return false
     * 
     * Commonly used for optional syntax elements and alternative parsing paths.
     */
    bool match(TokenType type);
    
    /**
     * @brief Checks if current token matches any of the provided types
     * @param types Vector of TokenTypes to match against
     * @return true if current token matches any type and is consumed, false otherwise
     * 
     * This overloaded version allows matching against multiple possible token types
     * in a single call. Useful for handling alternative syntax constructs or
     * multiple operators with the same precedence.
     * 
     * Example: match({PLUS, MINUS}) for additive operators
     */
    bool match(std::vector<TokenType> types);
    
    /**
     * @brief Consumes and returns the current token, advancing to the next
     * @return The token that was consumed
     * 
     * This method moves the parser forward by one token and returns the token
     * that was consumed. It's the primary mechanism for making progress through
     * the token stream during parsing.
     * 
     * Should only be called when certain that a token is available (not at EOF).
     */
    Token advance();
    
    /**
     * @brief Consumes current token if it matches expected type, otherwise reports error
     * @param type The expected TokenType
     * @param message Error message to display if token doesn't match
     * @return The consumed token (if successful)
     * @throws ParseError if current token doesn't match expected type
     * 
     * This method enforces grammar requirements by ensuring that specific tokens
     * appear where expected. If the expectation is not met, it provides detailed
     * error information including the expected vs. actual token and context.
     * 
     * Used for mandatory syntax elements like semicolons, closing braces, etc.
     */
    Token consume(TokenType type, const std::string& message);
    
    // ======================== STATEMENT PARSING METHODS ========================
    
    /**
     * @brief Parses the entire program (top-level parsing entry point)
     * @return Unique pointer to Program AST node containing all statements
     * 
     * This is the main entry point for parsing. It creates the root Program node
     * and parses all top-level statements until EOF is reached. The method:
     * - Creates a new Program AST node
     * - Repeatedly calls parseStatement() until EOF
     * - Handles parse errors and attempts recovery
     * - Returns the complete AST for the program
     */
    std::unique_ptr<Program> parseProgram();
    
    /**
     * @brief Parses a single statement and returns appropriate AST node
     * @return StatementPtr to the parsed statement AST node
     * 
     * This method dispatches to specific statement parsing methods based on
     * the current token. It handles all statement types:
     * - Variable declarations (let, const)
     * - Function declarations
     * - Control flow statements (if, while, for)
     * - Return statements
     * - Block statements
     * - Expression statements
     * 
     * The method uses lookahead to determine the statement type and calls
     * the appropriate specialized parsing method.
     */
    StatementPtr parseStatement();
    
    /**
     * @brief Parses variable declarations (let/const statements)
     * @return StatementPtr to VariableDeclaration AST node
     * 
     * Handles variable declaration syntax:
     * ```
     * let identifier: type = expression;
     * const identifier: type = expression;
     * let identifier = expression;  // type inference
     * ```
     * 
     * The method:
     * - Determines if declaration is mutable (let) or immutable (const)
     * - Parses the identifier name
     * - Optionally parses type annotation
     * - Optionally parses initializer expression
     * - Creates VariableDeclaration AST node with all components
     */
    StatementPtr parseVariableDeclaration();
    
    /**
     * @brief Parses function declarations
     * @return StatementPtr to FunctionDeclaration AST node
     * 
     * Handles function declaration syntax:
     * ```
     * function name(param1: type1, param2: type2): returnType {
     *     // body
     * }
     * ```
     * 
     * The method parses:
     * - Function name identifier
     * - Parameter list with types
     * - Optional return type annotation
     * - Function body as a block statement
     * - Creates FunctionDeclaration AST node
     */
    StatementPtr parseFunctionDeclaration();
    
    /**
     * @brief Parses external function declarations
     * @return StatementPtr to ExternFunctionDeclaration AST node
     * 
     * Handles external function declaration syntax:
     * ```
     * extern function name(param1: type1, param2: type2): returnType;
     * ```
     * 
     * The method parses:
     * - Function name identifier
     * - Parameter list with types
     * - Optional return type annotation
     * - Semicolon termination (no body)
     * - Creates ExternFunctionDeclaration AST node
     */
    StatementPtr parseExternFunctionDeclaration();
    
    /**
     * @brief Parses if/else conditional statements
     * @return StatementPtr to IfStatement AST node
     * 
     * Handles conditional statement syntax:
     * ```
     * if (condition) statement
     * if (condition) statement else statement
     * ```
     * 
     * The method:
     * - Parses the condition expression in parentheses
     * - Parses the then-statement
     * - Optionally parses else-statement if 'else' keyword is present
     * - Creates IfStatement AST node with all components
     */
    StatementPtr parseIfStatement();
    
    /**
     * @brief Parses while loop statements
     * @return StatementPtr to WhileStatement AST node
     * 
     * Handles while loop syntax:
     * ```
     * while (condition) statement
     * ```
     * 
     * The method:
     * - Parses the loop condition expression in parentheses
     * - Parses the loop body statement
     * - Creates WhileStatement AST node
     */
    StatementPtr parseWhileStatement();
    
    /**
     * @brief Parses for loop statements
     * @return StatementPtr to ForStatement AST node
     * 
     * Handles for loop syntax:
     * ```
     * for (init; condition; update) statement
     * ```
     * 
     * The method parses:
     * - Optional initialization statement
     * - Optional condition expression
     * - Optional update expression
     * - Loop body statement
     * - Creates ForStatement AST node
     */
    StatementPtr parseForStatement();
    
    /**
     * @brief Parses return statements
     * @return StatementPtr to ReturnStatement AST node
     * 
     * Handles return statement syntax:
     * ```
     * return;
     * return expression;
     * ```
     * 
     * The method:
     * - Optionally parses return value expression
     * - Ensures proper semicolon termination
     * - Creates ReturnStatement AST node
     */
    StatementPtr parseReturnStatement();
    
    /**
     * @brief Parses block statements (compound statements)
     * @return StatementPtr to BlockStatement AST node
     * 
     * Handles block statement syntax:
     * ```
     * {
     *     statement1;
     *     statement2;
     *     // ...
     * }
     * ```
     * 
     * The method:
     * - Parses opening brace
     * - Repeatedly parses statements until closing brace
     * - Creates BlockStatement AST node containing all statements
     * - Handles nested scoping implications
     */
    StatementPtr parseBlockStatement();
    
    /**
     * @brief Parses expression statements
     * @return StatementPtr to ExpressionStatement AST node
     * 
     * Handles statements that consist of a single expression followed by semicolon:
     * ```
     * expression;
     * functionCall();
     * assignment = value;
     * ```
     * 
     * The method:
     * - Parses the expression
     * - Ensures semicolon termination
     * - Creates ExpressionStatement AST node
     */
    StatementPtr parseExpressionStatement();
    
    // ======================== EXPRESSION PARSING METHODS ========================
    // These methods implement operator precedence parsing using recursive descent
    
    /**
     * @brief Entry point for expression parsing
     * @return ExpressionPtr to the parsed expression AST node
     * 
     * This method starts expression parsing at the lowest precedence level
     * (logical OR operations) and works up through the precedence hierarchy.
     * It serves as the main interface for parsing any expression.
     */
    ExpressionPtr parseExpression();
    
    /**
     * @brief Parses logical OR expressions (lowest precedence)
     * @return ExpressionPtr to LogicalOr or lower-precedence expression
     * 
     * Handles logical OR operations:
     * ```
     * expression || expression || ...
     * ```
     * 
     * Left-associative, lowest precedence. Short-circuit evaluation semantics.
     */
    ExpressionPtr parseLogicalOr();
    
    /**
     * @brief Parses logical AND expressions
     * @return ExpressionPtr to LogicalAnd or lower-precedence expression
     * 
     * Handles logical AND operations:
     * ```
     * expression && expression && ...
     * ```
     * 
     * Left-associative, higher precedence than OR. Short-circuit evaluation.
     */
    ExpressionPtr parseLogicalAnd();
    
    /**
     * @brief Parses equality expressions
     * @return ExpressionPtr to Equality or lower-precedence expression
     * 
     * Handles equality and inequality operations:
     * ```
     * expression == expression
     * expression != expression
     * ```
     * 
     * Left-associative, higher precedence than logical operators.
     */
    ExpressionPtr parseEquality();
    
    /**
     * @brief Parses comparison expressions
     * @return ExpressionPtr to Comparison or lower-precedence expression
     * 
     * Handles relational operations:
     * ```
     * expression < expression
     * expression <= expression
     * expression > expression
     * expression >= expression
     * ```
     * 
     * Left-associative, higher precedence than equality.
     */
    ExpressionPtr parseComparison();
    
    /**
     * @brief Parses additive expressions (terms)
     * @return ExpressionPtr to Term or lower-precedence expression
     * 
     * Handles addition and subtraction:
     * ```
     * expression + expression
     * expression - expression
     * ```
     * 
     * Left-associative, higher precedence than comparison.
     */
    ExpressionPtr parseTerm();
    
    /**
     * @brief Parses multiplicative expressions (factors)
     * @return ExpressionPtr to Factor or lower-precedence expression
     * 
     * Handles multiplication, division, and modulo:
     * ```
     * expression * expression
     * expression / expression
     * expression % expression
     * ```
     * 
     * Left-associative, higher precedence than additive.
     */
    ExpressionPtr parseFactor();
    
    /**
     * @brief Parses unary expressions
     * @return ExpressionPtr to Unary or lower-precedence expression
     * 
     * Handles unary prefix operations:
     * ```
     * !expression    // logical NOT
     * -expression    // arithmetic negation
     * &expression    // address-of
     * ```
     * 
     * Right-associative, higher precedence than binary operators.
     */
    ExpressionPtr parseUnary();
    
    /**
     * @brief Parses function calls and postfix expressions
     * @return ExpressionPtr to Call or lower-precedence expression
     * 
     * Handles function call syntax:
     * ```
     * function()
     * function(arg1, arg2, ...)
     * expression(arguments)
     * ```
     * 
     * Left-associative, higher precedence than unary.
     */
    ExpressionPtr parseCall();
    
    /**
     * @brief Parses primary expressions (highest precedence)
     * @return ExpressionPtr to primary expression AST node
     * 
     * Handles atomic expressions:
     * ```
     * 42              // number literals
     * "hello"         // string literals
     * 'c'             // character literals
     * identifier      // variable references
     * (expression)    // parenthesized expressions
     * ```
     * 
     * These are the building blocks of all expressions.
     */
    ExpressionPtr parsePrimary();
    
    // ======================== HELPER PARSING METHODS ========================
    
    /**
     * @brief Parses function parameter lists
     * @return Vector of Parameter objects containing name and type information
     * 
     * Parses parameter list syntax:
     * ```
     * ()                                   // empty parameter list
     * (param: type)                        // single parameter
     * (param1: type1, param2: type2, ...)  // multiple parameters
     * ```
     * 
     * Each parameter includes:
     * - Parameter name (identifier)
     * - Parameter type (including pointer types)
     * - Optional default values (if supported)
     */
    std::vector<FunctionDeclaration::Parameter> parseParameterList();
    
    /**
     * @brief Parses function call argument lists
     * @return Vector of ExpressionPtr objects representing the arguments
     * 
     * Parses argument list syntax:
     * ```
     * ()                   // no arguments
     * (arg)                // single argument
     * (arg1, arg2, ...)    // multiple arguments
     * ```
     * 
     * Each argument is a full expression that will be evaluated and
     * passed to the function call.
     */
    std::vector<ExpressionPtr> parseArgumentList();
    
    /**
     * @brief Parses type annotations
     * @return String representing the parsed type
     * 
     * Parses type syntax including:
     * ```
     * int32           // primitive types
     * str             // string type
     * bool            // boolean type
     * MyType          // user-defined types
     * ```
     * 
     * The method validates that the type is recognized and properly formed.
     */
    std::string parseType();
    
    /**
     * @brief Parses pointer type annotations
     * @return String representing the parsed pointer type
     * 
     * Parses pointer type syntax:
     * ```
     * int32*          // pointer to int32
     * char*           // pointer to char
     * MyType*         // pointer to user-defined type
     * int32**         // pointer to pointer to int32
     * ```
     * 
     * Handles multiple levels of indirection and validates pointer syntax.
     */
    std::string parsePointerType();
    
    // ======================== ERROR HANDLING METHODS ========================
    
    /**
     * @brief Reports a parsing error with detailed context information
     * @param message Descriptive error message explaining what went wrong
     * @throws ParseError with comprehensive error information
     * 
     * This method creates detailed error reports that include:
     * - The specific error message
     * - Current token information (type, value, position)
     * - Source code context around the error
     * - Suggestions for fixing the error when possible
     * 
     * The error includes enough information for IDE integration and
     * helpful compiler diagnostics.
     */
    void error(const std::string& message);
    
    /**
     * @brief Performs error recovery by synchronizing to a known safe state
     * 
     * When a parsing error occurs, this method attempts to recover by:
     * - Advancing past the problematic tokens
     * - Finding statement boundaries (semicolons, braces)
     * - Resetting to a state where parsing can continue
     * - Allowing detection of multiple errors in a single parse run
     * 
     * This implements panic-mode error recovery, which is simple but
     * effective for providing multiple error reports to the user.
     */
    void synchronize();
    
public:
    /**
     * @brief Constructs a new Parser with the given token sequence
     * @param tokens Vector of tokens produced by the lexer
     * 
     * Initializes the parser with:
     * - The complete token sequence to be parsed
     * - Current position set to the beginning (index 0)
     * - Error state reset
     * 
     * The parser assumes the token sequence is complete and properly terminated
     * with an EOF_TOKEN.
     */
    explicit Parser(const std::vector<Token>& tokens);
    
    /**
     * @brief Parses the complete token sequence and returns the AST
     * @return Unique pointer to Program AST node representing the parsed code
     * @throws ParseError if parsing fails due to syntax errors
     * 
     * This is the main public interface for the parser. It:
     * - Parses the entire program from start to finish
     * - Returns a complete AST representation
     * - Handles and reports any syntax errors encountered
     * - Provides error recovery to find multiple errors when possible
     * 
     * The returned AST can be used by subsequent compilation phases
     * (semantic analysis, code generation, etc.).
     */
    std::unique_ptr<Program> parse();
    
    /**
     * @class ParseError
     * @brief Exception class for parsing errors with detailed context
     * 
     * This exception class provides comprehensive error information for
     * parsing failures, including:
     * - Human-readable error message
     * - Token where the error occurred
     * - Position information for error reporting
     * - Context for IDE integration and debugging
     * 
     * The class extends std::exception for standard error handling
     * while providing additional parser-specific information.
     */
    class ParseError : public std::exception {
    private:
        std::string message;    // Descriptive error message
        Token token;           // Token where error occurred
        
    public:
        /**
         * @brief Constructs a ParseError with message and token context
         * @param msg Human-readable error description
         * @param tok Token where the error was detected
         */
        ParseError(const std::string& msg, const Token& tok);
        
        /**
         * @brief Returns the error message for standard exception handling
         * @return C-string containing the error message
         */
        const char* what() const noexcept override;
        
        /**
         * @brief Returns the token where the error occurred
         * @return Reference to the problematic token
         * 
         * This allows error handlers to access detailed position information
         * and context about where the parsing failure occurred.
         */
        const Token& getToken() const;
    };
};

} // namespace emlang

#endif // EM_LANG_PARSER_H
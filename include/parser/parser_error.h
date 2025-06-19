//===--- parser_error.h - Parser Error Handling -----------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Parser error handling interface for EMLang
//===----------------------------------------------------------------------===//

#ifndef EMLANG_PARSER_ERROR_H
#define EMLANG_PARSER_ERROR_H

#pragma once

#include "lexer/token.h"
#include <string>
#include <vector>
#include <exception>
#include <emlang_export.h>

namespace emlang {

/**
 * @class ParseError
 * @brief Exception class for parser errors
 * 
 * ParseError represents a syntax error encountered during parsing. It provides
 * detailed information about the error location and context for better debugging
 * and user feedback.
 * 
 * **Error Information:**
 * - Error message describing what went wrong
 * - Token where the error occurred
 * - Line and column numbers for precise location
 * - Complete formatted error message
 * 
 * **Usage Example:**
 * @code
 * try {
 *     parser.parseExpression();
 * } catch (const ParseError& e) {
 *     std::cerr << "Parse failed: " << e.what() << std::endl;
 *     std::cerr << "Error at line " << e.getLine() << std::endl;
 * }
 * @endcode
 */
class EMLANG_API ParseError : public std::exception {
private:
    std::string message_;      // Error description
    std::string fullMessage_;  // Complete formatted error message
    Token token_;              // Token where error occurred
    size_t line_;              // Line number (1-based)
    size_t column_;            // Column number (1-based)

public:
    /**
     * @brief Construct ParseError with token context
     * @param message Error description
     * @param token Token where the error occurred
     */
    ParseError(const std::string& message, const Token& token);

    /**
     * @brief Construct ParseError with explicit location
     * @param message Error description
     * @param line Line number where error occurred
     * @param column Column number where error occurred
     */
    ParseError(const std::string& message, size_t line, size_t column);

    /**
     * @brief Get complete formatted error message
     * @return C-string containing the full error message
     */
    const char* what() const noexcept override;

    /**
     * @brief Get the basic error message without location info
     * @return Reference to the error message string
     */
    const std::string& getMessage() const;

    /**
     * @brief Get the token where the error occurred
     * @return Reference to the error token
     */
    const Token& getToken() const;

    /**
     * @brief Get the line number where the error occurred
     * @return Line number (1-based)
     */
    size_t getLine() const;

    /**
     * @brief Get the column number where the error occurred
     * @return Column number (1-based)
     */
    size_t getColumn() const;
};

/**
 * @class ErrorReporter
 * @brief Collects and manages parser errors during parsing
 * 
 * ErrorReporter provides centralized error management for the parser, allowing
 * it to continue parsing after errors and collect multiple errors in a single
 * pass. This improves the development experience by showing all syntax errors
 * at once rather than stopping at the first one.
 * 
 * **Features:**
 * - Collects multiple errors without stopping
 * - Provides error summaries and statistics
 * - Supports both errors and warnings
 * - Automatic error formatting and display
 * 
 * **Usage Example:**
 * @code
 * ErrorReporter reporter;
 * Parser parser(tokens, reporter);
 * parser.parse();
 * 
 * if (reporter.hasErrors()) {
 *     reporter.printAllErrors();
 *     return -1;
 * }
 * @endcode
 */
class EMLANG_API ErrorReporter {
private:
    std::vector<ParseError> errors_;  // Collected parse errors
    bool hasErrors_;                  // Quick error check flag

public:
    /**
     * @brief Construct an empty error reporter
     */
    ErrorReporter();

    /**
     * @brief Report a parse error with token context
     * @param message Error description
     * @param token Token where the error occurred
     */
    void reportError(const std::string& message, const Token& token);

    /**
     * @brief Report a parse error with explicit location
     * @param message Error description
     * @param line Line number where error occurred
     * @param column Column number where error occurred
     */
    void reportError(const std::string& message, size_t line, size_t column);

    /**
     * @brief Report a warning (non-fatal issue)
     * @param message Warning description
     * @param token Token where the warning occurred
     */
    void reportWarning(const std::string& message, const Token& token);

    /**
     * @brief Check if any errors have been reported
     * @return true if errors exist, false otherwise
     */
    bool hasErrors() const;

    /**
     * @brief Get all collected errors
     * @return Vector of ParseError objects
     */
    const std::vector<ParseError>& getErrors() const;

    /**
     * @brief Clear all collected errors
     */
    void clear();

    /**
     * @brief Get the total number of errors
     * @return Number of errors collected
     */
    size_t getErrorCount() const;

    /**
     * @brief Print all collected errors to standard output
     */
    void printAllErrors() const;
};

// ======================== Utility Functions ========================

/**
 * @brief Generate error message for expected vs actual token mismatch
 * @param expected The token type that was expected
 * @param actual The token that was actually found
 * @return Formatted error message
 */
EMLANG_API std::string getExpectedTokenMessage(TokenType expected, const Token& actual);

/**
 * @brief Generate error message for unexpected token
 * @param token The unexpected token
 * @return Formatted error message
 */
EMLANG_API std::string getUnexpectedTokenMessage(const Token& token);

/**
 * @brief Generate message for parser synchronization point
 * @return Formatted synchronization message
 */
EMLANG_API std::string getSynchronizationMessage();

} // namespace emlang

#endif // EMLANG_PARSER_ERROR_H

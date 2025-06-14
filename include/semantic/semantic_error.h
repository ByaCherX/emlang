//===--- semantic_error.h - Sem error ---------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Semantic Error Handling
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_SEMANTIC_ERROR_H
#define EM_LANG_SEMANTIC_ERROR_H

#include <emlang_export.h>
#include <string>
#include <stdexcept>
#include <vector>

namespace emlang {

/**
 * @class SemanticError
 * @brief Exception class for semantic analysis errors
 * 
 * SemanticError represents errors that occur during semantic analysis,
 * such as type mismatches, undefined symbols, and other semantic violations.
 */
class EMLANG_API SemanticError : public std::runtime_error {
private:
    size_t line;
    size_t column;
    
public:
    /**
     * @brief Constructs a SemanticError with message and location
     * @param message Error description
     * @param line Source line number (1-based)
     * @param column Source column number (1-based)
     */
    SemanticError(const std::string& message, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Gets the line number where the error occurred
     * @return Line number (1-based, 0 if unknown)
     */
    size_t getLine() const { return line; }
    
    /**
     * @brief Gets the column number where the error occurred
     * @return Column number (1-based, 0 if unknown)
     */
    size_t getColumn() const { return column; }
    
    /**
     * @brief Gets a formatted error message with location information
     * @return Formatted error message
     */
    std::string getFormattedMessage() const;
};

/**
 * @class SemanticErrorReporter
 * @brief Manages error and warning reporting during semantic analysis
 * 
 * The SemanticErrorReporter class provides centralized error and warning reporting
 * functionality for the semantic analyzer. It tracks error counts and
 * provides formatted output.
 */
class EMLANG_API SemanticErrorReporter {
private:
    std::vector<SemanticError> errors;
    std::vector<std::string> warnings;
    bool showWarnings;
    
public:    /**
     * @brief Constructs a new SemanticErrorReporter
     * @param showWarnings Whether to display warnings immediately
     */
    SemanticErrorReporter(bool showWarnings = true);
    
    /**
     * @brief Reports a semantic error
     * @param message Error description
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     */
    void reportError(const std::string& message, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Reports a semantic warning
     * @param message Warning description
     * @param line Source line number (0 for unknown)
     * @param column Source column number (0 for unknown)
     */
    void reportWarning(const std::string& message, size_t line = 0, size_t column = 0);
    
    /**
     * @brief Gets the number of errors reported
     * @return Error count
     */
    size_t getErrorCount() const;
    
    /**
     * @brief Gets the number of warnings reported
     * @return Warning count
     */
    size_t getWarningCount() const;
    
    /**
     * @brief Checks if any errors have been reported
     * @return true if errors were reported, false otherwise
     */
    bool hasErrors() const;
    
    /**
     * @brief Checks if any warnings have been reported
     * @return true if warnings were reported, false otherwise
     */
    bool hasWarnings() const;
    
    /**
     * @brief Resets the error and warning counts
     */
    void clear();
    
    /**
     * @brief Prints a summary of errors and warnings
     */
    void printSummary() const;
    
    /**
     * @brief Gets all errors
     * @return Reference to the errors vector
     */
    const std::vector<SemanticError>& getErrors() const;
    
    /**
     * @brief Gets all warnings
     * @return Reference to the warnings vector
     */
    const std::vector<std::string>& getWarnings() const;
};

} // namespace emlang

#endif // EM_LANG_SEMANTIC_ERROR_H

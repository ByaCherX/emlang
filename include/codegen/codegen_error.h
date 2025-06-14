//===--- codegen_error.h - Code Generation Error Handling -------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Error handling for EMLang code generation
//
// This file contains the error handling infrastructure for code generation,
// including error types, reporting mechanisms, and recovery strategies.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_ERROR_H
#define EM_CODEGEN_ERROR_H

#pragma once

#include <emlang_export.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace emlang {

/**
 * @enum CodegenErrorType
 * @brief Categorizes different types of code generation errors
 */
enum class CodegenErrorType {
    // Type-related errors
    UnknownType,
    TypeMismatch,
    InvalidPointerOperation,
    InvalidCast,

    // Symbol-related errors
    UndefinedVariable,
    UndefinedFunction,
    UndefinedSymbol,
    DuplicateSymbol,
    InvalidSymbolReference,

    // Control flow errors
    InvalidReturn,
    UnreachableCode,
    InvalidBranch,
    
    // Function-related errors
    ArgumentCountMismatch,
    ParameterTypeMismatch,
    InvalidFunctionCall,
    MissingMainFunction,
    
    // Memory-related errors
    InvalidMemoryAccess,
    NullPointerDereference,
    MemoryAllocationFailure,
    
    // LLVM backend errors
    LLVMGenerationError,
    LLVMVerificationError,
    OptimizationFailure,
    ObjectFileGenerationError,
    
    // General errors
    InternalError,
    NotImplemented
};

/**
 * @class CodegenError
 * @brief Represents a code generation error with detailed information
 * 
 * CodegenError encapsulates error information including type, message,
 * and context for proper error reporting and debugging.
 */
class EMLANG_API CodegenError : public std::runtime_error {
public:
    /**
     * @brief Constructs a code generation error
     * @param type Error type category
     * @param message Detailed error message
     * @param context Optional context information
     */
    CodegenError(CodegenErrorType type, const std::string& message, const std::string& context = "");

    /**
     * @brief Gets the error type
     * @return Error type category
     */
    CodegenErrorType getType() const { return type_; }    
    
    /**
     * @brief Gets the error context
     * @return Context information string
     */
    const std::string& getContext() const { return context_; }

    /**
     * @brief Gets a formatted error string
     * @return Formatted error message with type and context
     */
    std::string getFormattedMessage() const;    
    
    /**
     * @brief Converts error type to string representation
     * @param type Error type to convert
     * @return String representation of the error type
     */
    static std::string errorTypeToString(CodegenErrorType type);

private:
    CodegenErrorType type_;
    std::string context_;
};

/**
 * @class CodegenErrorReporter
 * @brief Manages error reporting and collection for code generation
 * 
 * CodegenErrorReporter provides centralized error management,
 * allowing accumulation of multiple errors and batch reporting.
 */
class EMLANG_API CodegenErrorReporter {
public:
    /**
     * @brief Default constructor
     */
    CodegenErrorReporter();

    /**
     * @brief Default destructor
     */
    ~CodegenErrorReporter() = default;

    // ======================== ERROR REPORTING ========================

    /**
     * @brief Reports an error with automatic context detection
     * @param message Error message
     */
    void error(const std::string& message);

    /**
     * @brief Reports a typed error with context
     * @param type Error type category
     * @param message Error message
     * @param context Optional context information
     */
    void error(CodegenErrorType type, const std::string& message, const std::string& context = "");

    /**
     * @brief Reports a warning
     * @param message Warning message
     */
    void warning(const std::string& message);

    /**
     * @brief Reports an informational message
     * @param message Info message
     */
    void info(const std::string& message);

    // ======================== ERROR MANAGEMENT ========================

    /**
     * @brief Checks if any errors have been reported
     * @return true if errors exist, false otherwise
     */
    bool hasErrors() const;

    /**
     * @brief Gets the number of reported errors
     * @return Number of errors
     */
    size_t getErrorCount() const;

    /**
     * @brief Gets the number of reported warnings
     * @return Number of warnings
     */
    size_t getWarningCount() const;

    /**
     * @brief Gets all reported errors
     * @return Vector of error objects
     */
    const std::vector<CodegenError>& getErrors() const;

    /**
     * @brief Clears all reported errors and warnings
     */
    void clearErrors();

    // ======================== OUTPUT CONTROL ========================

    /**
     * @brief Sets whether to print errors immediately
     * @param immediate true to print immediately, false to accumulate
     */
    void setImmediateOutput(bool immediate);

    /**
     * @brief Prints all accumulated errors to the specified stream
     * @param stream Output stream (default: std::cerr)
     */
    void printErrors(std::ostream& stream = std::cerr) const;

    /**
     * @brief Prints all accumulated warnings to the specified stream
     * @param stream Output stream (default: std::cerr)
     */
    void printWarnings(std::ostream& stream = std::cerr) const;

    /**
     * @brief Prints a summary of errors and warnings
     * @param stream Output stream (default: std::cerr)
     */
    void printSummary(std::ostream& stream = std::cerr) const;

    // ======================== CONTEXT MANAGEMENT ========================

    /**
     * @brief Sets the current code generation context
     * @param context Context description (e.g., function name, file location)
     */
    void setContext(const std::string& context);

    /**
     * @brief Gets the current context
     * @return Current context string
     */
    const std::string& getContext() const;

    /**
     * @brief Pushes a new context onto the context stack
     * @param context New context to push
     */
    void pushContext(const std::string& context);

    /**
     * @brief Pops the most recent context from the context stack
     */
    void popContext();

private:
    // ======================== INTERNAL STATE ========================

    std::vector<CodegenError> errors_;
    std::vector<std::string> warnings_;
    std::vector<std::string> contextStack_;
    bool immediateOutput_;

    // ======================== HELPER METHODS ========================

    /**
     * @brief Converts error type to string
     * @param type Error type
     * @return String representation of error type
     */
    std::string errorTypeToString(CodegenErrorType type) const;

    /**
     * @brief Gets current context string from context stack
     * @return Combined context string
     */
    std::string getCurrentContextString() const;
};

// ======================== UTILITY FUNCTIONS ========================

/**
 * @brief Creates a type mismatch error message
 * @param expected Expected type
 * @param actual Actual type
 * @return Formatted error message
 */
EMLANG_API std::string makeTypeMismatchError(const std::string& expected, const std::string& actual);

/**
 * @brief Creates an undefined symbol error message
 * @param symbolName Symbol name
 * @param symbolType Symbol type (variable, function, etc.)
 * @return Formatted error message
 */
EMLANG_API std::string makeUndefinedSymbolError(const std::string& symbolName, const std::string& symbolType);

/**
 * @brief Creates an argument count mismatch error message
 * @param functionName Function name
 * @param expected Expected argument count
 * @param actual Actual argument count
 * @return Formatted error message
 */
EMLANG_API std::string makeArgumentCountError(const std::string& functionName, size_t expected, size_t actual);

} // namespace emlang

#endif // EM_CODEGEN_ERROR_H

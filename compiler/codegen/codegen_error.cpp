//===--- codegen_error.cpp - Code Generation Error Handling ----*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of error handling for EMLang code generation
//===----------------------------------------------------------------------===//

#include "codegen/codegen_error.h"
#include <iostream>
#include <sstream>
#include <map>

namespace emlang {

// ======================== CodegenError Implementation ========================

CodegenError::CodegenError(CodegenErrorType type, const std::string& message, const std::string& context)
    : std::runtime_error(message), type_(type), context_(context) {}

std::string CodegenError::getFormattedMessage() const {
    std::ostringstream oss;
    
    // Add error type prefix
    oss << "[" << CodegenError::errorTypeToString(type_) << "] ";
    
    // Add main message
    oss << what();
    
    // Add context if available
    if (!context_.empty()) {
        oss << " (Context: " << context_ << ")";
    }
    
    return oss.str();
}

std::string CodegenError::errorTypeToString(CodegenErrorType type) {
    switch (type) {
        case CodegenErrorType::UnknownType:
            return "UNKNOWN TYPE";
        case CodegenErrorType::TypeMismatch:
            return "TYPE MISMATCH";
        case CodegenErrorType::InvalidPointerOperation:
            return "INVALID POINTER OPERATION";
        case CodegenErrorType::InvalidCast:
            return "INVALID CAST";
        case CodegenErrorType::UndefinedVariable:
            return "UNDEFINED VARIABLE";
        case CodegenErrorType::UndefinedFunction:
            return "UNDEFINED FUNCTION";
        case CodegenErrorType::DuplicateSymbol:
            return "DUPLICATE SYMBOL";
        case CodegenErrorType::InvalidSymbolReference:
            return "INVALID SYMBOL REFERENCE";
        case CodegenErrorType::InvalidReturn:
            return "INVALID RETURN";
        case CodegenErrorType::UnreachableCode:
            return "UNREACHABLE CODE";
        case CodegenErrorType::InvalidBranch:
            return "INVALID BRANCH";
        case CodegenErrorType::ArgumentCountMismatch:
            return "ARGUMENT COUNT MISMATCH";
        case CodegenErrorType::ParameterTypeMismatch:
            return "PARAMETER TYPE MISMATCH";
        case CodegenErrorType::InvalidFunctionCall:
            return "INVALID FUNCTION CALL";
        case CodegenErrorType::MissingMainFunction:
            return "MISSING MAIN FUNCTION";
        case CodegenErrorType::InvalidMemoryAccess:
            return "INVALID MEMORY ACCESS";
        case CodegenErrorType::NullPointerDereference:
            return "NULL POINTER DEREFERENCE";
        case CodegenErrorType::MemoryAllocationFailure:
            return "MEMORY ALLOCATION FAILURE";
        case CodegenErrorType::LLVMGenerationError:
            return "LLVM GENERATION ERROR";
        case CodegenErrorType::LLVMVerificationError:
            return "LLVM VERIFICATION ERROR";
        case CodegenErrorType::OptimizationFailure:
            return "OPTIMIZATION FAILURE";
        case CodegenErrorType::ObjectFileGenerationError:
            return "OBJECT FILE GENERATION ERROR";
        case CodegenErrorType::InternalError:
            return "INTERNAL ERROR";
        case CodegenErrorType::NotImplemented:
            return "NOT IMPLEMENTED";
        default:
            return "UNKNOWN ERROR";
    }
}

// ======================== CodegenErrorReporter Implementation ========================

CodegenErrorReporter::CodegenErrorReporter() : immediateOutput_(false) {}

void CodegenErrorReporter::error(const std::string& message) {
    error(CodegenErrorType::InternalError, message);
}

void CodegenErrorReporter::error(CodegenErrorType type, const std::string& message, const std::string& context) {
    std::string fullContext = context.empty() ? getCurrentContextString() : context;
    CodegenError error(type, message, fullContext);
    errors_.push_back(error);
    
    if (immediateOutput_) {
        std::cerr << error.getFormattedMessage() << std::endl;
    }
}

void CodegenErrorReporter::warning(const std::string& message) {
    warnings_.push_back(message);
    
    if (immediateOutput_) {
        std::cerr << "[WARNING] " << message << std::endl;
    }
}

void CodegenErrorReporter::info(const std::string& message) {
    if (immediateOutput_) {
        std::cerr << "[INFO] " << message << std::endl;
    }
}

bool CodegenErrorReporter::hasErrors() const {
    return !errors_.empty();
}

size_t CodegenErrorReporter::getErrorCount() const {
    return errors_.size();
}

size_t CodegenErrorReporter::getWarningCount() const {
    return warnings_.size();
}

const std::vector<CodegenError>& CodegenErrorReporter::getErrors() const {
    return errors_;
}

void CodegenErrorReporter::clearErrors() {
    errors_.clear();
    warnings_.clear();
}

void CodegenErrorReporter::setImmediateOutput(bool immediate) {
    immediateOutput_ = immediate;
}

void CodegenErrorReporter::printErrors(std::ostream& stream) const {
    stream << "=== Code Generation Errors ===" << std::endl;
    for (const auto& error : errors_) {
        stream << error.getFormattedMessage() << std::endl;
    }
}

void CodegenErrorReporter::printWarnings(std::ostream& stream) const {
    stream << "=== Code Generation Warnings ===" << std::endl;
    for (const auto& warning : warnings_) {
        stream << "[WARNING] " << warning << std::endl;
    }
}

void CodegenErrorReporter::printSummary(std::ostream& stream) const {
    if (!hasErrors() && warnings_.empty()) {
        stream << "No code generation errors or warnings." << std::endl;
        return;
    }
    
    stream << "======================================" << std::endl;
    stream << "    Code Generation Summary          " << std::endl;
    stream << "======================================" << std::endl;
    stream << "Errors: " << errors_.size() << std::endl;
    stream << "Warnings: " << warnings_.size() << std::endl;
    
    if (!errors_.empty()) {
        // Count errors by type
        std::map<CodegenErrorType, int> errorCounts;
        for (const auto& error : errors_) {
            errorCounts[error.getType()]++;
        }
        
        stream << "Error breakdown:" << std::endl;
        for (const auto& pair : errorCounts) {
            stream << "  " << errorTypeToString(pair.first) << ": " << pair.second << std::endl;
        }
        
        stream << "======================================" << std::endl;
        printErrors(stream);
    }
    
    if (!warnings_.empty()) {
        stream << "======================================" << std::endl;
        printWarnings(stream);
    }
    
    stream << "======================================" << std::endl;
}

void CodegenErrorReporter::setContext(const std::string& context) {
    contextStack_.clear();
    contextStack_.push_back(context);
}

const std::string& CodegenErrorReporter::getContext() const {
    static const std::string empty;
    return contextStack_.empty() ? empty : contextStack_.back();
}

void CodegenErrorReporter::pushContext(const std::string& context) {
    contextStack_.push_back(context);
}

void CodegenErrorReporter::popContext() {
    if (!contextStack_.empty()) {
        contextStack_.pop_back();
    }
}

std::string CodegenErrorReporter::errorTypeToString(CodegenErrorType type) const {
    return CodegenError::errorTypeToString(type);
}

std::string CodegenErrorReporter::getCurrentContextString() const {
    if (contextStack_.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < contextStack_.size(); ++i) {
        if (i > 0) oss << " -> ";
        oss << contextStack_[i];
    }
    return oss.str();
}

// ======================== Utility Functions ========================

std::string makeTypeMismatchError(const std::string& expected, const std::string& actual) {
    return "Expected type '" + expected + "', but got '" + actual + "'";
}

std::string makeUndefinedSymbolError(const std::string& symbolName, const std::string& symbolType) {
    return "Undefined " + symbolType + " '" + symbolName + "'";
}

std::string makeArgumentCountError(const std::string& functionName, size_t expected, size_t actual) {
    return "Function '" + functionName + "' expects " + std::to_string(expected) + 
           " arguments, but got " + std::to_string(actual);
}

} // namespace emlang

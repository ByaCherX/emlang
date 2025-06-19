//===--- parser_error.cpp - Parser Error Handling ------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Parser error handling implementation for EMLang
//===----------------------------------------------------------------------===//

#include "parser/parser_error.h"
#include <sstream>
#include <iostream>

namespace emlang {

// ======================== ParseError Implementation ========================

ParseError::ParseError(const std::string& message, const Token& token)
    : message_(message), token_(token), line_(token.line), column_(token.column) {
    // Build complete error message with location information
    std::ostringstream oss;
    oss << "Parse error at line " << line_ << ", column " << column_ 
        << ": " << message_;
    if (!token_.value.empty()) {
        oss << " (near '" << token_.value << "')";
    }
    fullMessage_ = oss.str();
}

ParseError::ParseError(const std::string& message, size_t line, size_t column)
    : message_(message), token_(Token(TokenType::INVALID, "", line, column)), 
      line_(line), column_(column) {
    std::ostringstream oss;
    oss << "Parse error at line " << line_ << ", column " << column_ 
        << ": " << message_;
    fullMessage_ = oss.str();
}

const char* ParseError::what() const noexcept {
    return fullMessage_.c_str();
}

const std::string& ParseError::getMessage() const {
    return message_;
}

const Token& ParseError::getToken() const {
    return token_;
}

size_t ParseError::getLine() const {
    return line_;
}

size_t ParseError::getColumn() const {
    return column_;
}

// ======================== ErrorReporter Implementation ========================

ErrorReporter::ErrorReporter() : hasErrors_(false) {}

void ErrorReporter::reportError(const std::string& message, const Token& token) {
    hasErrors_ = true;
    ParseError error(message, token);
    errors_.push_back(error);
    
    // Print error immediately for debugging
    std::cerr << "ERROR: " << error.what() << std::endl;
}

void ErrorReporter::reportError(const std::string& message, size_t line, size_t column) {
    hasErrors_ = true;
    ParseError error(message, line, column);
    errors_.push_back(error);
    
    // Print error immediately for debugging
    std::cerr << "ERROR: " << error.what() << std::endl;
}

void ErrorReporter::reportWarning(const std::string& message, const Token& token) {
    std::cerr << "WARNING: " << message << " at line " << token.line 
              << ", column " << token.column;
    if (!token.value.empty()) {
        std::cerr << " (near '" << token.value << "')";
    }
    std::cerr << std::endl;
}

bool ErrorReporter::hasErrors() const {
    return hasErrors_;
}

const std::vector<ParseError>& ErrorReporter::getErrors() const {
    return errors_;
}

void ErrorReporter::clear() {
    hasErrors_ = false;
    errors_.clear();
}

size_t ErrorReporter::getErrorCount() const {
    return errors_.size();
}

void ErrorReporter::printAllErrors() const {
    if (errors_.empty()) {
        std::cout << "No parsing errors found." << std::endl;
        return;
    }
    
    std::cout << "Found " << errors_.size() << " parsing error(s):" << std::endl;
    for (const auto& error : errors_) {
        std::cout << "  " << error.what() << std::endl;
    }
}

// ======================== Utility Functions ========================

std::string getExpectedTokenMessage(TokenType expected, const Token& actual) {
    std::ostringstream oss;
    oss << "Expected '" << Token::tokenTypeToString(expected) 
        << "' but found '" << Token::tokenTypeToString(actual.type) << "'";
    return oss.str();
}

std::string getUnexpectedTokenMessage(const Token& token) {
    std::ostringstream oss;
    oss << "Unexpected token '" << Token::tokenTypeToString(token.type) << "'";
    if (!token.value.empty()) {
        oss << " ('" << token.value << "')";
    }
    return oss.str();
}

std::string getSynchronizationMessage() {
    return "Parser recovered and continued from this point";
}

} // namespace emlang

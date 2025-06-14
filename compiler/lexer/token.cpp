//===--- token.cpp - Token implementation ---------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Token class implementation for the EMLang lexer
//
// This file contains the implementation of the Token class and related
// utilities that were moved from lexer.cpp for better code organization.
//===----------------------------------------------------------------------===//

#include "lexer/token.h"
#include <iostream>

namespace emlang {

// Token constructor
Token::Token(TokenType type, const std::string& value, size_t line, size_t column)
    : type(type), value(value), line(line), column(column) {}

std::string Token::toString() const {
    return tokenTypeToString(type) + "(" + value + ") at " + std::to_string(line) + ":" + std::to_string(column);
}

std::string Token::tokenTypeToString(TokenType type) {    
    auto it = emlang::tokenNames.find(type);
    return (it != emlang::tokenNames.end()) ? it->second : "UNKNOWN";
}

} // namespace emlang

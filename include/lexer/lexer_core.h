//===--- lexer_core.h - Core lexer interface --------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Core lexer interface for EMLang
//
// This file contains the main Lexer class that tokenizes source code.
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_LEXER_CORE_H
#define EM_LANG_LEXER_CORE_H

#pragma once

#include "emlang_export.h"
#include "token.h"
#include <vector>
#include <memory>

namespace emlang {

/**
 * @class Lexer
 * @brief Lexical analyzer that converts source code text into tokens
 */
class EMLANG_API Lexer {
private:
    std::string source;    // The complete source code string being analyzed
    size_t position;       // Current character position in the source (0-based)
    size_t line;           // Current line number in the source (1-based)
    size_t column;         // Current column number in the source (1-based)
    char currentChar;      // The character currently being examined
    
    /**
     * @brief Advances to the next character in the source code
     */
    void advance();
    
    /**
     * @brief Skips over whitespace characters (spaces, tabs, etc.)
     */
    void skipWhitespace();
    
    /**
     * @brief Skips over comment text until the end of the comment
     */
    void skipComment();
    
    /**
     * @brief Reads and returns a complete numeric literal
     */
    std::string readNumber();
    
    /**
     * @brief Reads and returns a complete string literal
     */
    std::string readString();
    
    /**
     * @brief Reads and returns a character literal
     */
    std::string readCharLiteral();
    
    /**
     * @brief Reads and returns a complete identifier or keyword
     */
    std::string readIdentifier();
    
    /**
     * @brief Determines if an identifier is a reserved keyword
     */
    TokenType getKeywordType(const std::string& identifier);
    
public:
    /**
     * @brief Constructs a new Lexer object with the given source code
     */
    explicit Lexer(const std::string& source);
    
    /**
     * @brief Scans and returns the next token from the source code
     */
    Token nextToken();
    
    /**
     * @brief Tokenizes the entire source code and returns all tokens
     */
    std::vector<Token> tokenize();
    
    /**
     * @brief Reports a lexical error with the given message
     */
    void error(const std::string& message);
};

} // namespace emlang

#endif // EM_LANG_LEXER_CORE_H

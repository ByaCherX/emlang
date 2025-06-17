//===--- token.h - Token interface ------------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Token interface for the EMLang lexer
//
// This file contains the Token class and TokenType enumeration used by
// the lexer for representing lexical units.
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_TOKEN_H
#define EM_LANG_TOKEN_H

#pragma once

#include <emlang_export.h>
#include <string>
#include <map>

namespace emlang {

/**
 * @enum TokenType
 * @brief Enumeration of all possible token types in the EMLang language
 */
enum class TokenType {
    // Special (0x00-0x0F)
    EOF_TOKEN =             0,     // End of file token
    NEWLINE =               1,     // Newline character token
    WHITESPACE =            2,     // Whitespace token (spaces, tabs)
    COMMENT =               3,     // Comment token (single-line or multi-line)
    BLOCK_COMMENT =         4,     // Block comment token (/* ... */)
    DOC_COMMENT =           5,     // Documentation comment token (/** ... */)

    // Literals (0x10-0x1F)
    INT =                   16,    // Integer literal
    FLOAT =                 17,    // Floating point literal
    CHAR =                  18,    // Character literal
    STR =                   19,    // String literal
    BOOL =                  20,    // Boolean literal (true/false)
    ARRAY =                 21,    // Array literal
    NULL_LITERAL =          22,    // Null literal

    // Keywords (0x20-0x4F)
    IDENTIFIER =            32,    // Identifier (variable, function, etc.)
    LET =                   33,    // let variable declaration
    CONST =                 34,    // const constant declaration
    FUNCTION =              35,    // function declaration
    EXTERN =                36,    // extern function declaration
    IF =                    37,    // if conditional statement
    ELSE =                  38,    // else conditional statement
    WHILE =                 39,    // while loop statement
    FOR =                   40,    // for loop statement
    RETURN =                41,    // return statement

    // Operators (0x50-0x6F)
    PLUS =                  80,    // + addition operator
    MINUS =                 81,    // - subtraction operator
    MULTIPLY =              82,    // * multiplication operator
    DIVIDE =                83,    // / division operator
    MODULO =                84,    // % modulo operator
    ASSIGN =                85,    // = assignment operator
    EQUAL =                 86,    // == equality operator
    NOT_EQUAL =             87,    // != inequality operator
    LESS_THAN =             88,    // < relational operator
    GREATER_THAN =          89,    // > relational operator
    LESS_EQUAL =            90,    // <= relational operator
    GREATER_EQUAL =         91,    // >= relational operator
    LEFT_SHIFT =            92,    // << left shift operator
    RIGHT_SHIFT =           93,    // >> right shift operator
    LOGICAL_AND =           94,    // && logical conjunction
    LOGICAL_OR =            95,    // || logical disjunction
    LOGICAL_NOT =           96,    // ! logical negation
    BITWISE_AND =           97,    // & bitwise AND operator
    BITWISE_OR =            98,    // | bitwise OR operator
    BITWISE_XOR =           99,    // ^ bitwise XOR operator
    BITWISE_INVERT =        100,   // ~ bitwise INVERSION operator
#ifdef EMLANG_ENABLE_POINTERS
    AMPERSAND =             85,    // & address-of operator
#endif
    
    // Delimiters (0x70-0x8F)
    SEMICOLON =             112,   // ; statement terminator
    COMMA =                 113,   // , argument separator
    DOT =                   114,   // . member access operator
    COLON =                 115,   // : type annotation or label
    LEFT_PAREN =            116,   // ( left parenthesis
    RIGHT_PAREN =           117,   // ) right parenthesis
    LEFT_BRACE =            118,   // { left brace
    RIGHT_BRACE =           119,   // } right brace
    LEFT_BRACKET =          120,   // [ left bracket
    RIGHT_BRACKET =         121,   // ] right bracket
    
    // Reserved (0x8F-0xFE)
    RESERVED_START =        0x8F,   // Start of reserved range
    RESERVED_END =          0xFE,   // End of reserved range
    
    // INVALID Token for special cases
    INVALID =               0xFF,   // Used for lexical errors and unknown characters
};

static const std::map<TokenType, std::string> tokenNames = {
    /// Special
    {TokenType::EOF_TOKEN, "EOF"},
    {TokenType::NEWLINE, "NEWLINE"},
    {TokenType::WHITESPACE, "WHITESPACE"},
    {TokenType::COMMENT, "COMMENT"},
    {TokenType::BLOCK_COMMENT, "BLOCK_COMMENT"},
    {TokenType::DOC_COMMENT, "DOC_COMMENT"},

    /// Literals
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::CHAR, "CHAR"},
    {TokenType::STR, "STR"},
    {TokenType::BOOL, "BOOL"},
    {TokenType::ARRAY, "ARRAY"},
    {TokenType::NULL_LITERAL, "NULL"},

    /// Keywords
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::LET, "LET"},
    {TokenType::CONST, "CONST"},
    {TokenType::FUNCTION, "FUNCTION"},
    {TokenType::EXTERN, "EXTERN"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::FOR, "FOR"},
    {TokenType::RETURN, "RETURN"},
    
    /// Operators
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::MULTIPLY, "MULTIPLY"},
    {TokenType::DIVIDE, "DIVIDE"},
    {TokenType::MODULO, "MODULO"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::EQUAL, "EQUAL"},
    {TokenType::NOT_EQUAL, "NOT_EQUAL"},
    {TokenType::LESS_THAN, "LESS_THAN"},
    {TokenType::GREATER_THAN, "GREATER_THAN"},
    {TokenType::LESS_EQUAL, "LESS_EQUAL"},
    {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
    {TokenType::LEFT_SHIFT, "LEFT_SHIFT"},
    {TokenType::RIGHT_SHIFT, "RIGHT_SHIFT"},
    {TokenType::LOGICAL_AND, "LOGICAL_AND"},
    {TokenType::LOGICAL_OR, "LOGICAL_OR"},
    {TokenType::LOGICAL_NOT, "LOGICAL_NOT"},
    {TokenType::BITWISE_AND, "BITWISE_AND"},
    {TokenType::BITWISE_OR, "BITWISE_OR"},
    {TokenType::BITWISE_XOR, "BITWISE_XOR"},
    {TokenType::BITWISE_IVERT, "BITWISE_IVERT"},
#ifdef EMLANG_ENABLE_POINTERS
    {TokenType::AMPERSAND, "AMPERSAND"},
#endif

    /// Delimiters
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::DOT, "DOT"},
    {TokenType::COLON, "COLON"},
    {TokenType::LEFT_PAREN, "LEFT_PAREN"},
    {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
    {TokenType::LEFT_BRACE, "LEFT_BRACE"},
    {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
    {TokenType::LEFT_BRACKET, "LEFT_BRACKET"},
    {TokenType::RIGHT_BRACKET, "RIGHT_BRACKET"},

    {TokenType::INVALID, "INVALID"}
    
};

static const std::map<std::string, TokenType> keywords = {
    {"let", TokenType::LET},
    {"const", TokenType::CONST},
    {"function", TokenType::FUNCTION},
    {"extern", TokenType::EXTERN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"return", TokenType::RETURN},
    
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT},
    {"char", TokenType::CHAR},
    {"str", TokenType::STR},
    {"bool", TokenType::BOOL},
    {"array", TokenType::ARRAY},
    {"null", TokenType::NULL_LITERAL},
};

/**
 * @class Token
 * @brief Represents a single lexical unit (token) in the source code
 */
class EMLANG_API Token {
public:
    TokenType type;        // The category/type of this token
    std::string value;     // The actual text content from source code
    size_t line;           // Line number where this token appears (1-based)
    size_t column;         // Column number where this token starts (1-based)
    
    /**
     * @brief Constructs a new Token object
     */
    Token(TokenType type, const std::string& value, size_t line, size_t column);
    
    /**
     * @brief Destructor
     */
    ~Token() = default;
    
    /**
     * @brief Converts the token to a human-readable string representation
     */
    std::string toString() const;
    
    /**
     * @brief Converts a TokenType enum value to its string representation
     */
    static std::string tokenTypeToString(TokenType type);
};

} // namespace emlang

#endif // EM_LANG_TOKEN_H

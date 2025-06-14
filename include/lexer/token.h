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
    // Reserved (0x00-0x0F)
    RESERVED_BEGIN =        0x00,    // Reserved for future use
    RESERVED_END =          0x0F,    // Reserved for future use

    // Literals (0x10-0x1F)
    NUMBER =                16,    // Numeric literal
    STRING =                17,    // String literal
    CHAR_LITERAL =          18,    // Character literal
    IDENTIFIER =            19,    // Identifier (variable, function, etc.)
    
    // Keywords (0x20-0x3F)
    LET =                   32,    // let variable declaration
    CONST =                 33,    // const constant declaration
    FUNCTION =              34,    // function declaration
    EXTERN =                35,    // extern function declaration
    IF =                    36,    // if conditional statement
    ELSE =                  37,    // else conditional statement
    WHILE =                 38,    // while loop statement
    FOR =                   39,    // for loop statement
    RETURN =                40,    // return statement
    TRUE =                  41,    // true boolean literal
    FALSE =                 42,    // false boolean literal
    NULL_TOKEN =            43,    // null literal

    // C-style primitive type keywords (0x40-0x5F)
    INT8 =                  64,    // int8 signed 8-bit integer
    INT16 =                 65,    // int16 signed 16-bit integer  
    INT32 =                 66,    // int32 signed 32-bit integer
    INT64 =                 67,    // int64 signed 64-bit integer
    ISIZE =                 68,    // isize pointer-sized signed integer
    UINT8 =                 69,    // uint8 unsigned 8-bit integer
    UINT16 =                70,    // uint16 unsigned 16-bit integer
    UINT32 =                71,    // uint32 unsigned 32-bit integer
    UINT64 =                72,    // uint64 unsigned 64-bit integer
    USIZE =                 73,    // usize pointer-sized unsigned integer
    FLOAT =                 74,    // float 32-bit floating point
    DOUBLE =                75,    // double 64-bit floating point
    CHAR =                  76,    // char Unicode scalar value
    STR =                   77,    // str string slice
    BOOL =                  78,    // bool boolean type

    // Operators (0x60-0x7F)
    PLUS =                  96,    // + addition operator
    MINUS =                 97,    // - subtraction operator
    MULTIPLY =              98,    // * multiplication operator
    DIVIDE =                99,    // / division operator
    MODULO =                100,   // % modulo operator
    ASSIGN =                101,   // = assignment operator
    EQUAL =                 102,   // == equality operator
    NOT_EQUAL =             103,   // != inequality operator
    LESS_THAN =             104,   // < relational operator
    GREATER_THAN =          105,   // > relational operator
    LESS_EQUAL =            106,   // <= relational operator
    GREATER_EQUAL =         107,   // >= relational operator
    LOGICAL_AND =           108,   // && logical conjunction
    LOGICAL_OR =            109,   // || logical disjunction
    LOGICAL_NOT =           110,   // ! logical negation
    AMPERSAND =             111,   // & address-of operator
    
    // Delimiters (0x80-0x8F)
    SEMICOLON =             128,   // ; statement terminator
    COMMA =                 129,   // , argument separator
    DOT =                   130,   // . member access operator
    COLON =                 131,   // : type annotation or label
    
    // Brackets (0x90-0x9F)
    LEFT_PAREN =            144,   // ( left parenthesis
    RIGHT_PAREN =           145,   // ) right parenthesis
    LEFT_BRACE =            146,   // { left brace
    RIGHT_BRACE =           147,   // } right brace
    LEFT_BRACKET =          148,   // [ left bracket
    RIGHT_BRACKET =         149,   // ] right bracket
    
    // Special (0xA0-0xAF)
    NEWLINE =               160,    // Newline character
    EOF_TOKEN =             161,    // End of file token    
    
    // INVALID Token for special cases
    INVALID =               0xFF,   // Used for lexical errors and unknown characters
};

static const std::map<TokenType, std::string> tokenNames = {
    {TokenType::NUMBER, "NUMBER"},
    {TokenType::STRING, "STRING"},
    {TokenType::CHAR_LITERAL, "CHAR_LITERAL"},
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
    {TokenType::TRUE, "TRUE"},
    {TokenType::FALSE, "FALSE"},
    {TokenType::NULL_TOKEN, "NULL"},

    // C-style primitive type tokens
    {TokenType::INT8, "INT8"},
    {TokenType::INT16, "INT16"},
    {TokenType::INT32, "INT32"},
    {TokenType::INT64, "INT64"},
    {TokenType::ISIZE, "ISIZE"},
    {TokenType::UINT8, "UINT8"},
    {TokenType::UINT16, "UINT16"},
    {TokenType::UINT32, "UINT32"},
    {TokenType::UINT64, "UINT64"},
    {TokenType::USIZE, "USIZE"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::DOUBLE, "DOUBLE"},
    {TokenType::CHAR, "CHAR"},
    {TokenType::STR, "STR"},
    {TokenType::BOOL, "BOOL"},
    
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
    {TokenType::LOGICAL_AND, "LOGICAL_AND"},
    {TokenType::LOGICAL_OR, "LOGICAL_OR"},
    {TokenType::LOGICAL_NOT, "LOGICAL_NOT"},
    {TokenType::AMPERSAND, "AMPERSAND"},
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
    {TokenType::NEWLINE, "NEWLINE"},
    {TokenType::EOF_TOKEN, "EOF"},
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
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_TOKEN},
    // C-style primitive type keywords
    {"int8", TokenType::INT8},
    {"int16", TokenType::INT16},
    {"int32", TokenType::INT32},
    {"int64", TokenType::INT64},
    {"isize", TokenType::ISIZE},
    {"uint8", TokenType::UINT8},
    {"uint16", TokenType::UINT16},
    {"uint32", TokenType::UINT32},
    {"uint64", TokenType::UINT64},
    {"usize", TokenType::USIZE},
    {"float", TokenType::FLOAT},
    {"double", TokenType::DOUBLE},
    {"char", TokenType::CHAR},
    {"str", TokenType::STR},
    {"bool", TokenType::BOOL}
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

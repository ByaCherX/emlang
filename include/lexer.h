//===--- lexer.h - Token interface ------------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # Lexical analyzer for the EMLang programming language
//
// This file contains the lexer implementation for EMLang, which is responsible for
// converting raw source code text into a stream of tokens. The lexer performs the
// first phase of compilation by breaking down the input text into meaningful units
// that can be processed by the parser.
// 
// The lexer operates as a finite state machine that reads characters one by one
// and groups them into tokens based on predefined patterns and rules.
//
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_LEXER_H
#define EM_LANG_LEXER_H

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

#include <string>
#include <vector>
#include <memory>


namespace emlang {

/**
 * @enum TokenType
 * @brief Enumeration of all possible token types in the EMLang language
 * 
 * This enum defines all the different types of tokens that the lexer can recognize.
 * Each token type is assigned a unique hexadecimal value for efficient processing
 * and debugging. The values are organized into logical groups:
 * 
 * - 0x00-0x0F: Reserved for future extensions
 * - 0x10-0x1F: Literal values (numbers, strings, identifiers)
 * - 0x20-0x3F: Language keywords (if, while, function, etc.)
 * - 0x40-0x5F: Primitive type keywords (int8, float, bool, etc.)
 * - 0x60-0x7F: Operators (arithmetic, logical, comparison)
 * - 0x80-0x8F: Delimiters (semicolon, comma, dot, colon)
 * - 0x90-0x9F: Brackets and parentheses
 * - 0xA0-0xAF: Special tokens (newline, EOF)
 * - 0xFF: Invalid/error token
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
    MINUS =                 97,    // - addition operator
    MULTIPLY =              98,    // * multiplication operator
    DIVIDE =                99,    // / division operator
    MODULO =                100,   // % modulo operator
    ASSIGN =                101,   // = assignment operator
    EQUAL =                 102,   // == equality operator
    NOT_EQUAL =             103,   // !=, == equality operators
    LESS_THAN =             104,   // < relational operators
    GREATER_THAN =          105,   // > relational operators
    LESS_EQUAL =            106,   // <= relational operators
    GREATER_EQUAL =         107,   // >= relational operators
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

/**
 * @class Token
 * @brief Represents a single lexical unit (token) in the source code
 * 
 * A Token is the basic unit of lexical analysis. It encapsulates:
 * - The type of token (keyword, operator, literal, etc.)
 * - The actual text value from the source code
 * - Position information for error reporting and debugging
 * 
 * Tokens are created by the lexer and consumed by the parser during
 * the compilation process. Each token maintains its original position
 * in the source code to enable accurate error reporting.
 */
class EMLANG_API Token {
public:
    TokenType type;        // The category/type of this token
    std::string value;     // The actual text content from source code
    size_t line;           // Line number where this token appears (1-based)
    size_t column;         // Column number where this token starts (1-based)
    
    /**
     * @brief Constructs a new Token object
     * @param type The TokenType classification for this token
     * @param value The string value extracted from source code
     * @param line The line number in source code (1-based indexing)
     * @param column The column number in source code (1-based indexing)
     */
    Token(TokenType type, const std::string& value, size_t line, size_t column);
    
    /**
     * @brief Converts the token to a human-readable string representation
     * @return String containing token type, value, and position information
     * 
     * This method is primarily used for debugging and error reporting.
     * Example output: "IDENTIFIER 'myVariable' at line 5, column 10"
     */
    std::string toString() const;
    
    /**
     * @brief Converts a TokenType enum value to its string representation
     * @param type The TokenType to convert
     * @return String name of the token type (e.g., "NUMBER", "IF", "PLUS")
     * 
     * This static method provides a way to get human-readable names for
     * token types, useful for debugging and error messages.
     */
    static std::string tokenTypeToString(TokenType type);
};

/**
 * @class Lexer
 * @brief Lexical analyzer that converts source code text into tokens
 * 
 * The Lexer class implements a scanner/tokenizer for the EMLang programming language.
 * It reads through source code character by character and groups them into meaningful
 * tokens that can be processed by the parser.
 * 
 * Key responsibilities:
 * - Character-by-character source code analysis
 * - Recognition of keywords, operators, literals, and identifiers
 * - Handling of whitespace, comments, and special characters
 * - Position tracking for error reporting
 * - Error detection and reporting for invalid character sequences
 * 
 * The lexer operates as a deterministic finite automaton (DFA) where each
 * character determines the next state and potential token recognition.
 * 
 * Usage example:
 * @code
 * Lexer lexer("let x = 42;");
 * std::vector<Token> tokens = lexer.tokenize();
 * // Results in tokens: LET, IDENTIFIER("x"), ASSIGN, NUMBER("42"), SEMICOLON, EOF_TOKEN
 * @endcode
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
     * 
     * This method moves the lexer forward by one character, updating the
     * position, line, and column counters appropriately. It handles:
     * - Normal character advancement
     * - Line break detection and line number incrementing
     * - End-of-file detection
     * - Column position tracking
     */
    void advance();
    
    /**
     * @brief Skips over whitespace characters (spaces, tabs, etc.)
     * 
     * Whitespace characters are generally not significant in EMLang syntax
     * (except for line breaks in some contexts). This method advances through
     * consecutive whitespace characters until a non-whitespace character is found.
     * 
     * Handles: spaces, tabs, carriage returns (but not newlines, which are
     * sometimes syntactically significant)
     */
    void skipWhitespace();
    
    /**
     * @brief Skips over comment text until the end of the comment
     * 
     * Comments are non-executable text that should be ignored during tokenization.
     * This method handles different comment styles:
     * - Single-line comments (// to end of line)
     * - Multi-line comments (/* to *\/)
     * - Documentation comments if supported
     * 
     * The method advances the lexer position past the entire comment.
     */
    void skipComment();
    
    /**
     * @brief Reads and returns a complete numeric literal
     * @return String containing the full numeric value
     * 
     * This method recognizes and extracts numeric literals from the source code.
     * It handles various numeric formats:
     * - Integer literals (42, 1000)
     * - Floating-point literals (3.14, 2.0)
     * - Scientific notation (1e10, 3.14e-2)
     * - Different bases (0x1F for hex, 0b1010 for binary, 0o77 for octal)
     * 
     * The method continues reading characters as long as they form a valid
     * numeric sequence according to EMLang's numeric literal rules.
     */
    std::string readNumber();
    
    /**
     * @brief Reads and returns a complete string literal
     * @return String containing the string content (without quotes)
     * 
     * This method extracts string literals enclosed in double quotes.
     * It handles:
     * - Escape sequences (\n, \t, \", \\, etc.)
     * - Unicode escape sequences (\u1234)
     * - Raw strings if supported
     * - Multi-line strings if supported
     * 
     * The method validates that the string is properly terminated and
     * processes escape sequences to their actual character values.
     */
    std::string readString();
    
    /**
     * @brief Reads and returns a character literal
     * @return String containing the character content (without single quotes)
     * 
     * Character literals are single characters enclosed in single quotes.
     * This method handles:
     * - Regular characters ('a', '7', ' ')
     * - Escape sequences ('\n', '\t', '\'', '\\')
     * - Unicode characters ('\u0041')
     * 
     * Character literals must contain exactly one character (after escape
     * sequence processing) and be properly terminated with a closing quote.
     */
    std::string readCharLiteral();
    
    /**
     * @brief Reads and returns a complete identifier or keyword
     * @return String containing the identifier text
     * 
     * Identifiers are names used for variables, functions, types, etc.
     * They follow specific rules:
     * - Must start with a letter or underscore
     * - Can contain letters, digits, and underscores
     * - Case-sensitive
     * 
     * This method reads characters that form a valid identifier and
     * returns the complete name. The caller is responsible for determining
     * if the identifier is actually a reserved keyword.
     */
    std::string readIdentifier();
    
    /**
     * @brief Determines if an identifier is a reserved keyword
     * @param identifier The identifier string to check
     * @return TokenType corresponding to the keyword, or IDENTIFIER if not a keyword
     * 
     * This method looks up the given identifier in the language's keyword table.
     * Keywords are reserved words that have special meaning in the language
     * (like 'if', 'while', 'function', 'let', etc.).
     * 
     * If the identifier matches a keyword, the appropriate TokenType is returned.
     * Otherwise, IDENTIFIER is returned to indicate it's a user-defined name.
     */
    TokenType getKeywordType(const std::string& identifier);
    
public:
    /**
     * @brief Constructs a new Lexer object with the given source code
     * @param source The complete source code string to tokenize
     * 
     * Initializes the lexer with the source code and sets up the initial state:
     * - Position at the beginning of the source
     * - Line and column numbers at 1
     * - Current character set to the first character (or EOF if empty)
     */
    explicit Lexer(const std::string& source);
    
    /**
     * @brief Scans and returns the next token from the source code
     * @return Token object representing the next lexical unit
     * 
     * This is the core method of the lexer. It analyzes the current position
     * in the source code and determines what type of token starts at that position.
     * 
     * The method:
     * 1. Skips any whitespace and comments
     * 2. Examines the current character to determine token type
     * 3. Calls appropriate helper methods to read the complete token
     * 4. Creates and returns a Token object with type, value, and position
     * 
     * If called repeatedly, it will return all tokens in the source code
     * sequence, ending with an EOF_TOKEN.
     */
    Token nextToken();
    
    /**
     * @brief Tokenizes the entire source code and returns all tokens
     * @return Vector containing all tokens found in the source code
     * 
     * This convenience method calls nextToken() repeatedly until the entire
     * source code has been processed. It returns a vector containing all
     * tokens in order, including the final EOF_TOKEN.
     * 
     * This method is useful when you want to tokenize everything at once
     * rather than processing tokens one at a time.
     */
    std::vector<Token> tokenize();
    
    /**
     * @brief Reports a lexical error with the given message
     * @param message Descriptive error message
     * 
     * This method is called when the lexer encounters invalid input that
     * cannot be tokenized according to the language rules. It provides
     * detailed error information including:
     * - The error message
     * - Current position (line and column)
     * - Context around the error location
     * 
     * The method may throw an exception or set an error flag depending
     * on the error handling strategy used by the compiler.
     */
    void error(const std::string& message);
};

} // namespace emlang

#endif // EM_LANG_LEXER_H
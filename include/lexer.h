#pragma once

#include <string>
#include <vector>
#include <memory>

namespace emlang {

// Token türleri
enum class TokenType {    
    // Literals
    NUMBER,
    STRING,
    CHAR_LITERAL,
    IDENTIFIER,
    
    // Keywords
    LET,
    CONST,
    FUNCTION,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    TRUE,
    FALSE,
    NULL_TOKEN,

    // C-style primitive type keywords
    INT8,      // int8 signed 8-bit integer
    INT16,     // int16 signed 16-bit integer  
    INT32,     // int32 signed 32-bit integer
    INT64,     // int64 signed 64-bit integer
    ISIZE,     // isize pointer-sized signed integer
    UINT8,     // uint8 unsigned 8-bit integer
    UINT16,    // uint16 unsigned 16-bit integer
    UINT32,    // uint32 unsigned 32-bit integer
    UINT64,    // uint64 unsigned 64-bit integer
    USIZE,     // usize pointer-sized unsigned integer
    FLOAT,     // float 32-bit floating point
    DOUBLE,    // double 64-bit floating point
    CHAR,      // char Unicode scalar value
    STR,       // str string slice
    BOOL,      // bool boolean type

    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_NOT,
    AMPERSAND,     // & address-of operator
    
    // Delimiters
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    
    // Brackets
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    
    // Special
    NEWLINE,
    EOF_TOKEN,
    INVALID
};

// Token sınıfı
class Token {
public:
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    Token(TokenType type, const std::string& value, size_t line, size_t column);
    
    std::string toString() const;
    static std::string tokenTypeToString(TokenType type);
};

// Lexer sınıfı
class Lexer {
private:
    std::string source;
    size_t position;
    size_t line;
    size_t column;
    char currentChar;
    
    void advance();
    void skipWhitespace();
    void skipComment();    std::string readNumber();
    std::string readString();
    std::string readCharLiteral();
    std::string readIdentifier();
    TokenType getKeywordType(const std::string& identifier);
    
public:
    explicit Lexer(const std::string& source);
    
    Token nextToken();
    std::vector<Token> tokenize();
    
    // Error handling
    void error(const std::string& message);
};

} // namespace emlang

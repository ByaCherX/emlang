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
    void skipComment();
    std::string readNumber();
    std::string readString();
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

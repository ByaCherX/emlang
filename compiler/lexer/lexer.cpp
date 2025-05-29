#include "../include/lexer.h"
#include <iostream>
#include <cctype>
#include <map>
#include <stdexcept>

namespace emlang {

// Token constructor
Token::Token(TokenType type, const std::string& value, size_t line, size_t column)
    : type(type), value(value), line(line), column(column) {}

std::string Token::toString() const {
    return tokenTypeToString(type) + "(" + value + ") at " + std::to_string(line) + ":" + std::to_string(column);
}

std::string Token::tokenTypeToString(TokenType type) {
    static const std::map<TokenType, std::string> tokenNames = {
        {TokenType::NUMBER, "NUMBER"},
        {TokenType::STRING, "STRING"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::LET, "LET"},
        {TokenType::CONST, "CONST"},
        {TokenType::FUNCTION, "FUNCTION"},
        {TokenType::IF, "IF"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::FOR, "FOR"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::NULL_TOKEN, "NULL"},
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
    
    auto it = tokenNames.find(type);
    return (it != tokenNames.end()) ? it->second : "UNKNOWN";
}

// Lexer constructor
Lexer::Lexer(const std::string& source) 
    : source(source), position(0), line(1), column(1) {
    currentChar = position < source.length() ? source[position] : '\0';
}

void Lexer::advance() {
    if (currentChar == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    
    position++;
    currentChar = position < source.length() ? source[position] : '\0';
}

void Lexer::skipWhitespace() {
    while (currentChar != '\0' && std::isspace(currentChar) && currentChar != '\n') {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar == '/' && position + 1 < source.length() && source[position + 1] == '/') {
        // Single line comment
        while (currentChar != '\0' && currentChar != '\n') {
            advance();
        }
    } else if (currentChar == '/' && position + 1 < source.length() && source[position + 1] == '*') {
        // Multi-line comment
        advance(); // skip '/'
        advance(); // skip '*'
        
        while (currentChar != '\0') {
            if (currentChar == '*' && position + 1 < source.length() && source[position + 1] == '/') {
                advance(); // skip '*'
                advance(); // skip '/'
                break;
            }
            advance();
        }
    }
}

std::string Lexer::readNumber() {
    std::string number;
    bool hasDot = false;
    
    while (currentChar != '\0' && (std::isdigit(currentChar) || (currentChar == '.' && !hasDot))) {
        if (currentChar == '.') {
            hasDot = true;
        }
        number += currentChar;
        advance();
    }
    
    return number;
}

std::string Lexer::readString() {
    std::string str;
    char quote = currentChar;
    advance(); // skip opening quote
    
    while (currentChar != '\0' && currentChar != quote) {
        if (currentChar == '\\') {
            advance();
            switch (currentChar) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                case '\'': str += '\''; break;
                default: str += currentChar; break;
            }
        } else {
            str += currentChar;
        }
        advance();
    }
    
    if (currentChar == quote) {
        advance(); // skip closing quote
    } else {
        error("Unterminated string literal");
    }
    
    return str;
}

std::string Lexer::readIdentifier() {
    std::string identifier;
    
    while (currentChar != '\0' && (std::isalnum(currentChar) || currentChar == '_')) {
        identifier += currentChar;
        advance();
    }
    
    return identifier;
}

TokenType Lexer::getKeywordType(const std::string& identifier) {
    static const std::map<std::string, TokenType> keywords = {
        {"let", TokenType::LET},
        {"const", TokenType::CONST},
        {"function", TokenType::FUNCTION},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"return", TokenType::RETURN},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"null", TokenType::NULL_TOKEN}
    };
    
    auto it = keywords.find(identifier);
    return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
}

Token Lexer::nextToken() {
    while (currentChar != '\0') {
        size_t tokenLine = line;
        size_t tokenColumn = column;
        
        // Skip whitespace
        if (std::isspace(currentChar) && currentChar != '\n') {
            skipWhitespace();
            continue;
        }
        
        // Handle newlines
        if (currentChar == '\n') {
            advance();
            return Token(TokenType::NEWLINE, "\\n", tokenLine, tokenColumn);
        }
        
        // Handle comments
        if (currentChar == '/' && position + 1 < source.length() && 
            (source[position + 1] == '/' || source[position + 1] == '*')) {
            skipComment();
            continue;
        }
        
        // Numbers
        if (std::isdigit(currentChar)) {
            std::string number = readNumber();
            return Token(TokenType::NUMBER, number, tokenLine, tokenColumn);
        }
        
        // Strings
        if (currentChar == '"' || currentChar == '\'') {
            std::string str = readString();
            return Token(TokenType::STRING, str, tokenLine, tokenColumn);
        }
        
        // Identifiers and keywords
        if (std::isalpha(currentChar) || currentChar == '_') {
            std::string identifier = readIdentifier();
            TokenType type = getKeywordType(identifier);
            return Token(type, identifier, tokenLine, tokenColumn);
        }
        
        // Two-character operators
        if (position + 1 < source.length()) {
            std::string twoChar = std::string(1, currentChar) + source[position + 1];
            
            if (twoChar == "==") {
                advance(); advance();
                return Token(TokenType::EQUAL, "==", tokenLine, tokenColumn);
            } else if (twoChar == "!=") {
                advance(); advance();
                return Token(TokenType::NOT_EQUAL, "!=", tokenLine, tokenColumn);
            } else if (twoChar == "<=") {
                advance(); advance();
                return Token(TokenType::LESS_EQUAL, "<=", tokenLine, tokenColumn);
            } else if (twoChar == ">=") {
                advance(); advance();
                return Token(TokenType::GREATER_EQUAL, ">=", tokenLine, tokenColumn);
            } else if (twoChar == "&&") {
                advance(); advance();
                return Token(TokenType::LOGICAL_AND, "&&", tokenLine, tokenColumn);
            } else if (twoChar == "||") {
                advance(); advance();
                return Token(TokenType::LOGICAL_OR, "||", tokenLine, tokenColumn);
            }
        }
          // Single-character tokens
        switch (currentChar) {
            case '+': advance(); return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
            case '-': advance(); return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
            case '*': advance(); return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
            case '/': 
                // Check if it's a comment first
                if (position + 1 < source.length() && 
                    (source[position + 1] == '/' || source[position + 1] == '*')) {
                    skipComment();
                    continue;
                } else {
                    advance(); 
                    return Token(TokenType::DIVIDE, "/", tokenLine, tokenColumn);
                }
            case '%': advance(); return Token(TokenType::MODULO, "%", tokenLine, tokenColumn);
            case '=': advance(); return Token(TokenType::ASSIGN, "=", tokenLine, tokenColumn);
            case '<': advance(); return Token(TokenType::LESS_THAN, "<", tokenLine, tokenColumn);
            case '>': advance(); return Token(TokenType::GREATER_THAN, ">", tokenLine, tokenColumn);
            case '!': advance(); return Token(TokenType::LOGICAL_NOT, "!", tokenLine, tokenColumn);
            case ';': advance(); return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
            case ',': advance(); return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);
            case '.': advance(); return Token(TokenType::DOT, ".", tokenLine, tokenColumn);
            case ':': advance(); return Token(TokenType::COLON, ":", tokenLine, tokenColumn);
            case '(': advance(); return Token(TokenType::LEFT_PAREN, "(", tokenLine, tokenColumn);
            case ')': advance(); return Token(TokenType::RIGHT_PAREN, ")", tokenLine, tokenColumn);
            case '{': advance(); return Token(TokenType::LEFT_BRACE, "{", tokenLine, tokenColumn);
            case '}': advance(); return Token(TokenType::RIGHT_BRACE, "}", tokenLine, tokenColumn);
            case '[': advance(); return Token(TokenType::LEFT_BRACKET, "[", tokenLine, tokenColumn);
            case ']': advance(); return Token(TokenType::RIGHT_BRACKET, "]", tokenLine, tokenColumn);
            
            default:
                error("Unexpected character: " + std::string(1, currentChar));
                advance();
                return Token(TokenType::INVALID, std::string(1, currentChar), tokenLine, tokenColumn);
        }
    }
    
    return Token(TokenType::EOF_TOKEN, "", line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    Token token = nextToken();
    while (token.type != TokenType::EOF_TOKEN) {
        if (token.type != TokenType::NEWLINE) { // Skip newlines for now
            tokens.push_back(token);
        }
        token = nextToken();
    }
    tokens.push_back(token); // Add EOF token
    
    return tokens;
}

void Lexer::error(const std::string& message) {
    std::cerr << "Lexer Error at " << line << ":" << column << " - " << message << std::endl;
    throw std::runtime_error("Lexer error: " + message);
}

} // namespace emlang

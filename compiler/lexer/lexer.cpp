#include "lexer/lexer_core.h"
#include "lexer/token.h"
#include <iostream>
#include <cctype>
#include <map>
#include <stdexcept>

namespace emlang {

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
    advance(); // skip opening double quote
    
    while (currentChar != '\0' && currentChar != '"') {
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
    
    if (currentChar == '"') {
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
    auto it = emlang::keywords.find(identifier);
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
          // Strings (double quotes)
        if (currentChar == '"') {
            std::string str = readString();
            return Token(TokenType::STRING, str, tokenLine, tokenColumn);
        }
        
        // Character literals (single quotes)
        if (currentChar == '\'') {
            std::string charLit = readCharLiteral();
            return Token(TokenType::CHAR_LITERAL, charLit, tokenLine, tokenColumn);
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
            case '>': advance(); return Token(TokenType::GREATER_THAN, ">", tokenLine, tokenColumn);            case '!': advance(); return Token(TokenType::LOGICAL_NOT, "!", tokenLine, tokenColumn);
            case '&': advance(); return Token(TokenType::AMPERSAND, "&", tokenLine, tokenColumn);
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

std::string Lexer::readCharLiteral() {
    std::string charStr;
    advance(); // skip opening single quote
    
    if (currentChar == '\0') {
        error("Unterminated character literal");
        return "";
    }
    
    if (currentChar == '\'') {
        error("Empty character literal");
        return "";
    }
      if (currentChar == '\\') {
        advance();
        switch (currentChar) {
            case 'n': charStr = "\\n"; break;
            case 't': charStr = "\\t"; break;
            case 'r': charStr = "\\r"; break;
            case '\\': charStr = "\\\\"; break;
            case '\'': charStr = "\\'"; break;
            case '"': charStr = "\\\""; break;
            case '0': charStr = "\\0"; break;
            case 'u': {
                // Unicode escape sequence \u{XXXX}
                advance();
                if (currentChar != '{') {
                    error("Expected '{' after \\u in Unicode escape");
                    return "";
                }
                advance();
                
                std::string hexCode;
                while (currentChar != '\0' && currentChar != '}' && hexCode.length() < 8) {
                    if (std::isxdigit(currentChar)) {
                        hexCode += currentChar;
                        advance();
                    } else {
                        error("Invalid hex digit in Unicode escape");
                        return "";
                    }
                }
                
                if (currentChar != '}') {
                    error("Unterminated Unicode escape sequence");
                    return "";
                }
                
                if (hexCode.empty()) {
                    error("Empty Unicode escape sequence");
                    return "";
                }
                
                // Convert hex to Unicode scalar value
                try {
                    unsigned long codepoint = std::stoul(hexCode, nullptr, 16);
                    if (codepoint > 0x10FFFF) {
                        error("Unicode codepoint out of range");
                        return "";
                    }
                    // For now, just store the hex value as string
                    charStr = "\\u{" + hexCode + "}";
                } catch (const std::exception& e) {
                    error("Invalid Unicode escape sequence");
                    return "";
                }
                break;
            }
            default:
                error("Invalid escape sequence in character literal");
                return "";
        }
        advance();
    } else {
        // Regular character
        charStr = currentChar;
        advance();
    }
    
    if (currentChar == '\'') {
        advance(); // skip closing quote
    } else {
        error("Unterminated character literal");
        return "";
    }
    
    return charStr;
}

} // namespace emlang

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
    str.reserve(64); // Reserve space for performance
    advance(); // skip opening double quote
    
    while (currentChar != '\0' && currentChar != '"') {
        if (currentChar == '\\') {
            advance();
            if (currentChar == '\0') {
                error("Unterminated escape sequence in string literal");
                break;
            }
            
            switch (currentChar) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                case '\'': str += '\''; break;
                case '0': str += '\0'; break;
                case 'u': {
                    // Enhanced Unicode support
                    advance();
                    if (currentChar != '{') {
                        error("Expected '{' after \\u in Unicode escape");
                        break;
                    }
                    advance();
                    
                    std::string hexCode;
                    hexCode.reserve(8);
                    
                    while (currentChar != '\0' && currentChar != '}' && hexCode.length() < 8) {
                        if (std::isxdigit(currentChar)) {
                            hexCode += currentChar;
                            advance();
                        } else {
                            error("Invalid hex digit in Unicode escape");
                            break;
                        }
                    }
                    
                    if (currentChar == '}' && !hexCode.empty()) {
                        try {
                            unsigned long codepoint = std::stoul(hexCode, nullptr, 16);
                            if (codepoint <= 0x10FFFF) {
                                // Convert to UTF-8
                                if (codepoint <= 0x7F) {
                                    str += static_cast<char>(codepoint);
                                } else if (codepoint <= 0x7FF) {
                                    str += static_cast<char>(0xC0 | (codepoint >> 6));
                                    str += static_cast<char>(0x80 | (codepoint & 0x3F));
                                } else if (codepoint <= 0xFFFF) {
                                    str += static_cast<char>(0xE0 | (codepoint >> 12));
                                    str += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                                    str += static_cast<char>(0x80 | (codepoint & 0x3F));
                                } else {
                                    str += static_cast<char>(0xF0 | (codepoint >> 18));
                                    str += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                                    str += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                                    str += static_cast<char>(0x80 | (codepoint & 0x3F));
                                }
                            } else {
                                error("Unicode codepoint out of range");
                            }
                        } catch (const std::exception&) {
                            error("Invalid Unicode escape sequence");
                        }
                    } else {
                        error("Malformed Unicode escape sequence");
                    }
                    break;
                }
                default: 
                    // Preserve unknown escape sequences
                    str += '\\';
                    str += currentChar; 
                    break;
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
        
        // Handle comments (single-line, multi-line, and doc comments)
        if (currentChar == '/' && position + 1 < source.length()) {
            char nextChar = source[position + 1];
            if (nextChar == '/') {
                skipComment();
                continue;
            } else if (nextChar == '*') {
                // Check for doc comment (/**)
                if (position + 2 < source.length() && source[position + 2] == '*') {
                    skipComment();
                    continue;
                } else {
                    skipComment();
                    continue;
                }
            }
        }
        
        // Numbers
        if (std::isdigit(currentChar)) {
            std::string number = readNumber();
            // Determine if it's integer or float based on content
            TokenType numberType = (number.find('.') != std::string::npos) ? TokenType::FLOAT : TokenType::INT;
            return Token(numberType, number, tokenLine, tokenColumn);
        }
        
        // String literals (double quotes) with Unicode support
        if (currentChar == '"') {
            std::string str = readString();
            return Token(TokenType::STR, str, tokenLine, tokenColumn);
        }
        
        // Character literals (single quotes) with Unicode support
        if (currentChar == '\'') {
            std::string charLit = readCharLiteral();
            return Token(TokenType::CHAR, charLit, tokenLine, tokenColumn);
        }
        
        // Identifiers and keywords
        if (std::isalpha(currentChar) || currentChar == '_') {
            std::string identifier = readIdentifier();
            
            // Special handling for boolean literals
            if (identifier == "true" || identifier == "false") {
                return Token(TokenType::BOOL, identifier, tokenLine, tokenColumn);
            }
            
            // Special handling for null literal
            if (identifier == "null") {
                return Token(TokenType::NULL_LITERAL, identifier, tokenLine, tokenColumn);
            }
            
            TokenType type = getKeywordType(identifier);
            return Token(type, identifier, tokenLine, tokenColumn);
        }
        
        // Multi-character operators
        if (position + 1 < source.length()) {
            std::string twoChar = std::string(1, currentChar) + source[position + 1];
            
            // Comparison operators
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
            }
            // Logical operators
            else if (twoChar == "&&") {
                advance(); advance();
                return Token(TokenType::LOGICAL_AND, "&&", tokenLine, tokenColumn);
            } else if (twoChar == "||") {
                advance(); advance();
                return Token(TokenType::LOGICAL_OR, "||", tokenLine, tokenColumn);
            }
            // Bitwise shift operators
            else if (twoChar == "<<") {
                advance(); advance();
                return Token(TokenType::LEFT_SHIFT, "<<", tokenLine, tokenColumn);
            } else if (twoChar == ">>") {
                advance(); advance();
                return Token(TokenType::RIGHT_SHIFT, ">>", tokenLine, tokenColumn);
            }
        }
        
        // Single-character tokens
        switch (currentChar) {
            // Arithmetic operators
            case '+': advance(); return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
            case '-': advance(); return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
            case '*': advance(); return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
            case '/': advance(); return Token(TokenType::DIVIDE, "/", tokenLine, tokenColumn);
            case '%': advance(); return Token(TokenType::MODULO, "%", tokenLine, tokenColumn);
            
            // Assignment and comparison
            case '=': advance(); return Token(TokenType::ASSIGN, "=", tokenLine, tokenColumn);
            case '<': advance(); return Token(TokenType::LESS_THAN, "<", tokenLine, tokenColumn);
            case '>': advance(); return Token(TokenType::GREATER_THAN, ">", tokenLine, tokenColumn);
            
            // Logical and bitwise operators
            case '!': advance(); return Token(TokenType::LOGICAL_NOT, "!", tokenLine, tokenColumn);
            case '&': advance(); return Token(TokenType::BITWISE_AND, "&", tokenLine, tokenColumn);
            case '|': advance(); return Token(TokenType::BITWISE_OR, "|", tokenLine, tokenColumn);
            case '^': advance(); return Token(TokenType::BITWISE_XOR, "^", tokenLine, tokenColumn);
            case '~': advance(); return Token(TokenType::BITWISE_INVERT, "~", tokenLine, tokenColumn);
            
            // Delimiters and punctuation
            case ';': advance(); return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
            case ',': advance(); return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);
            case '.': advance(); return Token(TokenType::DOT, ".", tokenLine, tokenColumn);
            case ':': advance(); return Token(TokenType::COLON, ":", tokenLine, tokenColumn);
            
            // Brackets and parentheses
            case '(': advance(); return Token(TokenType::LEFT_PAREN, "(", tokenLine, tokenColumn);
            case ')': advance(); return Token(TokenType::RIGHT_PAREN, ")", tokenLine, tokenColumn);
            case '{': advance(); return Token(TokenType::LEFT_BRACE, "{", tokenLine, tokenColumn);
            case '}': advance(); return Token(TokenType::RIGHT_BRACE, "}", tokenLine, tokenColumn);
            case '[': advance(); return Token(TokenType::LEFT_BRACKET, "[", tokenLine, tokenColumn);
            case ']': advance(); return Token(TokenType::RIGHT_BRACKET, "]", tokenLine, tokenColumn);
            
            // Invalid character handling
            default:
                std::string invalidChar(1, currentChar);
                advance(); // Skip the invalid character for recovery
                error("Unexpected character: '" + invalidChar + "' (0x" + 
                      std::to_string(static_cast<unsigned char>(invalidChar[0])) + ")");
                return Token(TokenType::INVALID, invalidChar, tokenLine, tokenColumn);
        }
    }
    
    // End of file token
    return Token(TokenType::EOF_TOKEN, "", line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(source.length() / 6); // Rough estimate for performance
    
    try {
        Token token = nextToken();
        while (token.type != TokenType::EOF_TOKEN) {
            // we might want to keep certain tokens
            // that are typically filtered out in traditional compilation
            if (token.type != TokenType::WHITESPACE) { // Keep newlines
                tokens.push_back(std::move(token));
            }
            token = nextToken();
        }
        tokens.push_back(std::move(token)); // Add EOF token
        
    } catch (const std::exception& e) {
        // continue tokenization if possible
        std::cerr << "Tokenization warning: " << e.what() << std::endl;
        std::cerr << "Attempting to continue tokenization..." << std::endl;
        
        // Skip problematic character and continue
        if (currentChar != '\0') {
            advance();
            // Recursively continue tokenization
            auto remainingTokens = tokenize();
            tokens.insert(tokens.end(), remainingTokens.begin(), remainingTokens.end());
        }
    }
    
    // Optimization: shrink to fit for memory efficiency in JIT
    tokens.shrink_to_fit();
    
    return tokens;
}

void Lexer::error(const std::string& message) {
    // Enhanced error reporting
    std::string context = "";
    
    // Extract context around the error position
    size_t contextStart = position >= 20 ? position - 20 : 0;
    size_t contextEnd = position + 20 < source.length() ? position + 20 : source.length();
    
    if (contextStart < contextEnd) {
        context = source.substr(contextStart, contextEnd - contextStart);
        // Replace control characters for better display
        for (char& c : context) {
            if (c == '\n') c = '↵';
            else if (c == '\t') c = '→';
        }
    }
    
    std::cerr << "Lexer Error at " << line << ":" << column << " - " << message << std::endl;
    if (!context.empty()) {
        std::cerr << "Context: ..." << context << "..." << std::endl;
        std::cerr << "         ";
        for (size_t i = 0; i < (position - contextStart) + 3; ++i) {
            std::cerr << " ";
        }
        std::cerr << "^" << std::endl;
    }
    
    // we want to continue processing when possible
    // instead of throwing immediately
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

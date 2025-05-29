#pragma once

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

namespace emlang {

// Parser sınıfı
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    // Token kontrol metodları
    Token& currentToken();
    Token& peekToken(size_t offset = 1);
    bool isAtEnd();
    bool check(TokenType type);
    bool match(TokenType type);
    bool match(std::vector<TokenType> types);
    Token advance();
    Token consume(TokenType type, const std::string& message);
    
    // Parsing metodları
    std::unique_ptr<Program> parseProgram();
    StatementPtr parseStatement();
    StatementPtr parseVariableDeclaration();
    StatementPtr parseFunctionDeclaration();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseBlockStatement();
    StatementPtr parseExpressionStatement();
    
    // Expression parsing (operator precedence)
    ExpressionPtr parseExpression();
    ExpressionPtr parseLogicalOr();
    ExpressionPtr parseLogicalAnd();
    ExpressionPtr parseEquality();
    ExpressionPtr parseComparison();
    ExpressionPtr parseTerm();
    ExpressionPtr parseFactor();
    ExpressionPtr parseUnary();
    ExpressionPtr parseCall();
    ExpressionPtr parsePrimary();
    
    // Helper metodları
    std::vector<FunctionDeclaration::Parameter> parseParameterList();
    std::vector<ExpressionPtr> parseArgumentList();
    
    // Error handling
    void error(const std::string& message);
    void synchronize();
    
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    std::unique_ptr<Program> parse();
    
    // Error reporting
    class ParseError : public std::exception {
    private:
        std::string message;
        Token token;
        
    public:
        ParseError(const std::string& msg, const Token& tok);
        const char* what() const noexcept override;
        const Token& getToken() const;
    };
};

} // namespace emlang

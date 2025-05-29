#include "../include/parser.h"
#include <iostream>
#include <stdexcept>

namespace emlang {

// ParseError implementation
Parser::ParseError::ParseError(const std::string& msg, const Token& tok)
    : message(msg), token(tok) {}

const char* Parser::ParseError::what() const noexcept {
    return message.c_str();
}

const Token& Parser::ParseError::getToken() const {
    return token;
}

// Parser constructor
Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

Token& Parser::currentToken() {
    return tokens[current];
}

Token& Parser::peekToken(size_t offset) {
    size_t index = current + offset;
    if (index >= tokens.size()) {
        return tokens.back(); // Return EOF token
    }
    return tokens[index];
}

bool Parser::isAtEnd() {
    return currentToken().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return currentToken().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::vector<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    
    error(message);
    throw ParseError(message, currentToken());
}

std::unique_ptr<Program> Parser::parse() {
    try {
        return parseProgram();
    } catch (const ParseError& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<StatementPtr> statements;
    
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    return std::make_unique<Program>(std::move(statements));
}

StatementPtr Parser::parseStatement() {
    try {
        if (match(TokenType::LET) || match(TokenType::CONST)) {
            current--; // Back up to re-read the token
            return parseVariableDeclaration();
        }
        
        if (match(TokenType::FUNCTION)) {
            current--; // Back up to re-read the token
            return parseFunctionDeclaration();
        }
        
        if (match(TokenType::IF)) {
            current--; // Back up to re-read the token
            return parseIfStatement();
        }
        
        if (match(TokenType::WHILE)) {
            current--; // Back up to re-read the token
            return parseWhileStatement();
        }
        
        if (match(TokenType::FOR)) {
            current--; // Back up to re-read the token
            return parseForStatement();
        }
        
        if (match(TokenType::RETURN)) {
            current--; // Back up to re-read the token
            return parseReturnStatement();
        }
        
        if (match(TokenType::LEFT_BRACE)) {
            current--; // Back up to re-read the token
            return parseBlockStatement();
        }
        
        return parseExpressionStatement();
        
    } catch (const ParseError& e) {
        synchronize();
        throw;
    }
}

StatementPtr Parser::parseVariableDeclaration() {
    bool isConst = match(TokenType::CONST);
    if (!isConst) {
        consume(TokenType::LET, "Expected 'let' or 'const'");
    }
    
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    
    std::string type;
    if (match(TokenType::COLON)) {
        Token typeToken = consume(TokenType::IDENTIFIER, "Expected type name");
        type = typeToken.value;
    }
    
    ExpressionPtr initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    
    return std::make_unique<VariableDeclaration>(name.value, type, std::move(initializer), isConst, name.line, name.column);
}

StatementPtr Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNCTION, "Expected 'function'");
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    auto parameters = parseParameterList();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    
    std::string returnType;
    if (match(TokenType::COLON)) {
        Token retType = consume(TokenType::IDENTIFIER, "Expected return type");
        returnType = retType.value;
    }
    
    auto body = parseBlockStatement();
    
    return std::make_unique<FunctionDeclaration>(name.value, std::move(parameters), returnType, std::move(body), name.line, name.column);
}

StatementPtr Parser::parseIfStatement() {
    Token ifToken = consume(TokenType::IF, "Expected 'if'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto thenBranch = parseStatement();
    
    StatementPtr elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement();
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch), ifToken.line, ifToken.column);
}

StatementPtr Parser::parseWhileStatement() {
    Token whileToken = consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = parseStatement();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body), whileToken.line, whileToken.column);
}

StatementPtr Parser::parseForStatement() {
    // For now, just consume the token and return a placeholder
    // TODO: Implement for loop parsing
    consume(TokenType::FOR, "Expected 'for'");
    error("For loops not yet implemented");
    return nullptr;
}

StatementPtr Parser::parseReturnStatement() {
    Token returnToken = consume(TokenType::RETURN, "Expected 'return'");
    
    ExpressionPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return value");
    
    return std::make_unique<ReturnStatement>(std::move(value), returnToken.line, returnToken.column);
}

StatementPtr Parser::parseBlockStatement() {
    Token leftBrace = consume(TokenType::LEFT_BRACE, "Expected '{'");
    
    std::vector<StatementPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
    
    return std::make_unique<BlockStatement>(std::move(statements), leftBrace.line, leftBrace.column);
}

StatementPtr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    
    return std::make_unique<ExpressionStatement>(std::move(expr), expr->line, expr->column);
}

ExpressionPtr Parser::parseExpression() {
    return parseLogicalOr();
}

ExpressionPtr Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(TokenType::LOGICAL_OR)) {
        Token op = tokens[current - 1];
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::LOGICAL_AND)) {
        Token op = tokens[current - 1];
        auto right = parseEquality();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({TokenType::NOT_EQUAL, TokenType::EQUAL})) {
        Token op = tokens[current - 1];
        auto right = parseComparison();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({TokenType::GREATER_THAN, TokenType::GREATER_EQUAL, TokenType::LESS_THAN, TokenType::LESS_EQUAL})) {
        Token op = tokens[current - 1];
        auto right = parseTerm();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = tokens[current - 1];
        auto right = parseFactor();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({TokenType::DIVIDE, TokenType::MULTIPLY, TokenType::MODULO})) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        expr = std::make_unique<BinaryOpExpression>(std::move(expr), op.value, std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseUnary() {
    if (match({TokenType::LOGICAL_NOT, TokenType::MINUS})) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        return std::make_unique<UnaryOpExpression>(op.value, std::move(right), op.line, op.column);
    }
    
    return parseCall();
}

ExpressionPtr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            auto args = parseArgumentList();
            consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
            
            // Convert identifier expression to function call
            if (auto identifier = dynamic_cast<IdentifierExpression*>(expr.get())) {
                std::string funcName = identifier->name;
                expr.release(); // Release ownership
                expr = std::make_unique<FunctionCallExpression>(funcName, std::move(args), identifier->line, identifier->column);
            } else {
                error("Invalid function call target");
            }
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionPtr Parser::parsePrimary() {
    if (match(TokenType::TRUE)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpression>(LiteralExpression::LiteralType::BOOLEAN, "true", token.line, token.column);
    }
    
    if (match(TokenType::FALSE)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpression>(LiteralExpression::LiteralType::BOOLEAN, "false", token.line, token.column);
    }
    
    if (match(TokenType::NULL_TOKEN)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpression>(LiteralExpression::LiteralType::NULL_LITERAL, "null", token.line, token.column);
    }
    
    if (match(TokenType::NUMBER)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpression>(LiteralExpression::LiteralType::NUMBER, token.value, token.line, token.column);
    }
    
    if (match(TokenType::STRING)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpression>(LiteralExpression::LiteralType::STRING, token.value, token.line, token.column);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        Token token = tokens[current - 1];
        return std::make_unique<IdentifierExpression>(token.value, token.line, token.column);
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    error("Expected expression");
    throw ParseError("Expected expression", currentToken());
}

std::vector<FunctionDeclaration::Parameter> Parser::parseParameterList() {
    std::vector<FunctionDeclaration::Parameter> parameters;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");
            consume(TokenType::COLON, "Expected ':' after parameter name");
            Token type = consume(TokenType::IDENTIFIER, "Expected parameter type");
            
            parameters.push_back({name.value, type.value});
        } while (match(TokenType::COMMA));
    }
    
    return parameters;
}

std::vector<ExpressionPtr> Parser::parseArgumentList() {
    std::vector<ExpressionPtr> arguments;
    
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA));
    }
    
    return arguments;
}

void Parser::error(const std::string& message) {
    Token& token = currentToken();
    std::cerr << "Parse error at " << token.line << ":" << token.column 
              << " (" << Token::tokenTypeToString(token.type) << " '" << token.value << "'): " 
              << message << std::endl;
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (tokens[current - 1].type == TokenType::SEMICOLON) return;
        
        switch (currentToken().type) {
            case TokenType::FUNCTION:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}

} // namespace emlang

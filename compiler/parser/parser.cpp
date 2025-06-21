//===--- parser.cpp - Recursive Descent Parser ------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// EMLang recursive descent parser implementation
//===----------------------------------------------------------------------===//

#include "parser/parser.h"
#include "parser/parser_error.h"
#include <iostream>
#include <stdexcept>

namespace emlang {

/// Constructor for the Parser class

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

/****************************************
*  Token Navigation Methods
****************************************/

Token& Parser::currentToken() {
    return tokens[current];
}

/**
 * @brief Look ahead at tokens without consuming them
 * @param offset Number of positions to look ahead (default: 1)
 * @return Reference to the token at current + offset
 * 
 * Essential for parsing decisions and disambiguation. If the requested
 * position is beyond the token sequence, returns the EOF token.
 */
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

/****************************************
*  Parsing Entry Point
****************************************/

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

/****************************************/

StatementPtr Parser::parseStatement() {
    try {
        // Skip newlines and handle empty lines
        while (match(TokenType::NEWLINE)) {
            // Skip newlines - continue to next statement
        }
        
        // Check if we've reached the end after skipping newlines
        if (isAtEnd()) {
            return nullptr;
        }
        
        // Standard declarations
        if (match(TokenType::LET) || match(TokenType::CONST)) {
            current--; // Back up to re-read the token
            return parseVariableDeclaration();
        }
        
        if (match(TokenType::FUNCTION)) {
            current--; // Back up to re-read the token
            return parseFunctionDeclaration();
        }
        
        if (match(TokenType::EXTERN)) {
            current--; // Back up to re-read the token
            return parseExternFunctionDeclaration();
        }
        
        // Control flow statements
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
        
        // Return statements
        if (match(TokenType::RETURN)) {
            current--; // Back up to re-read the token
            return parseReturnStatement();
        }
        
        // Block statements
        if (match(TokenType::LEFT_BRACE)) {
            current--; // Back up to re-read the token
            return parseBlockStatement();
        }
        
        // Check for tokens that should not start an expression statement
        if (check(TokenType::RIGHT_BRACE) || check(TokenType::EOF_TOKEN)) {
            return nullptr;
        }
        
        // Default to expression statement - parse an expression followed by semicolon
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        return std::make_unique<ExpressionStmt>(std::move(expr));
        
    } catch (const ParseError& e) {
        synchronize();
        throw e;
    }
}

/****************************************
*  Declaration Parsing Methods
****************************************/

StatementPtr Parser::parseVariableDeclaration() {
    bool isConst = match(TokenType::CONST);
    if (!isConst) {
        consume(TokenType::LET, "Expected 'let' or 'const'");
    }
    
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
    std::string type;
    if (match(TokenType::COLON)) {
        type = parseType();
    }
    
    ExpressionPtr initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    
    return std::make_unique<VariableDecl>(name.value, type, std::move(initializer), isConst, name.line, name.column);
}

StatementPtr Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNCTION, "Expected 'function'");
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    auto parameters = parseParameterList();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
      std::string returnType;
    if (match(TokenType::COLON)) {
        returnType = parseType();
    }
    
    auto body = parseBlockStatement();
    
    return std::make_unique<FunctionDecl>(name.value, std::move(parameters), returnType, std::move(body), name.line, name.column);
}

StatementPtr Parser::parseExternFunctionDeclaration() {
    consume(TokenType::EXTERN, "Expected 'extern'");
    consume(TokenType::FUNCTION, "Expected 'function' after 'extern'");
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    auto parameters = parseParameterList();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    
    std::string returnType;
    if (match(TokenType::COLON)) {
        returnType = parseType();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after extern function declaration");
      // Convert FunctionDecl::Parameter to ExternFunctionDecl::Parameter
    std::vector<Parameter> externParams;
    for (const auto& param : parameters) {
        externParams.push_back({param.name, param.type});
    }
    
    return std::make_unique<ExternFunctionDecl>(name.value, std::move(externParams), returnType, name.line, name.column);
}

/* RESERVED - IMPORT_DECL */

/****************************************
*  Statement Parsing Methods
****************************************/

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
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch), ifToken.line, ifToken.column);
}

StatementPtr Parser::parseWhileStatement() {
    Token whileToken = consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = parseStatement();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), whileToken.line, whileToken.column);
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
    
    return std::make_unique<ReturnStmt>(std::move(value), returnToken.line, returnToken.column);
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
    
    return std::make_unique<BlockStmt>(std::move(statements), leftBrace.line, leftBrace.column);
}

StatementPtr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    
    return std::make_unique<ExpressionStmt>(std::move(expr), expr->line, expr->column);
}

ExpressionPtr Parser::parseExpression() {
    // Parse assignment expressions (right-associative, lowest precedence)
    ExpressionPtr expr = parseLogicalOr();
    
    // Check for assignment operator
    if (match(TokenType::ASSIGN)) {
        Token op = tokens[current - 1];
        ExpressionPtr right = parseExpression(); // Right-associative, recursive call
        
        return std::make_unique<AssignmentExpr>(std::move(expr), std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(TokenType::LOGICAL_OR)) {
        Token op = tokens[current - 1];
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::LOGICAL_AND)) {
        Token op = tokens[current - 1];
        auto right = parseEquality();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({TokenType::NOT_EQUAL, TokenType::EQUAL})) {
        Token op = tokens[current - 1];
        auto right = parseComparison();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({TokenType::GREATER_THAN, TokenType::GREATER_EQUAL, TokenType::LESS_THAN, TokenType::LESS_EQUAL})) {
        Token op = tokens[current - 1];
        auto right = parseTerm();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = tokens[current - 1];
        auto right = parseFactor();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({TokenType::DIVIDE, TokenType::MULTIPLY, TokenType::MODULO})) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        expr = std::make_unique<BinaryOpExpr>(std::move(expr), tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
    return expr;
}

ExpressionPtr Parser::parseUnary() {
    if (match({TokenType::LOGICAL_NOT, TokenType::MINUS})) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        return std::make_unique<UnaryOpExpr>(tokenToBinOp(op), std::move(right), op.line, op.column);
    }
    
#ifdef EMLANG_ENABLE_POINTERS
    // Pointer dereference (*ptr)
    if (match(TokenType::MULTIPLY)) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        return std::make_unique<DereferenceExpr>(std::move(right), op.line, op.column);
    }
    
    // Address-of operator (&var)
    if (match(TokenType::AMPERSAND)) {
        Token op = tokens[current - 1];
        auto right = parseUnary();
        return std::make_unique<AddressOfExpr>(std::move(right), op.line, op.column);
    }
#endif

    return parseCall();
}

ExpressionPtr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            // Function call
            auto args = parseArgumentList();
            consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
            
            // Convert identifier expression to function call
            if (auto identifier = dynamic_cast<IdentifierExpr*>(expr.get())) {
                std::string funcName = identifier->name;
                expr.release(); // Release ownership
                expr = std::make_unique<FunctionCallExpr>(funcName, std::move(args), identifier->line, identifier->column);
            } else {
                error("Invalid function call target");
            }
        } else if (check(TokenType::DOT)) {
            // Member access
            expr = parseMemberAccess(std::move(expr));
        } else if (check(TokenType::LEFT_BRACKET)) {
            // Array/object indexing
            expr = parseIndexAccess(std::move(expr));
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionPtr Parser::parsePrimary() {
    // Numeric literals
    if (match(TokenType::INT)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::INT, token.value, token.line, token.column);
    }

    // Floating-point literals
    if (match(TokenType::FLOAT)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::FLOAT, token.value, token.line, token.column);
    }

    // Character literals
    if (match(TokenType::CHAR)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::CHAR, token.value, token.line, token.column);
    }
    // String literals
    if (match(TokenType::STR)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::STR, token.value, token.line, token.column);
    }

    // Boolean literals
    if (match(TokenType::BOOL)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::BOOL, token.value, token.line, token.column);
    }
    
    // Null literal
    if (match(TokenType::NULL_LITERAL)) {
        Token token = tokens[current - 1];
        return std::make_unique<LiteralExpr>(LiteralType::NULL_LITERAL, "null", token.line, token.column);
    }
    
    // Array literals
    if (check(TokenType::LEFT_BRACKET)) {
        return parseArrayLiteral();
    }
    
    // Object literals
    if (check(TokenType::LEFT_BRACE)) {
        return parseObjectLiteral();
    }
    
    // Identifier expressions
    if (match(TokenType::IDENTIFIER)) {
        Token token = tokens[current - 1];
        
        return std::make_unique<IdentifierExpr>(token.value, token.line, token.column);
    }
    
    // Parenthesized expressions
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    error("Expected expression");
    return nullptr;
}

std::string Parser::parseType() {
    std::string baseType;
    
    // Check for primitive types
    if (check(TokenType::INT)) {
        advance();
        baseType = "int";
    } else if (check(TokenType::FLOAT)) {
        advance();
        baseType = "float";
    } else if (check(TokenType::CHAR)) {
        advance();
        baseType = "char";
    } else if (check(TokenType::STR)) {
        advance();
        baseType = "str";
    } else if (check(TokenType::BOOL)) {
        advance();
        baseType = "bool";
    } else if (check(TokenType::IDENTIFIER)) {
        // Custom types (structs, classes, etc.)
        Token typeToken = advance();
        baseType = typeToken.value;
    } else {
        error("Expected type name");
        throw ParseError("Expected type name", currentToken());
    }
    
    //! Experimental feature: Pointers
    // Check for pointer modifiers (C-style: int32*, char**, etc.)
    while (check(TokenType::MULTIPLY)) {
        error("Pointer types are not supported in this version of EMLang", currentToken().line, currentToken().column);
        advance();
        baseType += "*";
    }
    
    return baseType;
}

std::vector<Parameter> Parser::parseParameterList() {
    std::vector<Parameter> parameters;
    
    if (!check(TokenType::RIGHT_PAREN)) {        
        do {
            Token name = consume(TokenType::IDENTIFIER, "Expected parameter name");
            consume(TokenType::COLON, "Expected ':' after parameter name");
            std::string type = parseType();
            
            parameters.push_back({name.value, type});
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

std::string Parser::parsePointerType() {
    // This method is for future advanced pointer parsing
    // For now, we use parseType() which already handles pointers
    return parseType();
}

// ======================== MODERN EXPRESSION PARSING METHODS ========================

#ifdef EMLANG_FEATURE_CASTING
ExpressionPtr Parser::parseCastExpression(ExpressionPtr operand) {
    consume(TokenType::AS, "Expected 'as' keyword for cast expression");
    std::string targetType = parseType();
    return std::make_unique<CastExpr>(std::move(operand), targetType);
}
#endif

ExpressionPtr Parser::parseArrayLiteral() {
    consume(TokenType::LEFT_BRACKET, "Expected '[' to start array literal");
    
    std::vector<ExpressionPtr> elements;
    
    // Handle empty array
    if (check(TokenType::RIGHT_BRACKET)) {
        advance(); // consume ']'
        return std::make_unique<ArrayExpr>(std::move(elements));
    }
    
    // Parse array elements
    do {
        elements.push_back(parseExpression());
    } while (match(TokenType::COMMA));
    
    consume(TokenType::RIGHT_BRACKET, "Expected ']' after array elements");
    return std::make_unique<ArrayExpr>(std::move(elements));
}

ExpressionPtr Parser::parseObjectLiteral() {
    consume(TokenType::LEFT_BRACE, "Expected '{' to start object literal");
    
    std::vector<ObjectField> fields;
    
    // Handle empty object
    if (check(TokenType::RIGHT_BRACE)) {
        advance(); // consume '}'
        return std::make_unique<ObjectExpr>(std::move(fields));
    }
    
    // Parse object fields
    do {
        std::string key;
        
        // Parse key (identifier or string literal)
        if (check(TokenType::IDENTIFIER)) {
            key = currentToken().value;
            advance();
        } else if (check(TokenType::STR)) {
            key = currentToken().value;
            advance();
        } else {
            error("Expected identifier or string literal for object key");
        }
          consume(TokenType::COLON, "Expected ':' after object key");
        ExpressionPtr value = parseExpression();
        
        fields.emplace_back(key, std::move(value));
    } while (match(TokenType::COMMA));
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after object fields");    
    return std::make_unique<ObjectExpr>(std::move(fields));
}

/****************************************
*  Member Access and Index Access Methods
****************************************/

ExpressionPtr Parser::parseMemberAccess(ExpressionPtr object) {
    consume(TokenType::DOT, "Expected '.' for member access");
    
    if (!check(TokenType::IDENTIFIER)) {
        error("Expected member name after '.'");
        return nullptr;
    }
    
    Token memberToken = advance();
    std::string memberName = memberToken.value;
    
    // Check if this is a method call by looking ahead for parentheses
    bool isMethodCall = check(TokenType::LEFT_PAREN);
    
    return std::make_unique<MemberExpr>(
        std::move(object), 
        memberName, 
        isMethodCall,
        memberToken.line, 
        memberToken.column
    );
}

ExpressionPtr Parser::parseIndexAccess(ExpressionPtr array) {
    consume(TokenType::LEFT_BRACKET, "Expected '[' for index access");
    
    ExpressionPtr index = parseExpression();
    if (!index) {
        error("Expected index expression");
        return nullptr;
    }
    
    Token closeBracket = consume(TokenType::RIGHT_BRACKET, "Expected ']' after index expression");
    
    return std::make_unique<IndexExpr>(
        std::move(array), 
        std::move(index),
        closeBracket.line, 
        closeBracket.column
    );
}

// ======================== OPTIMIZATION HELPERS ========================

ExpressionPtr Parser::optimizeConstantExpression(ExpressionPtr expr) {
    // Basic constant folding for numeric literals
    if (auto binaryExpr = dynamic_cast<BinaryOpExpr*>(expr.get())) {
        auto leftLit = dynamic_cast<LiteralExpr*>(binaryExpr->left.get());
        auto rightLit = dynamic_cast<LiteralExpr*>(binaryExpr->right.get());
        
        if (leftLit && rightLit && 
            leftLit->literalType == LiteralType::INT && 
            rightLit->literalType == LiteralType::INT) {
            
            int leftVal = std::stoi(leftLit->value);
            int rightVal = std::stoi(rightLit->value);
            int result = 0;
            
            BinaryOpExpr::BinOp op = binaryExpr->operator_;
            if (binOpToString(op) == "+") result = leftVal + rightVal;
            else if (binOpToString(op) == "-") result = leftVal - rightVal;
            else if (binOpToString(op) == "*") result = leftVal * rightVal;
            else if (binOpToString(op) == "/") {
                if (rightVal != 0) result = leftVal / rightVal;
                else return expr; // Don't optimize division by zero
            } else {
                return expr; // Unknown operator
            }
            
            return std::make_unique<LiteralExpr>(LiteralType::INT, std::to_string(result));
        }
    }
    
    return expr;
}

bool Parser::isConstantExpression(const ExpressionPtr& expr) {
    if (auto literal = dynamic_cast<const LiteralExpr*>(expr.get())) {
        return true;
    }
    
    if (auto binary = dynamic_cast<const BinaryOpExpr*>(expr.get())) {
        return isConstantExpression(binary->left) && 
               isConstantExpression(binary->right);
    }
    
    if (auto unary = dynamic_cast<const UnaryOpExpr*>(expr.get())) {
        return isConstantExpression(unary->operand);
    }
    
    return false;
}

bool Parser::validateOperatorPrecedence(const ExpressionPtr& leftExpr, 
                                       const ExpressionPtr& rightExpr, 
                                       const std::string& op) {
    // Simplified precedence validation - could be expanded
    // Returns true if precedence is clear, false if parentheses might help readability
    
    auto leftBinary = dynamic_cast<const BinaryOpExpr*>(leftExpr.get());
    auto rightBinary = dynamic_cast<const BinaryOpExpr*>(rightExpr.get());
    
    if (!leftBinary && !rightBinary) {
        return true; // No nested binary expressions
    }
    
    // Could implement more sophisticated precedence checking here    
    return true;
}

/****************************************
*  Helper Functions
****************************************/

BinaryOpExpr::BinOp Parser::tokenToBinOp(const Token& token) {
    switch (token.type) {
        case TokenType::PLUS: return BinaryOpExpr::BinOp::ADD;
        case TokenType::MINUS: return BinaryOpExpr::BinOp::SUB;
        case TokenType::MULTIPLY: return BinaryOpExpr::BinOp::MUL;
        case TokenType::DIVIDE: return BinaryOpExpr::BinOp::DIV;
        case TokenType::MODULO: return BinaryOpExpr::BinOp::MOD;
        case TokenType::BITWISE_AND: return BinaryOpExpr::BinOp::AND;
        case TokenType::BITWISE_OR: return BinaryOpExpr::BinOp::OR;
        case TokenType::BITWISE_XOR: return BinaryOpExpr::BinOp::XOR;
        case TokenType::BITWISE_INVERT: return BinaryOpExpr::BinOp::INV;
        case TokenType::LEFT_SHIFT: return BinaryOpExpr::BinOp::SHL;
        case TokenType::RIGHT_SHIFT: return BinaryOpExpr::BinOp::SHR;
        case TokenType::EQUAL: return BinaryOpExpr::BinOp::EQ;
        case TokenType::NOT_EQUAL: return BinaryOpExpr::BinOp::NE;
        case TokenType::LESS_THAN: return BinaryOpExpr::BinOp::LT;
        case TokenType::LESS_EQUAL: return BinaryOpExpr::BinOp::LE;  
        case TokenType::GREATER_THAN: return BinaryOpExpr::BinOp::GT;
        case TokenType::GREATER_EQUAL: return BinaryOpExpr::BinOp::GE;
        case TokenType::LOGICAL_AND: return BinaryOpExpr::BinOp::LAND;
        // Note: LOGICAL_OR and LOGICAL_NOT have conflicts with BITWISE tokens in token.h
        // Skipping them for now until token.h is fixed
        default:
            // Use existing error function that's already defined below
            std::cerr << "Invalid binary operator token" << std::endl;
            return BinaryOpExpr::BinOp::ADD; // Default fallback
    }
}

// ======================== Parser::ParseError Implementation ========================

void Parser::error(const std::string& message, size_t line, size_t column) {
    if (line == 0 || column == 0) {
        Token& token = currentToken();
        line = token.line;
        column = token.column;
        std::cerr << "Parse error at " << token.line << ":" << token.column 
                  << " (" << Token::tokenTypeToString(token.type) << " '" << token.value << "'): " 
                  << message << std::endl;
    } else {
        std::cerr << "Parse error at " << line << ":" << column << ": " 
                  << message << std::endl;
    }
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

Parser::ParseError::ParseError(const std::string& msg, const Token& tok)
    : message(msg), token(tok) {}

const char* Parser::ParseError::what() const noexcept {
    return message.c_str();
}

const Token& Parser::ParseError::getToken() const {
    return token;
}

}

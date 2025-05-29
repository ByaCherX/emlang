// Semantic Analyzer implementation
#include "../include/semantic.h"
#include <iostream>

namespace emlang {

// Scope implementation
Scope::Scope(Scope* parent) : parent(parent) {}

bool Scope::define(const std::string& name, const std::string& type, bool isConst, bool isFunc, size_t line, size_t column) {
    if (symbols.find(name) != symbols.end()) {
        return false; // Symbol already exists in this scope
    }
    
    symbols[name] = std::make_unique<Symbol>(name, type, isConst, isFunc, line, column);
    return true;
}

Symbol* Scope::lookup(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    
    if (parent) {
        return parent->lookup(name);
    }
    
    return nullptr;
}

bool Scope::exists(const std::string& name) {
    return lookup(name) != nullptr;
}

bool Scope::existsInCurrentScope(const std::string& name) {
    return symbols.find(name) != symbols.end();
}

Scope* Scope::getParent() const {
    return parent;
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer() : currentScope(nullptr), hasErrors(false) {
    // Create global scope
    enterScope();
}

bool SemanticAnalyzer::analyze(Program& program) {
    hasErrors = false;
    program.accept(*this);
    return !hasErrors;
}

bool SemanticAnalyzer::hasSemanticErrors() const {
    return hasErrors;
}

void SemanticAnalyzer::enterScope() {
    auto newScope = std::make_unique<Scope>(currentScope);
    currentScope = newScope.get();
    scopes.push_back(std::move(newScope));
}

void SemanticAnalyzer::exitScope() {
    if (currentScope && currentScope->getParent()) {
        currentScope = currentScope->getParent();
    }
}

std::string SemanticAnalyzer::getExpressionType(Expression& expr) {
    expr.accept(*this);
    return currentExpressionType;
}

bool SemanticAnalyzer::isCompatibleType(const std::string& expected, const std::string& actual) {
    // For now, exact match required
    return expected == actual;
}

bool SemanticAnalyzer::isNumericType(const std::string& type) {
    return type == "number" || type == "int" || type == "float" || type == "double";
}

bool SemanticAnalyzer::isBooleanType(const std::string& type) {
    return type == "boolean" || type == "bool";
}

bool SemanticAnalyzer::isStringType(const std::string& type) {
    return type == "string";
}

void SemanticAnalyzer::error(const std::string& message, size_t line, size_t column) {
    std::cerr << "Semantic Error";
    if (line > 0) {
        std::cerr << " at " << line << ":" << column;
    }
    std::cerr << " - " << message << std::endl;
    hasErrors = true;
}

void SemanticAnalyzer::warning(const std::string& message, size_t line, size_t column) {
    std::cout << "Warning";
    if (line > 0) {
        std::cout << " at " << line << ":" << column;
    }
    std::cout << " - " << message << std::endl;
}

// AST Visitor implementations
void SemanticAnalyzer::visit(LiteralExpression& node) {
    switch (node.literalType) {
        case LiteralExpression::LiteralType::NUMBER:
            currentExpressionType = "number";
            break;
        case LiteralExpression::LiteralType::STRING:
            currentExpressionType = "string";
            break;
        case LiteralExpression::LiteralType::BOOLEAN:
            currentExpressionType = "boolean";
            break;
        case LiteralExpression::LiteralType::NULL_LITERAL:
            currentExpressionType = "null";
            break;
    }
}

void SemanticAnalyzer::visit(IdentifierExpression& node) {
    Symbol* symbol = currentScope->lookup(node.name);
    if (!symbol) {
        error("Undefined identifier: " + node.name, node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    currentExpressionType = symbol->type;
}

void SemanticAnalyzer::visit(BinaryOpExpression& node) {
    std::string leftType = getExpressionType(*node.left);
    std::string rightType = getExpressionType(*node.right);
    
    // Arithmetic operators
    if (node.operator_ == "+" || node.operator_ == "-" || node.operator_ == "*" || 
        node.operator_ == "/" || node.operator_ == "%") {
        
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            error("Arithmetic operations require numeric types", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "number";
    }
    // Comparison operators
    else if (node.operator_ == "<" || node.operator_ == ">" || 
             node.operator_ == "<=" || node.operator_ == ">=") {
        
        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            error("Comparison operations require numeric types", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "boolean";
    }
    // Equality operators
    else if (node.operator_ == "==" || node.operator_ == "!=") {
        if (!isCompatibleType(leftType, rightType)) {
            error("Equality operations require compatible types", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "boolean";
    }
    // Logical operators
    else if (node.operator_ == "&&" || node.operator_ == "||") {
        if (!isBooleanType(leftType) || !isBooleanType(rightType)) {
            error("Logical operations require boolean types", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "boolean";
    }
    else {
        error("Unknown binary operator: " + node.operator_, node.line, node.column);
        currentExpressionType = "error";
    }
}

void SemanticAnalyzer::visit(UnaryOpExpression& node) {
    std::string operandType = getExpressionType(*node.operand);
    
    if (node.operator_ == "-") {
        if (!isNumericType(operandType)) {
            error("Unary minus requires numeric type", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "number";
    }
    else if (node.operator_ == "!") {
        if (!isBooleanType(operandType)) {
            error("Logical not requires boolean type", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "boolean";
    }
    else {
        error("Unknown unary operator: " + node.operator_, node.line, node.column);
        currentExpressionType = "error";
    }
}

void SemanticAnalyzer::visit(FunctionCallExpression& node) {
    Symbol* symbol = currentScope->lookup(node.functionName);
    if (!symbol) {
        error("Undefined function: " + node.functionName, node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    if (!symbol->isFunction) {
        error("Identifier is not a function: " + node.functionName, node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    // TODO: Check argument types and count
    currentExpressionType = symbol->type;
}

void SemanticAnalyzer::visit(VariableDeclaration& node) {
    // Check if variable already exists in current scope
    if (currentScope->existsInCurrentScope(node.name)) {
        error("Variable already declared in current scope: " + node.name, node.line, node.column);
        return;
    }
    
    // Type check initializer if present
    if (node.initializer) {
        std::string initType = getExpressionType(*node.initializer);
        if (!node.type.empty() && !isCompatibleType(node.type, initType)) {
            error("Type mismatch in variable declaration: expected " + node.type + ", got " + initType, 
                  node.line, node.column);
            return;
        }
    }
    
    // Define variable in current scope
    std::string varType = node.type.empty() ? currentExpressionType : node.type;
    currentScope->define(node.name, varType, node.isConstant, false, node.line, node.column);
}

void SemanticAnalyzer::visit(FunctionDeclaration& node) {
    // Check if function already exists in current scope
    if (currentScope->existsInCurrentScope(node.name)) {
        error("Function already declared: " + node.name, node.line, node.column);
        return;
    }
    
    // Define function in current scope
    currentScope->define(node.name, node.returnType, false, true, node.line, node.column);
    
    // Enter function scope
    enterScope();
    
    // Define parameters
    for (const auto& param : node.parameters) {
        currentScope->define(param.name, param.type, false, false, node.line, node.column);
    }
    
    // Set current function return type
    std::string oldReturnType = currentFunctionReturnType;
    currentFunctionReturnType = node.returnType;
    
    // Analyze function body
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Restore previous function return type
    currentFunctionReturnType = oldReturnType;
    
    // Exit function scope
    exitScope();
}

void SemanticAnalyzer::visit(BlockStatement& node) {
    enterScope();
    
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    
    exitScope();
}

void SemanticAnalyzer::visit(IfStatement& node) {
    std::string conditionType = getExpressionType(*node.condition);
    if (!isBooleanType(conditionType)) {
        error("If condition must be boolean type", node.line, node.column);
    }
    
    if (node.thenBranch) {
        node.thenBranch->accept(*this);
    }
    
    if (node.elseBranch) {
        node.elseBranch->accept(*this);
    }
}

void SemanticAnalyzer::visit(WhileStatement& node) {
    std::string conditionType = getExpressionType(*node.condition);
    if (!isBooleanType(conditionType)) {
        error("While condition must be boolean type", node.line, node.column);
    }
    
    if (node.body) {
        node.body->accept(*this);
    }
}

void SemanticAnalyzer::visit(ReturnStatement& node) {
    if (currentFunctionReturnType.empty()) {
        error("Return statement outside of function", node.line, node.column);
        return;
    }
    
    if (node.value) {
        std::string returnType = getExpressionType(*node.value);
        if (!isCompatibleType(currentFunctionReturnType, returnType)) {
            error("Return type mismatch: expected " + currentFunctionReturnType + ", got " + returnType, 
                  node.line, node.column);
        }
    } else if (currentFunctionReturnType != "void") {
        error("Function must return a value", node.line, node.column);
    }
}

void SemanticAnalyzer::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void SemanticAnalyzer::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

} // namespace emlang

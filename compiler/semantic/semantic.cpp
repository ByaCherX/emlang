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
    // Check for exact match
    if (expected == actual) return true;
    
    // Check for implicit conversion
    if (canImplicitlyConvert(actual, expected)) return true;
    
    // Special case: allow "number" literals to be assigned to any numeric type
    if (actual == "number" && isNumericType(expected)) return true;
    
    // Special case: allow "boolean" literals to be assigned to "bool" type
    if (actual == "boolean" && expected == "bool") return true;
    
    // Special case: allow "char" literals to be assigned to "char" type
    if (actual == "char" && expected == "char") return true;
    
    // Special case: allow "string" literals to be assigned to "str" type
    if (actual == "string" && expected == "str") return true;
    
    // Special case: allow "string" literals to be assigned to "char" (single character)
    if (actual == "string" && expected == "char") return true;
    
    return false;
}

bool SemanticAnalyzer::isNumericType(const std::string& type) {
    // Legacy types
    if (type == "number" || type == "int") {
        return true;
    }
    
    // C-style signed integer types
    if (type == "int8" || type == "int16" || type == "int32" || type == "int64" || type == "isize") {
        return true;
    }
    
    // C-style unsigned integer types
    if (type == "uint8" || type == "uint16" || type == "uint32" || type == "uint64" || type == "usize") {
        return true;
    }
    
    // C-style floating point types
    if (type == "float" || type == "double") {
        return true;
    }
    
    return false;
}

bool SemanticAnalyzer::isBooleanType(const std::string& type) {
    return type == "boolean" || type == "bool";
}

bool SemanticAnalyzer::isStringType(const std::string& type) {
    return type == "string" || type == "str";
}

// C-style type checking helpers
bool SemanticAnalyzer::isSignedInteger(const std::string& type) {
    return type == "int8" || type == "int16" || type == "int32" || 
           type == "int64" || type == "isize" || type == "int" || type == "number";
}

bool SemanticAnalyzer::isUnsignedInteger(const std::string& type) {
    return type == "uint8" || type == "uint16" || type == "uint32" || 
           type == "uint64" || type == "usize";
}

bool SemanticAnalyzer::isFloatingPoint(const std::string& type) {
    return type == "float" || type == "double";
}

bool SemanticAnalyzer::isCharType(const std::string& type) {
    return type == "char";
}

bool SemanticAnalyzer::isUnitType(const std::string& type) {
    return type == "()" || type == "void";
}

bool SemanticAnalyzer::isPrimitiveType(const std::string& type) {
    return isSignedInteger(type) || isUnsignedInteger(type) || 
           isFloatingPoint(type) || isBooleanType(type) ||
           isCharType(type) || isUnitType(type) || isStringType(type);
}

bool SemanticAnalyzer::canImplicitlyConvert(const std::string& from, const std::string& to) {
    if (from == to) return true;
    
    // Allow "number" literals to be converted to any numeric type
    if (from == "number" && isNumericType(to)) return true;
    
    // Implicit widening conversions for integers
    if (from == "int8" && (to == "int16" || to == "int32" || to == "int64")) return true;
    if (from == "int16" && (to == "int32" || to == "int64")) return true;
    if (from == "int32" && to == "int64") return true;
    
    if (from == "uint8" && (to == "uint16" || to == "uint32" || to == "uint64")) return true;
    if (from == "uint16" && (to == "uint32" || to == "uint64")) return true;
    if (from == "uint32" && to == "uint64") return true;
    
    // Implicit conversion for floating point
    if (from == "float" && to == "double") return true;
    
    // Legacy compatibility (for backward compatibility with old naming)
    if (from == "int" && to == "int32") return true;
    if (from == "i8" && to == "int8") return true;
    if (from == "i16" && to == "int16") return true;
    if (from == "i32" && to == "int32") return true;
    if (from == "i64" && to == "int64") return true;
    if (from == "u8" && to == "uint8") return true;
    if (from == "u16" && to == "uint16") return true;
    if (from == "u32" && to == "uint32") return true;
    if (from == "u64" && to == "uint64") return true;
    if (from == "f32" && to == "float") return true;
    if (from == "f64" && to == "double") return true;
    if (from == "boolean" && to == "bool") return true;
    
    return false;
}

std::string SemanticAnalyzer::getCommonType(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return type1;
    
    // If one can be implicitly converted to the other, use the target type
    if (canImplicitlyConvert(type1, type2)) return type2;
    if (canImplicitlyConvert(type2, type1)) return type1;
    
    // For numeric operations, promote to larger type
    if (isSignedInteger(type1) && isSignedInteger(type2)) {
        if (type1 == "int64" || type2 == "int64") return "int64";
        if (type1 == "int32" || type2 == "int32") return "int32";
        if (type1 == "int16" || type2 == "int16") return "int16";
        return "int8";
    }
    
    if (isUnsignedInteger(type1) && isUnsignedInteger(type2)) {
        if (type1 == "uint64" || type2 == "uint64") return "uint64";
        if (type1 == "uint32" || type2 == "uint32") return "uint32";
        if (type1 == "uint16" || type2 == "uint16") return "uint16";
        return "uint8";
    }
    
    if (isFloatingPoint(type1) && isFloatingPoint(type2)) {
        if (type1 == "double" || type2 == "double") return "double";
        return "float";
    }
    
    // No common type found
    return "";
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
        case LiteralExpression::LiteralType::CHAR:
            currentExpressionType = "char";
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

void SemanticAnalyzer::visit(ExternFunctionDeclaration& node) {
    // Check if function name already exists in current scope
    if (currentScope->lookup(node.name)) {
        error("Function '" + node.name + "' is already declared", node.line, node.column);
        return;
    }
    
    // Validate parameter types
    for (const auto& param : node.parameters) {
        if (!isPrimitiveType(param.type) && !isPointerType(param.type)) {
            error("Invalid parameter type '" + param.type + "' in extern function '" + node.name + "'", node.line, node.column);
        }
    }
    
    // Validate return type
    if (!isPrimitiveType(node.returnType) && !isPointerType(node.returnType) && node.returnType != "void") {
        error("Invalid return type '" + node.returnType + "' in extern function '" + node.name + "'", node.line, node.column);
    }
    
    // Register extern function in symbol table
    // External functions are marked as functions and constant (cannot be redefined)
    currentScope->define(node.name, node.returnType, true, true, node.line, node.column);
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

void SemanticAnalyzer::visit(DereferenceExpression& node) {
    node.operand->accept(*this);
    std::string operandType = getExpressionType(*node.operand);
    
    if (!isPointerType(operandType)) {
        error("Cannot dereference non-pointer type '" + operandType + "'", node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    // Dereference gives us the base type
    currentExpressionType = getPointerBaseType(operandType);
}

void SemanticAnalyzer::visit(AddressOfExpression& node) {
    node.operand->accept(*this);
    std::string operandType = getExpressionType(*node.operand);
    
    // Address-of operation creates a pointer to the operand type
    currentExpressionType = makePointerType(operandType);
}

void SemanticAnalyzer::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

bool SemanticAnalyzer::isPointerType(const std::string& type) {
    return type.length() > 0 && type.back() == '*';
}

std::string SemanticAnalyzer::getPointerBaseType(const std::string& pointerType) {
    if (!isPointerType(pointerType)) {
        return pointerType;
    }
    
    // Remove one level of pointer indirection
    return pointerType.substr(0, pointerType.length() - 1);
}

std::string SemanticAnalyzer::makePointerType(const std::string& baseType) {
    return baseType + "*";
}

} // namespace emlang

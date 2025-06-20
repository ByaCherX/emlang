
#include "semantic/analyzer.h"
#include "semantic/type_checker.h"
#include "../../include/builtins.h"
#include "ast.h"
#include <iostream>

namespace emlang {


Analyzer::Analyzer() : currentScope(nullptr), hasErrors(false) {
    // Initialize global scope
    scopes.push_back(std::make_unique<Scope>(nullptr));
    currentScope = scopes.back().get();
    
    // Register built-in functions
    registerBuiltinFunctions();
}

/***************************************
*  SCOPE MANAGEMENT METHODS
***************************************/

void Analyzer::enterScope() {
    scopes.push_back(std::make_unique<Scope>(currentScope));
    currentScope = scopes.back().get();
}

void Analyzer::exitScope() {
    if (scopes.size() > 1) { // Don't exit global scope
        scopes.pop_back();
        currentScope = scopes.back().get();
    }
}

std::string Analyzer::getExpressionType(Expression& expr) {
    // Save current expression type
    std::string oldType = currentExpressionType;
    
    // Visit the expression to determine its type
    expr.accept(*this);
    
    // Get the determined type
    std::string resultType = currentExpressionType;
    
    // Restore previous expression type
    currentExpressionType = oldType;
    
    return resultType;
}

/***************************************
*  ANALYZER MAIN
***************************************/

bool Analyzer::analyze(Program& program) {
    hasErrors = false;
    
    // Analyze all top-level statements
    program.accept(*this);
    
    
    return !hasErrors;
}
bool Analyzer::hasSemanticErrors() const {
    return hasErrors;
}

void Analyzer::registerBuiltinFunctions() {
    // Get built-in functions from builtins.cpp
    std::map<std::string, emlang::BuiltinFunction> builtins = getBuiltinFunctions();
    
    for (const auto& [name, builtin] : builtins) {
        // Register each built-in function as a function symbol in global scope
        // Use a special function type signature format: "returnType(param1Type,param2Type,...)"
        std::string signature = builtin.returnType + "(";
        for (size_t i = 0; i < builtin.parameters.size(); ++i) {
            if (i > 0) signature += ",";
            signature += builtin.parameters[i].type;
        }
        signature += ")";
        
        // Register as function symbol in global scope
        currentScope->define(name, signature, true, true, 0, 0);
    }
}

/***************************************
*  AST VISITOR IMPLEMENTATIONS
***************************************/

void Analyzer::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

/******************************
* Visitor - Expression
******************************/

void Analyzer::visit(LiteralExpr& node) {
    switch (node.literalType) {
        case LiteralType::INT:
            currentExpressionType = "int32";
            break;
        case LiteralType::FLOAT:
            currentExpressionType = "float";
            break;
        case LiteralType::STR:
            currentExpressionType = "str";
            break;
        case LiteralType::CHAR:
            currentExpressionType = "char";
            break;
        case LiteralType::BOOL:
            currentExpressionType = "bool";
            break;
        case LiteralType::NULL_LITERAL:
            currentExpressionType = "null";
            break;
    }
}

void Analyzer::visit(IdentifierExpr& node) {
    Symbol* symbol = currentScope->lookup(node.name);
    if (!symbol) {
        error("Undefined identifier: " + node.name, node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    currentExpressionType = symbol->type;
}

void Analyzer::visit(BinaryOpExpr& node) {
    std::string leftType = getExpressionType(*node.left);
    std::string rightType = getExpressionType(*node.right);
    
    // Arithmetic operators
    if (node.operator_ == BinaryOpExpr::BinOp::ADD || 
        node.operator_ == BinaryOpExpr::BinOp::SUB || 
        node.operator_ == BinaryOpExpr::BinOp::MUL ||
        node.operator_ == BinaryOpExpr::BinOp::DIV || 
        node.operator_ == BinaryOpExpr::BinOp::MOD) {
        
        if (!TypeChecker::isNumericType(leftType) || !TypeChecker::isNumericType(rightType)) {
            error("Arithmetic operations require numeric types", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = TypeChecker::promoteNumericTypes(leftType, rightType);
    }
    // Comparison operators
    else if (node.operator_ == BinaryOpExpr::BinOp::LT || 
             node.operator_ == BinaryOpExpr::BinOp::LE ||
             node.operator_ == BinaryOpExpr::BinOp::GT || 
             node.operator_ == BinaryOpExpr::BinOp::GE) {
        
        if (!TypeChecker::isNumericType(leftType) || !TypeChecker::isNumericType(rightType)) {
            error("Comparison operations require numeric types", node.line, node.column);
        }
        currentExpressionType = "bool";
    }
    // Equality operators
    else if (node.operator_ == BinaryOpExpr::BinOp::EQ || 
             node.operator_ == BinaryOpExpr::BinOp::NE) {
        
        if (!TypeChecker::areTypesCompatible(leftType, rightType)) {
            error("Cannot compare incompatible types: " + leftType + " and " + rightType, 
                  node.line, node.column);
        }
        currentExpressionType = "bool";
    }
    // Logical operators
    else if (node.operator_ == BinaryOpExpr::BinOp::LAND || 
             node.operator_ == BinaryOpExpr::BinOp::LOR) {
        
        if (leftType != "bool" || rightType != "bool") {
            error("Logical operations require boolean operands", node.line, node.column);
        }
        currentExpressionType = "bool";    
    }
    else {
        error("Unknown binary operator: " + binOpToString(node.operator_), node.line, node.column);
        currentExpressionType = "error";
    }
}

void Analyzer::visit(UnaryOpExpr& node) {
    std::string operandType = getExpressionType(*node.operand);
    
    if (node.operator_ == BinaryOpExpr::BinOp::SUB) {
        if (!TypeChecker::isNumericType(operandType)) {
            error("Unary minus requires numeric type", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = operandType; // Keep the same numeric type
    }
    else if (node.operator_ == BinaryOpExpr::BinOp::LNOT) {
        if (operandType != "bool") {
            error("Logical not requires boolean type", node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        currentExpressionType = "bool";
    }
    else {
        error("Unknown unary operator: " + binOpToString(node.operator_), node.line, node.column);
        currentExpressionType = "error";
    }
}

void Analyzer::visit(AssignmentExpr& node) {
    // Check if target is a valid lvalue that can be assigned to
    node.target->accept(*this);
    std::string targetType = currentExpressionType;
    
    // Check if target is a valid lvalue
    bool isValidLvalue = false;
    if (auto* identExpr = dynamic_cast<IdentifierExpr*>(node.target.get())) {
        // Target is an identifier, check if it exists and is not a constant
        Symbol* symbol = currentScope->lookup(identExpr->name);
        if (!symbol) {
            error("Undefined variable: " + identExpr->name, node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        
        if (symbol->isConstant) {
            error("Cannot assign to const variable: " + identExpr->name, node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        isValidLvalue = true;
    }
#ifdef EMLANG_FEATURE_POINTERS
    else if (dynamic_cast<DereferenceExpr*>(node.target.get())) {
        // Target is a dereference expression, which is a valid lvalue
        if (!TypeChecker::isPointerType(targetType)) {
            error("Cannot dereference non-pointer type: " + targetType, node.line, node.column);
            currentExpressionType = "error";
            return;
        }
        isValidLvalue = true;
    }
#endif // EMLANG_FEATURE_POINTERS
    
    if (!isValidLvalue) {
        error("Left side of assignment is not a valid lvalue", node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    // Evaluate and check the value expression
    node.value->accept(*this);
    std::string valueType = currentExpressionType;
    
    // Check type compatibility
    if (!TypeChecker::isCompatibleType(targetType, valueType)) {
        error("Type mismatch in assignment: cannot assign " + valueType + " to " + targetType, 
              node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    // Assignment expressions have the type of the target
    currentExpressionType = targetType;
}

void Analyzer::visit(FunctionCallExpr& node) {
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

void Analyzer::visit(MemberExpr& node) {
    // Analyze the object expression first
    std::string objectType = getExpressionType(*node.object);
    
    // For now, set the expression type to unknown
    // TODO: Implement proper member access type checking
    currentExpressionType = "unknown";
    
    // TODO: Add proper member resolution logic
    // This would involve checking if the object type has the requested member
    // and determining the member's type
}

void Analyzer::visit(IndexExpr& node) {
    // Analyze the array expression
    std::string arrayType = getExpressionType(*node.array);
    
    // Analyze the index expression
    std::string indexType = getExpressionType(*node.index);
    
    // Check that index is an integer type
    if (indexType != "int32" && indexType != "int64" && indexType != "isize" && 
        indexType != "uint32" && indexType != "uint64" && indexType != "usize") {
        error("Array index must be an integer type, got: " + indexType, 
              node.line, node.column);
    }
    
    // TODO: Extract element type from array type
    // For now, assume unknown type
    currentExpressionType = "unknown";
}

void Analyzer::visit(ArrayExpr& node) {
    // Analyze all elements
    std::string elementType = "";
    bool firstElement = true;
    
    for (auto& element : node.elements) {
        std::string currentElementType = getExpressionType(*element);
        
        if (firstElement) {
            elementType = currentElementType;
            firstElement = false;
        } else if (elementType != currentElementType) {
            error("Array elements must have the same type. Expected: " + 
                  elementType + ", got: " + currentElementType, 
                  node.line, node.column);
        }
    }
    
    // Set array type (element type + [])
    currentExpressionType = elementType.empty() ? "unknown[]" : elementType + "[]";
}

void Analyzer::visit(ObjectExpr& node) {
    // Analyze all field values
    for (auto& field : node.fields) {
        getExpressionType(*field.value);
    }
    
    // For now, set object type to generic object
    currentExpressionType = "object";
}

#ifdef EMLANG_FEATURE_CASTING
void Analyzer::visit(CastExpr& node) {
    // Analyze the operand expression
    std::string operandType = getExpressionType(*node.operand);
    
    // TODO: Add cast validity checking
    // Check if cast from operandType to targetType is valid
    
    // Set the expression type to the target type
    currentExpressionType = node.targetType;
}
#endif // EMLANG_FEATURE_CASTING

#ifdef EMLANG_FEATURE_POINTERS
void Analyzer::visit(DereferenceExpr& node) {
    node.operand->accept(*this);
    std::string operandType = getExpressionType(*node.operand);
    
    if (!TypeChecker::isPointerType(operandType)) {
        error("Cannot dereference non-pointer type '" + operandType + "'", node.line, node.column);
        currentExpressionType = "error";
        return;
    }
    
    // Dereference gives us the base type
    currentExpressionType = TypeChecker::getPointerBaseType(operandType);
}

void Analyzer::visit(AddressOfExpr& node) {
    node.operand->accept(*this);
    std::string operandType = getExpressionType(*node.operand);
    
    // Address-of operation creates a pointer to the operand type
    currentExpressionType = TypeChecker::makePointerType(operandType);
}
#endif // EMLANG_FEATURE_POINTERS

/******************************
* Visitor - Declaration
******************************/

void Analyzer::visit(VariableDecl& node) {
    // Check if variable already exists in current scope
    if (currentScope->existsInCurrentScope(node.name)) {
        error("Variable already declared in current scope: " + node.name, node.line, node.column);
        return;
    }
    
    // Type check initializer if present
    if (node.initializer) {
        std::string initType = getExpressionType(*node.initializer);
        std::string value = node.type.has_value() ? node.type.value() : "void";
        if (!value.empty() && !TypeChecker::isCompatibleType(value, initType)) {
            error("Type mismatch in variable declaration: expected " + value + ", got " + initType, 
                  node.line, node.column);
            return;
        }
    }
    
    // Define variable in current scope
    std::string varType = node.type->empty() ? currentExpressionType : node.type.value();
    currentScope->define(node.name, varType, node.isConstant, false, node.line, node.column);
}

void Analyzer::visit(FunctionDecl& node) {
    // Check if function already exists in current scope
    if (currentScope->existsInCurrentScope(node.name)) {
        error("Function already declared: " + node.name, node.line, node.column);
        return;
    }
    
    // Define function in current scope
    currentScope->define(node.name, node.returnType.value(), false, true, node.line, node.column);
    
    // Enter function scope
    enterScope();
    
    // Define parameters
    for (const auto& param : node.parameters) {
        currentScope->define(param.name, param.type, false, false, node.line, node.column);
    }
    
    // Set current function return type
    std::string oldReturnType = currentFunctionReturnType;
    currentFunctionReturnType = node.returnType.value();
    
    // Analyze function body
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Restore previous function return type
    currentFunctionReturnType = oldReturnType;
    
    // Exit function scope
    exitScope();
}

void Analyzer::visit(ExternFunctionDecl& node) {
    // Check if function name already exists in current scope
    if (currentScope->lookup(node.name)) {
        error("Function '" + node.name + "' is already declared", node.line, node.column);
        return;
    }
    
    // Validate parameter types
    for (const auto& param : node.parameters) {
        if (!TypeChecker::isPrimitiveType(param.type) && !TypeChecker::isPointerType(param.type)) {
            error("Invalid parameter type '" + param.type + "' in extern function '" + node.name + "'", node.line, node.column);
        }
    }
    
    // Validate return type
    if (!TypeChecker::isPrimitiveType(node.returnType) && !TypeChecker::isPointerType(node.returnType) && node.returnType != "void") {
        error("Invalid return type '" + node.returnType + "' in extern function '" + node.name + "'", node.line, node.column);
    }
    
    // Register extern function in symbol table
    // External functions are marked as functions and constant (cannot be redefined)
    currentScope->define(node.name, node.returnType, true, true, node.line, node.column);
}

/******************************
* Visitor - Statement
******************************/

void Analyzer::visit(BlockStmt& node) {
    enterScope();
    
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    
    exitScope();
}

void Analyzer::visit(IfStmt& node) {
    std::string conditionType = getExpressionType(*node.condition);
    // The if condition can be of boolean type or any value.
    // Integer and pointer types are evaluated as boolean (0 = false, other = true)
    if (!TypeChecker::isBooleanType(conditionType) && !TypeChecker::isNumericType(conditionType) && !TypeChecker::isPointerType(conditionType)) {
        error("If condition must be boolean, numeric or pointer type", node.line, node.column);
    }
    
    if (node.thenBranch) {
        node.thenBranch->accept(*this);
    }
    
    if (node.elseBranch) {
        node.elseBranch->accept(*this);
    }
}

void Analyzer::visit(WhileStmt& node) {
    std::string conditionType = getExpressionType(*node.condition);
    // While condition can be of boolean type or any value
    // Integer and pointer types are evaluated as boolean (0 = false, other = true)
    if (!TypeChecker::isBooleanType(conditionType) && !TypeChecker::isNumericType(conditionType) && !TypeChecker::isPointerType(conditionType)) {
        error("While condition must be boolean, numeric or pointer type", node.line, node.column);
    }
    
    if (node.body) {
        node.body->accept(*this);
    }
}

void Analyzer::visit(ForStmt& node) {
    enterScope();
    
    // Analyze initializer if present
    if (node.initializer) {
        node.initializer->accept(*this);
    }
    
    // Analyze condition if present
    if (node.condition) {
        std::string conditionType = getExpressionType(*node.condition);
        if (conditionType != "bool") {
            error("For loop condition must be boolean, got: " + conditionType, 
                  node.line, node.column);
        }
    }
    
    // Analyze increment if present
    if (node.increment) {
        getExpressionType(*node.increment);
    }
    
    // Analyze body
    node.body->accept(*this);
    
    exitScope();
}

void Analyzer::visit(ReturnStmt& node) {
    if (currentFunctionReturnType.empty()) {
        error("Return statement outside of function", node.line, node.column);
        return;
    }
    
    if (node.value) {
        std::string returnType = getExpressionType(*node.value);
        if (!TypeChecker::isCompatibleType(currentFunctionReturnType, returnType)) {
            error("Return type mismatch: expected " + currentFunctionReturnType + ", got " + returnType, 
                  node.line, node.column);
        }
    } else if (currentFunctionReturnType != "void") {
        error("Function must return a value", node.line, node.column);
    }
}

void Analyzer::visit(ExpressionStmt& node) {
    node.expression->accept(*this);
}

/***************************************
*  AST error/warning reporting
***************************************/

void Analyzer::error(const std::string& message, size_t line, size_t column) {
    hasErrors = true;
    std::cerr << "Semantic Error [" << line << ":" << column << "]: " << message << std::endl;
}

void Analyzer::warning(const std::string& message, size_t line, size_t column) {
    std::cerr << "Semantic Warning [" << line << ":" << column << "]: " << message << std::endl;
}

} // namespace emlang

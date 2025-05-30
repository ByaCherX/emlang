#pragma once

#include "ast.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace emlang {

// Symbol bilgisi
struct Symbol {
    std::string name;
    std::string type;
    bool isConstant;
    bool isFunction;
    size_t line;
    size_t column;
    
    Symbol(const std::string& name, const std::string& type, bool isConst = false, bool isFunc = false, size_t line = 0, size_t column = 0)
        : name(name), type(type), isConstant(isConst), isFunction(isFunc), line(line), column(column) {}
};

// Symbol table scope
class Scope {
private:
    std::map<std::string, std::unique_ptr<Symbol>> symbols;
    Scope* parent;
    
public:
    explicit Scope(Scope* parent = nullptr);
      // Symbol operations
    bool define(const std::string& name, const std::string& type, bool isConst = false, bool isFunc = false, size_t line = 0, size_t column = 0);
    Symbol* lookup(const std::string& name);
    bool exists(const std::string& name);
    bool existsInCurrentScope(const std::string& name);
    
    // Scope operations
    Scope* getParent() const;
};

// Semantic Analyzer sınıfı
class SemanticAnalyzer : public ASTVisitor {
private:
    std::vector<std::unique_ptr<Scope>> scopes;
    Scope* currentScope;
    std::string currentFunctionReturnType;
    bool hasErrors;
    
    // Type checking
    std::string getExpressionType(Expression& expr);
    bool isCompatibleType(const std::string& expected, const std::string& actual);
    bool isNumericType(const std::string& type);
    bool isBooleanType(const std::string& type);
    bool isStringType(const std::string& type);
    
    // Type checking helpers
    bool isSignedInteger(const std::string& type);
    bool isUnsignedInteger(const std::string& type);
    bool isFloatingPoint(const std::string& type);
    bool isCharType(const std::string& type);
    bool isUnitType(const std::string& type);
    bool isPrimitiveType(const std::string& type);
    bool canImplicitlyConvert(const std::string& from, const std::string& to);
    std::string getCommonType(const std::string& type1, const std::string& type2);
    
    // Scope management
    void enterScope();
    void exitScope();
    
    // Error reporting
    void error(const std::string& message, size_t line = 0, size_t column = 0);
    void warning(const std::string& message, size_t line = 0, size_t column = 0);
    
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer() = default;
    
    // Analyze AST
    bool analyze(Program& program);
    bool hasSemanticErrors() const;
    
    // AST Visitor methods
    void visit(LiteralExpression& node) override;
    void visit(IdentifierExpression& node) override;
    void visit(BinaryOpExpression& node) override;
    void visit(UnaryOpExpression& node) override;
    void visit(FunctionCallExpression& node) override;
    void visit(VariableDeclaration& node) override;
    void visit(FunctionDeclaration& node) override;
    void visit(BlockStatement& node) override;
    void visit(IfStatement& node) override;
    void visit(WhileStatement& node) override;
    void visit(ReturnStatement& node) override;
    void visit(ExpressionStatement& node) override;
    void visit(Program& node) override;
    
private:
    // Current expression type (used during type checking)
    std::string currentExpressionType;
};

} // namespace emlang

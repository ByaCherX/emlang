#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace emlang {

// Forward declarations
class ASTNode;
class Expression;
class Statement;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;

// AST Node türleri
enum class ASTNodeType {
    // Expressions
    LITERAL,
    IDENTIFIER,
    BINARY_OP,
    UNARY_OP,
    FUNCTION_CALL,
    ARRAY_ACCESS,
    
    // Statements
    EXPRESSION_STMT,
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BLOCK_STATEMENT,
    
    // Program
    PROGRAM
};

// Base AST Node
class ASTNode {
public:
    ASTNodeType type;
    size_t line;
    size_t column;
    
    ASTNode(ASTNodeType type, size_t line = 0, size_t column = 0);
    virtual ~ASTNode() = default;
    
    virtual std::string toString() const = 0;
    virtual void accept(class ASTVisitor& visitor) = 0;
};

// Expression base class
class Expression : public ASTNode {
public:
    Expression(ASTNodeType type, size_t line = 0, size_t column = 0);
};

// Statement base class
class Statement : public ASTNode {
public:
    Statement(ASTNodeType type, size_t line = 0, size_t column = 0);
};

// Literal Expression
class LiteralExpression : public Expression {
public:
    enum class LiteralType {
        NUMBER,
        STRING,
        BOOLEAN,
        NULL_LITERAL
    };
    
    LiteralType literalType;
    std::string value;
    
    LiteralExpression(LiteralType type, const std::string& value, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Identifier Expression
class IdentifierExpression : public Expression {
public:
    std::string name;
    
    IdentifierExpression(const std::string& name, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Binary Operation Expression
class BinaryOpExpression : public Expression {
public:
    ExpressionPtr left;
    std::string operator_;
    ExpressionPtr right;
    
    BinaryOpExpression(ExpressionPtr left, const std::string& op, ExpressionPtr right, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Unary Operation Expression
class UnaryOpExpression : public Expression {
public:
    std::string operator_;
    ExpressionPtr operand;
    
    UnaryOpExpression(const std::string& op, ExpressionPtr operand, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Function Call Expression
class FunctionCallExpression : public Expression {
public:
    std::string functionName;
    std::vector<ExpressionPtr> arguments;
    
    FunctionCallExpression(const std::string& name, std::vector<ExpressionPtr> args, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Variable Declaration Statement
class VariableDeclaration : public Statement {
public:
    std::string name;
    std::string type;
    ExpressionPtr initializer;
    bool isConstant;
    
    VariableDeclaration(const std::string& name, const std::string& type, ExpressionPtr init, bool isConst, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Function Declaration Statement
class FunctionDeclaration : public Statement {
public:
    struct Parameter {
        std::string name;
        std::string type;
    };
    
    std::string name;
    std::vector<Parameter> parameters;
    std::string returnType;
    StatementPtr body;
    
    FunctionDeclaration(const std::string& name, std::vector<Parameter> params, const std::string& retType, StatementPtr body, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Block Statement
class BlockStatement : public Statement {
public:
    std::vector<StatementPtr> statements;
    
    BlockStatement(std::vector<StatementPtr> stmts, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// If Statement
class IfStatement : public Statement {
public:
    ExpressionPtr condition;
    StatementPtr thenBranch;
    StatementPtr elseBranch;
    
    IfStatement(ExpressionPtr cond, StatementPtr then, StatementPtr else_, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// While Statement
class WhileStatement : public Statement {
public:
    ExpressionPtr condition;
    StatementPtr body;
    
    WhileStatement(ExpressionPtr cond, StatementPtr body, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Return Statement
class ReturnStatement : public Statement {
public:
    ExpressionPtr value;
    
    ReturnStatement(ExpressionPtr val, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Expression Statement
class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;
    
    ExpressionStatement(ExpressionPtr expr, size_t line = 0, size_t column = 0);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Program (root node)
class Program : public ASTNode {
public:
    std::vector<StatementPtr> statements;
    
    Program(std::vector<StatementPtr> stmts);
    
    std::string toString() const override;
    void accept(ASTVisitor& visitor) override;
};

// Visitor pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(LiteralExpression& node) = 0;
    virtual void visit(IdentifierExpression& node) = 0;
    virtual void visit(BinaryOpExpression& node) = 0;
    virtual void visit(UnaryOpExpression& node) = 0;
    virtual void visit(FunctionCallExpression& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(FunctionDeclaration& node) = 0;
    virtual void visit(BlockStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(Program& node) = 0;
};

} // namespace emlang

#pragma once

#include "ast.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <map>
#include <memory>

namespace emlang {

// Optimization levels
enum class OptimizationLevel {
    None = 0,
    O1 = 1,
    O2 = 2,
    O3 = 3
};

// LLVM Code Generator sınıfı
class CodeGenerator : public ASTVisitor {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
    // Symbol table for variables and functions
    std::map<std::string, llvm::Value*> namedValues;
    std::map<std::string, llvm::Function*> functions;
    
    // Current function being compiled
    llvm::Function* currentFunction;
    
    // Optimization level
    OptimizationLevel optimizationLevel;
    
    // Type mapping
    llvm::Type* getLLVMType(const std::string& typeName);
    llvm::Type* getNumberType();
    llvm::Type* getStringType();
    llvm::Type* getBooleanType();
      // C-style primitive integer types
    llvm::Type* getInt8Type();
    llvm::Type* getInt16Type();
    llvm::Type* getInt32Type();
    llvm::Type* getInt64Type();
    llvm::Type* getIsizeType();
    llvm::Type* getUint8Type();
    llvm::Type* getUint16Type();
    llvm::Type* getUint32Type();
    llvm::Type* getUint64Type();
    llvm::Type* getUsizeType();
    
    // C-style primitive float types
    llvm::Type* getFloatType();
    llvm::Type* getDoubleType();    
    
    // Other primitive types
    llvm::Type* getCharType();
    llvm::Type* getStrType();
    llvm::Type* getUnitType();
    
    // Helper methods for type checking
    bool isSignedInteger(const std::string& typeName);
    bool isUnsignedInteger(const std::string& typeName);
    bool isFloatingPoint(const std::string& typeName);
    bool isPrimitiveType(const std::string& typeName);
    
    // Helper methods
    llvm::Value* createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type);
    
    // Optimization passes
    void runOptimizationPasses();
    
public:
    CodeGenerator(const std::string& moduleName, OptimizationLevel optLevel = OptimizationLevel::None);
    ~CodeGenerator() = default;
    
    // Generate LLVM IR from AST
    void generateIR(Program& program);
    
    // Get generated module
    llvm::Module* getModule() const;
    
    // Print generated IR
    void printIR() const;
      // Write IR to file
    void writeIRToFile(const std::string& filename) const;
    
    // Write object file
    void writeObjectFile(const std::string& filename) const;
    
    // JIT compile and execute
    int executeMain();
    
    // Set optimization level
    void setOptimizationLevel(OptimizationLevel level);
    
    // Get optimization level
    OptimizationLevel getOptimizationLevel() const;
    
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
    // Current value being processed (result of expression evaluation)
    llvm::Value* currentValue;
    
    // Error handling
    void error(const std::string& message) const;
};

} // namespace emlang

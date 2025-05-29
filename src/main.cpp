#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace emlang;

// Basit bir AST yazdırıcı visitor
class ASTPrinter : public ASTVisitor {
private:
    int indent = 0;
    
    void printIndent() {
        for (int i = 0; i < indent; ++i) {
            std::cout << "  ";
        }
    }
    
public:
    void visit(LiteralExpression& node) override {
        printIndent();
        std::cout << "LiteralExpression: " << node.value << std::endl;
    }
    
    void visit(IdentifierExpression& node) override {
        printIndent();
        std::cout << "IdentifierExpression: " << node.name << std::endl;
    }
    
    void visit(BinaryOpExpression& node) override {
        printIndent();
        std::cout << "BinaryOpExpression: " << node.operator_ << std::endl;
        indent++;
        node.left->accept(*this);
        node.right->accept(*this);
        indent--;
    }
    
    void visit(UnaryOpExpression& node) override {
        printIndent();
        std::cout << "UnaryOpExpression: " << node.operator_ << std::endl;
        indent++;
        node.operand->accept(*this);
        indent--;
    }
    
    void visit(FunctionCallExpression& node) override {
        printIndent();
        std::cout << "FunctionCallExpression: " << node.functionName << std::endl;
        indent++;
        for (auto& arg : node.arguments) {
            arg->accept(*this);
        }
        indent--;
    }
    
    void visit(VariableDeclaration& node) override {
        printIndent();
        std::cout << "VariableDeclaration: " << (node.isConstant ? "const " : "let ") 
                  << node.name << ": " << node.type << std::endl;
        if (node.initializer) {
            indent++;
            node.initializer->accept(*this);
            indent--;
        }
    }
    
    void visit(FunctionDeclaration& node) override {
        printIndent();
        std::cout << "FunctionDeclaration: " << node.name << std::endl;
        indent++;
        for (auto& param : node.parameters) {
            printIndent();
            std::cout << "Parameter: " << param.name << ": " << param.type << std::endl;
        }
        if (node.body) {
            node.body->accept(*this);
        }
        indent--;
    }
    
    void visit(BlockStatement& node) override {
        printIndent();
        std::cout << "BlockStatement:" << std::endl;
        indent++;
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
        indent--;
    }
    
    void visit(IfStatement& node) override {
        printIndent();
        std::cout << "IfStatement:" << std::endl;
        indent++;
        std::cout << "Condition:" << std::endl;
        node.condition->accept(*this);
        std::cout << "Then:" << std::endl;
        node.thenBranch->accept(*this);
        if (node.elseBranch) {
            std::cout << "Else:" << std::endl;
            node.elseBranch->accept(*this);
        }
        indent--;
    }
    
    void visit(WhileStatement& node) override {
        printIndent();
        std::cout << "WhileStatement:" << std::endl;
        indent++;
        std::cout << "Condition:" << std::endl;
        node.condition->accept(*this);
        std::cout << "Body:" << std::endl;
        node.body->accept(*this);
        indent--;
    }
    
    void visit(ReturnStatement& node) override {
        printIndent();
        std::cout << "ReturnStatement:" << std::endl;
        if (node.value) {
            indent++;
            node.value->accept(*this);
            indent--;
        }
    }
    
    void visit(ExpressionStatement& node) override {
        printIndent();
        std::cout << "ExpressionStatement:" << std::endl;
        indent++;
        node.expression->accept(*this);
        indent--;
    }
    
    void visit(Program& node) override {
        std::cout << "Program:" << std::endl;
        indent++;
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
        indent--;
    }
};

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "=== TOKENS ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "EMLang Compiler v1.0.0" << std::endl;
    std::cout << "========================" << std::endl;
    
    if (argc > 1) {
        try {
            // Dosyadan kaynak kodu oku
            std::string source = readFile(argv[1]);
            std::cout << "Compiling file: " << argv[1] << std::endl << std::endl;
            
            // Lexical analysis
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            printTokens(tokens);
            
            // Syntax analysis
            Parser parser(tokens);
            auto ast = parser.parse();
              if (ast) {
                std::cout << "=== AST ===" << std::endl;
                ASTPrinter printer;
                ast->accept(printer);
                std::cout << std::endl;
                
                // Semantic analysis
                std::cout << "=== SEMANTIC ANALYSIS ===" << std::endl;
                SemanticAnalyzer analyzer;
                bool semanticSuccess = analyzer.analyze(*ast);
                  if (semanticSuccess) {
                    std::cout << "Semantic analysis successful!" << std::endl;
                    
                    // Code generation (LLVM)
                    std::cout << "=== CODE GENERATION ===" << std::endl;
                    CodeGenerator codegen("test_module");
                    codegen.generateIR(*ast);
                    codegen.printIR();
                    
                    std::cout << "Compilation successful!" << std::endl;
                } else {
                    std::cout << "Semantic analysis failed!" << std::endl;
                    return 1;
                }
            } else {
                std::cout << "Compilation failed!" << std::endl;
                return 1;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    } else {        // İnteraktif mod için basit bir örnek
        std::string source = R"(
            let x: number = 42;
            let message: string = "Hello, World!";
            
            function add(a: number, b: number): number {
                return a + b;
            }
            
            let result: number = add(x, 10);
            
            if (result > 50) {
                let newMessage: string = "Result is greater than 50";
            }
        )";
        
        std::cout << "Running test program..." << std::endl << std::endl;
        
        try {
            // Lexical analysis
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            printTokens(tokens);
            
            // Syntax analysis
            Parser parser(tokens);
            auto ast = parser.parse();
              if (ast) {
                std::cout << "=== AST ===" << std::endl;
                ASTPrinter printer;
                ast->accept(printer);
                std::cout << std::endl;
                
                // Semantic analysis
                std::cout << "=== SEMANTIC ANALYSIS ===" << std::endl;
                SemanticAnalyzer analyzer;
                bool semanticSuccess = analyzer.analyze(*ast);
                  if (semanticSuccess) {
                    std::cout << "Semantic analysis successful!" << std::endl;
                    
                    // Code generation (LLVM)
                    std::cout << "=== CODE GENERATION ===" << std::endl;
                    CodeGenerator codegen("test_module");
                    codegen.generateIR(*ast);
                    codegen.printIR();
                    
                    std::cout << "Test compilation successful!" << std::endl;
                } else {
                    std::cout << "Test semantic analysis failed!" << std::endl;
                    return 1;
                }
            } else {
                std::cout << "Test compilation failed!" << std::endl;
                return 1;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        
        std::cout << std::endl << "Usage: " << argv[0] << " <source_file>" << std::endl;
    }
    
    return 0;
}

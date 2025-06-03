#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace emlang;

// AST writer visitor class
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
    
    void visit(DereferenceExpression& node) override {
        printIndent();
        std::cout << "DereferenceExpression: *" << std::endl;
        indent++;
        node.operand->accept(*this);
        indent--;
    }
    
    void visit(AddressOfExpression& node) override {
        printIndent();
        std::cout << "AddressOfExpression: &" << std::endl;
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
    
    void visit(ExternFunctionDeclaration& node) override {
        printIndent();
        std::cout << "ExternFunctionDeclaration: " << node.name << std::endl;
        indent++;
        for (auto& param : node.parameters) {
            printIndent();
            std::cout << "Parameter: " << param.name << ": " << param.type << std::endl;
        }
        if (!node.returnType.empty()) {
            printIndent();
            std::cout << "ReturnType: " << node.returnType << std::endl;
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

// Token printer function
void printTokens(const std::vector<Token>& tokens) {
    std::cout << "=== TOKENS ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << std::endl;
}

// File reading utility function
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Kullanım bilgilerini yazdırma
void printUsage(const char* programName) {
    std::cout << "USAGE: " << programName << "[options] <source_file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --tokens               Show lexer tokens" << std::endl;
    std::cout << "  --ast                  Show AST structure" << std::endl;
    std::cout << "  --all                  Show both tokens and AST" << std::endl;
    std::cout << "  -h, --help             Show this help message" << std::endl;
}

// Komut satırı argümanları için yapı
struct CheckOptions {
    std::string inputFile;
    bool showTokens = false;
    bool showAST = false;
    bool showHelp = false;
};

// Argüman parse etme
CheckOptions parseArguments(int argc, char* argv[]) {
    CheckOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
        } else if (arg == "--tokens") {
            options.showTokens = true;
        } else if (arg == "--ast") {
            options.showAST = true;
        } else if (arg == "--all") {
            options.showTokens = true;
            options.showAST = true;
        } else if (arg.substr(0, 1) == "-") {
            throw std::runtime_error("Unknown option: " + arg);
        } else {
            if (options.inputFile.empty()) {
                options.inputFile = arg;
            } else {
                throw std::runtime_error("Multiple input files not supported");
            }
        }
    }
    
    return options;
}

int main(int argc, char* argv[]) {    
    try {
        CheckOptions options = parseArguments(argc, argv);
        
        if (options.showHelp || argc == 1) {
            printUsage(argv[0]);
            return 0;
        }
        
        if (options.inputFile.empty()) {
            std::cerr << "Error: No input file specified" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // If not showing tokens or AST, default to showing both
        if (!options.showTokens && !options.showAST) {
            options.showTokens = true;
            options.showAST = true;
        }
        
        std::cout << "Analyzing: " << options.inputFile << std::endl;
        std::cout << std::endl;
        
        // Read source file
        std::string source = readFile(options.inputFile);
        
        // Lexical Analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        if (options.showTokens) {
            printTokens(tokens);
        }
        
        // Syntax Anylysis
        if (options.showAST) {
            Parser parser(tokens);
            auto ast = parser.parse();
            
            if (!ast) {
                std::cerr << "Parsing failed: Syntax errors detected" << std::endl;
                return 1;
            }
            
            std::cout << "=== AST ===" << std::endl;
            ASTPrinter printer;
            ast->accept(printer);
            std::cout << std::endl;
        }
        
        std::cout << "Analysis completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

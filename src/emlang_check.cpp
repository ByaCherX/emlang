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
    void visit(LiteralExpr& node) override {
        printIndent();
        std::cout << "LiteralExpr: " << node.value << std::endl;
    }
    
    void visit(IdentifierExpr& node) override {
        printIndent();
        std::cout << "IdentifierExpr: " << node.name << std::endl;
    }
    
    void visit(BinaryOpExpr& node) override {
        printIndent();
        std::cout << "BinaryOpExpr: " << node.operator_ << std::endl;
        indent++;
        node.left->accept(*this);
        node.right->accept(*this);
        indent--;
    }
    
    void visit(UnaryOpExpr& node) override {
        printIndent();
        std::cout << "UnaryOpExpr: " << node.operator_ << std::endl;
        indent++;
        node.operand->accept(*this);
        indent--;
    }
    
    void visit(DereferenceExpr& node) override {
        printIndent();
        std::cout << "DereferenceExpr: *" << std::endl;
        indent++;
        node.operand->accept(*this);
        indent--;
    }
    
    void visit(AddressOfExpr& node) override {
        printIndent();
        std::cout << "AddressOfExpr: &" << std::endl;
        indent++;
        node.operand->accept(*this);
        indent--;
    }

    void visit(AssignmentExpr& node) override {
        printIndent();
        std::cout << "AssignmentExpr: =" << std::endl;
        indent++;
        node.target->accept(*this);
        node.value->accept(*this);
        indent--;
    }
    
    void visit(FunctionCallExpr& node) override {
        printIndent();
        std::cout << "FunctionCallExpr: " << node.functionName << std::endl;
        indent++;
        for (auto& arg : node.arguments) {
            arg->accept(*this);
        }
        indent--;
    }
    
    void visit(VariableDecl& node) override {
        printIndent();
        std::cout << "VariableDecl: " << (node.isConstant ? "const " : "let ") 
                  << node.name << ": " << node.type << std::endl;
        if (node.initializer) {
            indent++;
            node.initializer->accept(*this);
            indent--;
        }
    }
    
    void visit(FunctionDecl& node) override {
        printIndent();
        std::cout << "FunctionDecl: " << node.name << std::endl;
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
    
    void visit(ExternFunctionDecl& node) override {
        printIndent();
        std::cout << "ExternFunctionDecl: " << node.name << std::endl;
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
    
    void visit(BlockStmt& node) override {
        printIndent();
        std::cout << "BlockStmt:" << std::endl;
        indent++;
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
        indent--;
    }
    
    void visit(IfStmt& node) override {
        printIndent();
        std::cout << "IfStmt:" << std::endl;
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
    
    void visit(WhileStmt& node) override {
        printIndent();
        std::cout << "WhileStmt:" << std::endl;
        indent++;
        std::cout << "Condition:" << std::endl;
        node.condition->accept(*this);
        std::cout << "Body:" << std::endl;
        node.body->accept(*this);
        indent--;
    }
    
    void visit(ReturnStmt& node) override {
        printIndent();
        std::cout << "ReturnStmt:" << std::endl;
        if (node.value) {
            indent++;
            node.value->accept(*this);
            indent--;
        }
    }
    
    void visit(ExpressionStmt& node) override {
        printIndent();
        std::cout << "ExpressionStmt:" << std::endl;
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
static void printTokens(const std::vector<Token>& tokens) {
    std::cout << "=== TOKENS ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
    std::cout << std::endl;
}

// File reading utility function
static std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Print usage information
static void printUsage(const char* programName) {
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

// Argument parse
static CheckOptions parseArguments(int argc, char* argv[]) {
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

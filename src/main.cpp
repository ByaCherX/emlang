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

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <source_file> [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o, --output <file>     Specify output file name" << std::endl;
    std::cout << "  -O1                     Enable basic optimizations" << std::endl;
    std::cout << "  -O2                     Enable more optimizations" << std::endl;
    std::cout << "  -O3                     Enable aggressive optimizations" << std::endl;
    std::cout << "  --emit-llvm             Output LLVM IR instead of object file" << std::endl;
    std::cout << "  --debug                 Enable debug output" << std::endl;
    std::cout << "  -h, --help              Show this help message" << std::endl;
}

struct CompilerOptions {
    std::string inputFile;
    std::string outputFile;
    int optimizationLevel = 0;
    bool emitLLVM = false;
    bool debug = false;
    bool showHelp = false;
};

CompilerOptions parseArguments(int argc, char* argv[]) {
    CompilerOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                options.outputFile = argv[++i];
            } else {
                throw std::runtime_error("Option " + arg + " requires an argument");
            }
        } else if (arg == "-O1") {
            options.optimizationLevel = 1;
        } else if (arg == "-O2") {
            options.optimizationLevel = 2;
        } else if (arg == "-O3") {
            options.optimizationLevel = 3;
        } else if (arg == "--emit-llvm") {
            options.emitLLVM = true;
        } else if (arg == "--debug") {
            options.debug = true;
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
    std::cout << "EMLang Compiler v1.0.0" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        CompilerOptions options = parseArguments(argc, argv);
        
        if (options.showHelp || argc == 1) {
            printUsage(argv[0]);
            return 0;
        }
        
        if (options.inputFile.empty()) {
            std::cerr << "Error: No input file specified" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        // Determine output file name if not specified
        if (options.outputFile.empty()) {
            if (options.emitLLVM) {
                options.outputFile = options.inputFile.substr(0, options.inputFile.find_last_of('.')) + ".ll";
            } else {
                options.outputFile = options.inputFile.substr(0, options.inputFile.find_last_of('.')) + ".o";
            }
        }
        
        std::cout << "Compiling: " << options.inputFile << std::endl;
        std::cout << "Output: " << options.outputFile << std::endl;
        if (options.optimizationLevel > 0) {
            std::cout << "Optimization Level: O" << options.optimizationLevel << std::endl;
        }
        std::cout << std::endl;
        
        // Read source file
        std::string source = readFile(options.inputFile);
        
        // Lexical analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        if (options.debug) {
            printTokens(tokens);
        }
        
        // Syntax analysis
        Parser parser(tokens);
        auto ast = parser.parse();
        
        if (!ast) {
            std::cerr << "Compilation failed: Syntax errors detected" << std::endl;
            return 1;
        }
        
        if (options.debug) {
            std::cout << "=== AST ===" << std::endl;
            ASTPrinter printer;
            ast->accept(printer);
            std::cout << std::endl;
        }
        
        // Semantic analysis
        if (options.debug) {
            std::cout << "=== SEMANTIC ANALYSIS ===" << std::endl;
        }
        
        SemanticAnalyzer analyzer;
        bool semanticSuccess = analyzer.analyze(*ast);
        
        if (!semanticSuccess) {
            std::cerr << "Compilation failed: Semantic errors detected" << std::endl;
            return 1;
        }
        
        if (options.debug) {
            std::cout << "Semantic analysis successful!" << std::endl;
        }
          // Code generation
        if (options.debug) {
            std::cout << "=== CODE GENERATION ===" << std::endl;
        }
        
        // Convert optimization level
        OptimizationLevel optLevel = static_cast<OptimizationLevel>(options.optimizationLevel);
        CodeGenerator codegen("emlang_module", optLevel);
        codegen.generateIR(*ast);
        
        if (options.debug) {
            codegen.printIR();
        }
          // Output generation
        if (options.emitLLVM) {
            codegen.writeIRToFile(options.outputFile);
            std::cout << "LLVM IR written to: " << options.outputFile << std::endl;
        } else {
            try {
                codegen.writeObjectFile(options.outputFile);
                std::cout << "Object file written to: " << options.outputFile << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Warning: Object file generation failed: " << e.what() << std::endl;
                std::cout << "Falling back to LLVM IR output..." << std::endl;
                
                // Fallback to LLVM IR
                std::string llvmFile = options.inputFile.substr(0, options.inputFile.find_last_of('.')) + ".ll";
                codegen.writeIRToFile(llvmFile);
                std::cout << "LLVM IR written to: " << llvmFile << std::endl;
            }
        }
        
        std::cout << "Compilation successful!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

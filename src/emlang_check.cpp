#include "lexer.h"
#include "parser/parser.h"
#include "ast.h"
#include "ast/dumper.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace emlang;

// Token printer function
static void printTokens(const std::vector<Token>& tokens) {
    std::cout << "=== TOKENS ===" << std::endl;
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl << std::flush;
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
        emlang::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        if (options.showTokens) {
            //printTokens(tokens);
            std::cout << "Currently showing tokens is disabled. (token printing throwing error)" << std::endl;
        }
        
        // Parsing
        if (options.showAST) {
            emlang::Parser parser(tokens);
            auto ast = parser.parse();
            
            if (!ast) {
                std::cerr << "Parsing failed: Syntax errors detected" << std::endl;
                return 1;
            }
            
            std::cout << "=== AST ===" << std::endl;
            emlang::ASTDumper dumper;
            ast->accept(dumper);
            std::cout << std::endl;
        }
        
        std::cout << "Analysis completed successfully!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

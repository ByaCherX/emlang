//===--- main.cpp - Compiler main entry implementation --------------------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main Codegenerator Impl
//===----------------------------------------------------------------------===//

#include "lexer.h"
#include "ast.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"  //fix: semantic.h will rename when redesigned
#include "codegen/codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace emlang;

#define DEBUG_MODE 0

static std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void printUsage(const char* programName) {
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
    bool emitLLVM = false;
    bool debug = false;
    bool showHelp = false;
};

static CompilerOptions parseArguments(int argc, char* argv[]) {
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
            
        } else if (arg == "-O2") {
            
        } else if (arg == "-O3") {
            
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
    std::cout << "EMLang Compiler" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        CompilerOptions options = parseArguments(argc, argv);
#if(DEBUG_MODE)
        std::cout << "<<< Debugger Enabled >>>" << std::endl;
        const std::string path = "C:\\Users\\kralk\\Documents\\GitHub\\emlang\\";
        options.inputFile = path + "tests\\emlang_library_test.em";
        options.outputFile = path + "emlang_output.o";
        options.showHelp = false;
        argc++;
#endif
        
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
        
        // Read source file
        std::string source = readFile(options.inputFile);
        
        // Lexical analysis
        emlang::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Parse the tokens
        emlang::Parser parser(tokens);
        auto ast = parser.parse();
        
        if (!ast) {
            std::cerr << "Compilation failed: Syntax errors detected" << std::endl;
            return 1;
        }
        
        // Semantic analysis
        if (options.debug) {
            std::cout << "=== SEMANTIC ANALYSIS ===" << std::endl;
        }
        
        emlang::Analyzer analyzer;
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
        
        emlang::codegen::CodeGenerator codegen("emlang_module");
        codegen.generateIR(*ast);
        
        if (options.debug) {
            codegen.printIR();
        }
          
        // Output generation
        if (options.emitLLVM) {
            codegen.compileAOT(options.outputFile);
            std::cout << "LLVM IR written to: " << options.outputFile << std::endl;
        } else {
            try {
                codegen.compileAOT(options.outputFile);
                std::cout << "Object file written to: " << options.outputFile << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Warning: Object file generation failed: " << e.what() << std::endl;
                std::cout << "Falling back to LLVM IR output..." << std::endl;
                
                // Fallback to LLVM IR
                std::string llvmFile = options.inputFile.substr(0, options.inputFile.find_last_of('.')) + ".ll";
                codegen.compileAOT(llvmFile);
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

//===--- codegen.cpp - Main CodeGenerator Implementation --------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main code generator implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen.h"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/GlobalVariable.h>

#include <iostream>

namespace emlang {
namespace codegen {

/******************************
* CONSTRUCTION AND LIFECYCLE
******************************/

CodeGenerator::CodeGenerator(const std::string& moduleName, OptimizationLevel optLevel) 
    : currentValue(nullptr) {
    // Initialize modular components
    contextManager = std::make_unique<ContextManager>(moduleName, optLevel);
    valueMap = std::make_unique<ValueMap>();
    errorReporter = std::make_unique<CodegenErrorReporter>();
    
    // Initialize code generation components
    exprGenerator = std::make_unique<CGExpr>(*contextManager, *valueMap, *errorReporter);
    declGenerator = std::make_unique<CGDecl>(*contextManager, *valueMap, *errorReporter);
    stmtGenerator = std::make_unique<CGStmt>(*contextManager, *valueMap, *errorReporter, *exprGenerator);
    
    currentFunction = nullptr;
    currentExpressionType.clear();
}

/******************************
* PRIMARY CODE GENERATION
******************************/

void CodeGenerator::generateIR(Program& program) {
    // TODO: Register built-in functions - needs to be implemented
    // BuiltinsIntegration builtins(*contextManager, *valueMap);
    // builtins.registerBuiltinFunctions();
    
    // Process all statements in the program
    for (auto& stmt : program.statements) {
        // Route to appropriate generator based on AST node type
        if (auto funcDecl = dynamic_cast<FunctionDecl*>(stmt.get())) {
            declGenerator->generateFunctionDecl(*funcDecl);
        }
        else if (auto externDecl = dynamic_cast<ExternFunctionDecl*>(stmt.get())) {
            declGenerator->generateExternFunctionDecl(*externDecl);
        }
        else if (auto varDecl = dynamic_cast<VariableDecl*>(stmt.get())) {
            declGenerator->generateVariableDecl(*varDecl);
        }
        else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt.get())) {
            stmtGenerator->generateBlock(*blockStmt);
        }
        else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
            stmtGenerator->generateIf(*ifStmt);
        }
        else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
            stmtGenerator->generateWhile(*whileStmt);
        }
        else if (auto forStmt = dynamic_cast<ForStmt*>(stmt.get())) {
            stmtGenerator->generateFor(*forStmt);
        }
        else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt.get())) {
            stmtGenerator->generateReturn(*returnStmt);
        }
        else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt.get())) {
            stmtGenerator->generateExpressionStmt(*exprStmt);
        }
        else {
            error(CodegenErrorType::InternalError, "Unsupported statement type in program");
        }
    }
    
    // Run optimization passes if requested
    if (contextManager->getOptimizationLevel() != OptimizationLevel::None) {
        contextManager->runOptimizationPasses();
    }
    
    // Verify the module - simplified approach
    if (llvm::verifyModule(*contextManager->getModule(), &llvm::errs())) {
        error(CodegenErrorType::InternalError, "Module verification failed");
    }
}

void CodeGenerator::printIR() const {
    contextManager->printIR();
}

/******************************
* EXECUTION
******************************/

int CodeGenerator::executeMain() {
    return contextManager->executeMain();
}

void CodeGenerator::writeCodeToFile(const std::string& filename, bool emitLLVM) {
    if (emitLLVM) {
        contextManager->writeIRToFile(filename);
    }
    else {
        contextManager->writeObjectFile(filename);
    }
}

/******************************
* ERROR HANDLING
******************************/
bool CodeGenerator::hasErrors() const {
    return errorReporter->hasErrors();
}

void CodeGenerator::error(const std::string& message) {
    errorReporter->error(CodegenErrorType::InternalError, message);
    currentValue = nullptr;
}

void CodeGenerator::error(CodegenErrorType type, const std::string& message, const std::string& context) {
    errorReporter->error(type, message, context);
    currentValue = nullptr;
}

} // namespace codegen
} // namespace emlang
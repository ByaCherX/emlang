//===--- codegen.cpp - Main CodeGenerator Implementation --------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main code generator implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/codegen.h"
// Backend includes temporarily disabled until implementation is complete
// #include "codegen/aot_compiler.h"
// #include "codegen/jit/jit_engine.h"
#include "ast.h"

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

CodeGenerator::CodeGenerator(const std::string& moduleName) 
    : currentValue(nullptr) {
    // Initialize modular components
    contextManager   = std::make_unique<ContextManager>(moduleName);
    valueMap         = std::make_unique<ValueMap>();
    errorReporter    = std::make_unique<CodegenErrorReporter>();
    
    // Initialize code generation components
    exprGenerator    = std::make_unique<CGExpr>(*contextManager, *valueMap, *errorReporter);
    declGenerator    = std::make_unique<CGDecl>(*contextManager, *valueMap, *errorReporter);
    stmtGenerator    = std::make_unique<CGStmt>(*contextManager, *valueMap, *errorReporter, *exprGenerator);    
    
    // Initialize program orchestrator with specialized visitors
    programGenerator = std::make_unique<CGBase>(
        *contextManager, 
        *valueMap, 
        *errorReporter, 
        exprGenerator.get(), declGenerator.get(), stmtGenerator.get()
    );
    
    // Initialize backends - TODO: Implement backends fully
    // AOT backend temporarily disabled until implementation is complete
    // initializeAOTBackend();
    
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
    
    // Use the program orchestrator to generate IR using visitor pattern
    program.accept(*programGenerator);
    
    // Run optimization passes if requested
    contextManager->runOptimizationPasses();
    
    // Verify the module - simplified approach
    if (llvm::verifyModule(*contextManager->getModule(), &llvm::errs())) {
        errorReporter->error(CodegenErrorType::InternalError, "Module verification failed");
        return;
    }
    
    // Prepare module for backend processing
    if (!prepareModuleForBackend()) {
        errorReporter->error(CodegenErrorType::InternalError, "Failed to prepare module for backend");
    }
}

void CodeGenerator::printIR() const {
    contextManager->printIR();
}

/******************************
* EXECUTION
******************************/

void CodeGenerator::writeCodeToFile(const std::string& filename, bool emitLLVM) {
    if (emitLLVM) {
        contextManager->writeIRToFile(filename);
    }    else {
        // Use fallback to context manager's writeObjectFile
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

/******************************
* BACKEND MANAGEMENT
******************************/

bool CodeGenerator::compileAOT(const std::string& outputPath) {
    // AOT backend temporarily disabled until implementation is complete
    error("AOT backend is currently disabled (implementation in progress)");
    return false;
}

bool CodeGenerator::initializeJIT() {
    // JIT is experimental and disabled for now
    error("JIT backend is currently disabled (experimental feature)");
    return false;
}

/******************************
* BACKEND HELPERS
******************************/

bool CodeGenerator::initializeAOTBackend() {
    // AOT backend temporarily disabled until implementation is complete
    error("AOT backend initialization is currently disabled");
    return false;
}

bool CodeGenerator::prepareModuleForBackend() {
    auto* module = contextManager->getModule();
    if (!module) {
        return false;
    }
    
    // Verify module before backend processing
    if (llvm::verifyModule(*module, &llvm::errs())) {
        error("Module verification failed during backend preparation");
        return false;
    }
    
    // Additional preparation steps could be added here
    // For example: symbol resolution, metadata generation, etc.
    
    return true;
}

} // namespace codegen
} // namespace emlang
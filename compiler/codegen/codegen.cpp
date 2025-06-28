//===--- codegen.cpp - Main CodeGenerator Implementation --------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main code generator implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/codegen.h"
#include "codegen/aot_compiler.h"
// JIT backend still experimental
// #include "codegen/jit/jit_engine.h"
#include "ast.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/GlobalVariable.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <iostream>

namespace emlang {
namespace codegen {

// Helper function to convert LLVM Error to string
std::string toString(llvm::Error err) {
    std::string result;
    llvm::handleAllErrors(std::move(err), [&](const llvm::ErrorInfoBase& info) {
        result = info.message();
    });
    return result;
}

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
    
    // Initialize AOT backend
    aotBackend = std::make_unique<AOTCompiler>();
    initializeAOTBackend();
    
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
    
    // Verify the module - simplified approach
    if (llvm::verifyModule(*contextManager->getModule(), &llvm::errs())) {
        errorReporter->error(CodegenErrorType::InternalError, "Module verification failed");
        return;
    }
}

void CodeGenerator::printIR() const {
    contextManager->printIR();
}

/******************************
* EXECUTION
******************************/
// Currently JIT not implemented

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
    // Ensure AOT backend is initialized and ready
    if (!initializeAOTBackend()) {
        error("Failed to initialize AOT backend");
        return false;
    }
    
    auto* module = contextManager->getModule();
    if (!module) {
        error("No module available for AOT compilation");
        return false;
    }
    
    // Determine output format based on file extension
    OutputFormat format = OutputFormat::Object;
    if (outputPath.size() >= 3 && outputPath.substr(outputPath.size() - 3) == ".ll") {
        format = OutputFormat::LLVM_IR;
    } else if (outputPath.size() >= 3 && outputPath.substr(outputPath.size() - 3) == ".bc") {
        format = OutputFormat::Bitcode;
    } else if ((outputPath.size() >= 2 && outputPath.substr(outputPath.size() - 2) == ".s") || 
               (outputPath.size() >= 4 && outputPath.substr(outputPath.size() - 4) == ".asm")) {
        format = OutputFormat::Assembly;
    } else if ((outputPath.size() >= 4 && outputPath.substr(outputPath.size() - 4) == ".exe") || 
               outputPath.find('.') == std::string::npos) {
        format = OutputFormat::Executable;
    }
    
    // Compile using AOT backend
    if (auto err = this->aotBackend->compileModule(*module, outputPath, format)) {
        error("AOT compilation failed: " + ::emlang::codegen::toString(std::move(err)));
        return false;
    }
    
    return true;
}

/******************************
* BACKEND HELPERS
******************************/

bool CodeGenerator::initializeAOTBackend() {
    if (!this->aotBackend) {
        error("AOT backend not created");
        return false;
    }
    
    if (auto err = this->aotBackend->initialize()) {
        error("AOT backend initialization failed: " + ::emlang::codegen::toString(std::move(err)));
        return false;
    }

    return true;
}

} // namespace codegen
} // namespace emlang
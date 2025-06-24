//===--- context.cpp - Context Management Implementation ------------------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of context management for EMLang code generation
//===----------------------------------------------------------------------===//

#include "codegen/context.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Triple.h>

#include <optional>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <iostream>

namespace emlang {
namespace codegen {

// ======================== CONSTRUCTION ========================

ContextManager::ContextManager(const std::string& moduleName)
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>(moduleName, *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)) {
    
    initializeTargets();
    registerBuiltinFunctions();
}

// ======================== ALLOCA HELPER ========================

llvm::Value* ContextManager::createEntryBlockAlloca(llvm::Function* function, 
                                                       const std::string& varName, 
                                                       llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

// ======================== OPTIMIZATION CONTROL ========================

void ContextManager::runOptimizationPasses() {
    throw std::runtime_error("Optimization passes are not supported in ContextManager.");
}

// ======================== OUTPUT GENERATION ========================

void ContextManager::printIR() const {
    module->print(llvm::outs(), nullptr);
}

void ContextManager::writeIRToFile(const std::string& filename) const {
    std::error_code errorCode;
    llvm::raw_fd_ostream file(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        std::cerr << "Error opening file: " << errorCode.message() << std::endl;
        return;
    }
    
    module->print(file, nullptr);
}

void ContextManager::writeObjectFile(const std::string& filename) const {
    // Initialize the target registry
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    
    auto targetTriple = llvm::Triple::getArchTypeName(llvm::Triple::x86_64); //getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        std::cerr << "Error: " << error << std::endl;
        return;
    }
    
    auto CPU = "generic";    
    auto features = "";
    
    llvm::TargetOptions opt;
    std::optional<llvm::Reloc::Model> RM = std::nullopt;
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    
    module->setDataLayout(targetMachine->createDataLayout());
    
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        std::cerr << "Could not open file: " << errorCode.message() << std::endl;
        return;
    }
    
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
        return;
    }
    
    pass.run(*module);
    dest.flush();
}

// ======================== INITIALIZATION HELPERS ========================

void ContextManager::initializeTargets() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

void ContextManager::registerBuiltinFunctions() {    // Register printf function
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        llvm::PointerType::get(*context, 0),
        true  // varargs
    );
    
    llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );
      // Register puts function
    llvm::FunctionType* putsType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        llvm::PointerType::get(*context, 0),
        false
    );
    
    llvm::Function::Create(
        putsType,
        llvm::Function::ExternalLinkage,
        "puts",
        module.get()
    );
      // Register malloc function
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        llvm::Type::getInt64Ty(*context),
        false
    );
    
    llvm::Function::Create(
        mallocType,
        llvm::Function::ExternalLinkage,
        "malloc",
        module.get()
    );
      // Register free function
    llvm::FunctionType* freeType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context),
        llvm::PointerType::get(*context, 0),
        false
    );
    
    llvm::Function::Create(
        freeType,
        llvm::Function::ExternalLinkage,
        "free",
        module.get()
    );
}

} // namespace codegen
} // namespace emlang

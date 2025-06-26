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

/******************** CONSTRUCTION ********************/

ContextManager::ContextManager(const std::string& moduleName)
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>(moduleName, *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)) {
    
    initializeTargets();
    registerBuiltinFunctions();
}

/******************** ALLOCA HELPER ********************/

llvm::Value* ContextManager::createEntryBlockAlloca(llvm::Function* function, 
                                                       const std::string& varName, 
                                                       llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

/******************** IR HELPER ********************/

void ContextManager::printIR() const {
    module->print(llvm::outs(), nullptr);
}

/******************** INITIALIZATION HELPERS ********************/

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

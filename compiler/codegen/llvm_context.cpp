//===--- llvm_context.cpp - LLVM Context Management Implementation --------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of LLVM context management for EMLang code generation
//===----------------------------------------------------------------------===//

#include "codegen/llvm_context.h"

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
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include <optional>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <iostream>

namespace emlang {
namespace codegen {

// ======================== CONSTRUCTION ========================

LLVMContextManager::LLVMContextManager(const std::string& moduleName, OptimizationLevel optLevel)
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>(moduleName, *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)),
      optimizationLevel(optLevel) {
    
    initializeTargets();
    registerBuiltinFunctions();
}

// ======================== ALLOCA HELPER ========================

llvm::Value* LLVMContextManager::createEntryBlockAlloca(llvm::Function* function, 
                                                       const std::string& varName, 
                                                       llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

// ======================== OPTIMIZATION CONTROL ========================

void LLVMContextManager::setOptimizationLevel(OptimizationLevel level) {
    optimizationLevel = level;
}

OptimizationLevel LLVMContextManager::getOptimizationLevel() const {
    return optimizationLevel;
}

void LLVMContextManager::runOptimizationPasses() {
    if (optimizationLevel == OptimizationLevel::None) {
        return; // No optimizations
    }
    
    // Create a legacy pass manager for function passes
    auto functionPassManager = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());
    
    // Add optimization passes based on level
    switch (optimizationLevel) {
        case OptimizationLevel::O1:
            functionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            break;
              case OptimizationLevel::O2:
            functionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            // Note: GVN and other passes moved to new pass manager in LLVM 20.1.0
            functionPassManager->add(llvm::createCFGSimplificationPass());
            break;
            
        case OptimizationLevel::O3:
            functionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            // Note: GVN and aggressive DCE passes moved to new pass manager
            functionPassManager->add(llvm::createCFGSimplificationPass());
            functionPassManager->add(llvm::createTailCallEliminationPass());
            break;
            
        default:
            break;
    }
    
    functionPassManager->doInitialization();
    
    // Run passes on all functions
    for (auto& function : *module) {
        functionPassManager->run(function);
    }
    
    functionPassManager->doFinalization();
}

// ======================== OUTPUT GENERATION ========================

void LLVMContextManager::printIR() const {
    module->print(llvm::outs(), nullptr);
}

void LLVMContextManager::writeIRToFile(const std::string& filename) const {
    std::error_code errorCode;
    llvm::raw_fd_ostream file(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        std::cerr << "Error opening file: " << errorCode.message() << std::endl;
        return;
    }
    
    module->print(file, nullptr);
}

void LLVMContextManager::writeObjectFile(const std::string& filename) const {
    // Initialize the target registry
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();    
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    auto targetTriple = llvm::Triple::getArchTypeName(llvm::Triple::x86_64); //getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        std::cerr << "Error: " << error << std::endl;
        return;
    }
    
    auto CPU = "generic";    auto features = "";
    
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

int LLVMContextManager::executeMain() {
    // Find main function
    llvm::Function* mainFunction = module->getFunction("main");
    if (!mainFunction) {
        std::cerr << "Error: No main function found" << std::endl;
        return -1;
    }
    
    // Verify the module
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if (llvm::verifyModule(*module, &errorStream)) {
        std::cerr << "Module verification failed: " << error << std::endl;
        return -1;
    }
    
    // Create execution engine
    llvm::EngineBuilder engineBuilder(std::move(module));
    std::string engineError;
    engineBuilder.setErrorStr(&engineError);
    engineBuilder.setEngineKind(llvm::EngineKind::JIT);
    
    auto executionEngine = engineBuilder.create();
    if (!executionEngine) {
        std::cerr << "Failed to create execution engine: " << engineError << std::endl;
        return -1;
    }
    
    // JIT the main function
    executionEngine->finalizeObject();
    
    // Get the main function pointer
    auto mainPtr = (int(*)())executionEngine->getPointerToFunction(mainFunction);
    if (!mainPtr) {
        std::cerr << "Error: Could not get pointer to main function" << std::endl;
        return -1;
    }
    
    // Execute main function
    return mainPtr();
}

// ======================== INITIALIZATION HELPERS ========================

void LLVMContextManager::initializeTargets() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

void LLVMContextManager::registerBuiltinFunctions() {    // Register printf function
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

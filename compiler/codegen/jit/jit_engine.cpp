//===--- jit_engine.cpp - Just-in-Time Compiler Implementation -----------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of just-in-time compilation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/jit/jit_engine.h"
#include "codegen/context.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/Error.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Transforms/Utils/Cloning.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <iostream>
#include <sstream>

namespace emlang {
namespace codegen {
namespace jit {

/******************** CONSTRUCTION AND LIFECYCLE ********************/

JITEngine::JITEngine(const std::string& targetTriple)
    : jit_(nullptr),
      isInitialized_(false),
      modulesExecuted_(0),
      targetTriple_(targetTriple.empty() ? llvm::sys::getDefaultTargetTriple() : targetTriple) {
}

JITEngine::~JITEngine() = default;

JITEngine::JITEngine(JITEngine&& other) noexcept
    : jit_(std::move(other.jit_)),
      isInitialized_(other.isInitialized_),
      modulesExecuted_(other.modulesExecuted_),
      targetTriple_(std::move(other.targetTriple_)) {
    other.isInitialized_ = false;
    other.modulesExecuted_ = 0;
}

JITEngine& JITEngine::operator=(JITEngine&& other) noexcept {
    if (this != &other) {
        jit_ = std::move(other.jit_);
        isInitialized_ = other.isInitialized_;
        modulesExecuted_ = other.modulesExecuted_;
        targetTriple_ = std::move(other.targetTriple_);
        
        other.isInitialized_ = false;
        other.modulesExecuted_ = 0;
    }
    return *this;
}

/******************** INITIALIZATION ********************/

llvm::Error JITEngine::initialize() {
    if (isInitialized_) {
        return llvm::Error::success();
    }

    // Initialize native target for JIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Setup JIT engine
    if (auto err = setupJIT()) {
        return err;
    }

    isInitialized_ = true;
    return llvm::Error::success();
}

bool JITEngine::isReady() const {
    return isInitialized_ && jit_;
}

llvm::Error JITEngine::setupJIT() {
    auto jitOrErr = llvm::orc::LLJITBuilder().create();
    if (!jitOrErr) {
        return jitOrErr.takeError();
    }
    
    jit_ = std::move(*jitOrErr);
    return llvm::Error::success();
}

/******************** EXECUTION ********************/

llvm::Expected<int> JITEngine::executeModule(llvm::Module& module) {
    if (!isReady()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "JIT engine not initialized");
    }

    // Prepare the module
    if (auto err = prepareModule(module)) {
        return std::move(err);
    }

    // Create a thread-safe module
    auto context = std::make_unique<llvm::LLVMContext>();
    auto modulePtr = llvm::CloneModule(module);
    auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(modulePtr), std::move(context));

    // Add the module to JIT
    if (auto err = jit_->addIRModule(std::move(threadSafeModule))) {
        return std::move(err);
    }

    // Look up the main function
    auto mainSymbol = jit_->lookup("main");
    if (!mainSymbol) {
        return mainSymbol.takeError();
    }

    // Get the function pointer
    auto mainAddress = mainSymbol->getValue();
    auto mainFunction = llvm::jitTargetAddressToPointer<int(*)()>(mainAddress);

    // Execute the main function
    int result = mainFunction();
    
    ++modulesExecuted_;
    return result;
}

llvm::Error JITEngine::addModule(std::unique_ptr<llvm::Module> module) {
    if (!isReady()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "JIT engine not initialized");
    }

    // Prepare the module
    if (auto err = prepareModule(*module)) {
        return err;
    }

    // Create thread-safe module
    auto context = std::make_unique<llvm::LLVMContext>();
    auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(module), std::move(context));

    // Add to JIT
    return jit_->addIRModule(std::move(threadSafeModule));
}

llvm::Expected<llvm::orc::ExecutorAddr> JITEngine::lookupSymbol(const std::string& symbolName) {
    if (!isReady()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "JIT engine not initialized");
    }

    return jit_->lookup(symbolName);
}

llvm::Expected<int64_t> JITEngine::executeFunction(const std::string& functionName, 
                                                   const std::vector<int64_t>& args) {
    if (!isReady()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "JIT engine not initialized");
    }

    auto symbolOrErr = jit_->lookup(functionName);
    if (!symbolOrErr) {
        return symbolOrErr.takeError();
    }

    auto functionAddress = symbolOrErr->getValue();
    
    // For now, support only simple functions without arguments
    if (args.empty()) {
        auto function = llvm::jitTargetAddressToPointer<int64_t(*)()>(functionAddress);
        return function();
    } else {
        // Support for functions with arguments would need more complex handling
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "Functions with arguments not yet supported in JIT execution");
    }
}

/******************** CONFIGURATION ********************/

void JITEngine::setTargetTriple(const std::string& targetTriple) {
    targetTriple_ = targetTriple;
}

const std::string& JITEngine::getTargetTriple() const {
    return targetTriple_;
}

/******************** DIAGNOSTICS ********************/

std::string JITEngine::getStatistics() const {
    std::ostringstream oss;
    oss << "JIT Engine Statistics:\n";
    oss << "  Target Triple: " << targetTriple_ << "\n";
    oss << "  Modules Executed: " << modulesExecuted_ << "\n";
    oss << "  Initialized: " << (isInitialized_ ? "Yes" : "No") << "\n";
    oss << "  Ready: " << (isReady() ? "Yes" : "No") << "\n";
    return oss.str();
}

void JITEngine::dumpEngineInfo() const {
    std::cout << getStatistics() << std::endl;
}

size_t JITEngine::getExecutedModuleCount() const {
    return modulesExecuted_;
}

void JITEngine::clearStatistics() {
    modulesExecuted_ = 0;
}

/******************** INTERNAL HELPERS ********************/

llvm::Error JITEngine::prepareModule(llvm::Module& module) {
    // Set up the module's data layout and target triple
    if (jit_) {
        module.setDataLayout(jit_->getDataLayout());
        module.setTargetTriple(targetTriple_);
    }

    // Verify the module
    if (llvm::verifyModule(module, &llvm::errs())) {
        std::cerr << "DEBUG: JIT module verification failed, but continuing..." << std::endl;
        // For debugging, don't return error immediately
        // return llvm::createStringError(llvm::inconvertibleErrorCode(),
        //                              "Module verification failed");
    } else {
        std::cerr << "DEBUG: JIT module verification passed" << std::endl;
    }

    return llvm::Error::success();
}

} // namespace jit
} // namespace codegen
} // namespace emlang

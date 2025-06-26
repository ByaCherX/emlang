//===--- aot_compiler.cpp - Ahead-of-Time Compiler Implementation ---------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of ahead-of-time compilation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/aot_compiler.h"
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
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>
// PassManagerBuilder is deprecated in LLVM 20.x - using new pass manager instead
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/CodeGen/CommandFlags.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <iostream>
#include <fstream>
#include <sstream>

namespace emlang {
namespace codegen {

/******************** CONSTRUCTION AND LIFECYCLE ********************/

AOTCompiler::AOTCompiler(const std::string& targetTriple)
    : optimizationLevel_(OptLevel::None),
      targetTriple_(targetTriple.empty() ? llvm::sys::getDefaultTargetTriple() : targetTriple),
      targetMachine_(nullptr),
      isInitialized_(false),
      modulesCompiled_(0) {
}

AOTCompiler::~AOTCompiler() = default;

/******************** INITIALIZATION ********************/

llvm::Error AOTCompiler::initialize() {
    if (isInitialized_) {
        return llvm::Error::success();
    }

    // Initialize only native targets
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    // Setup target machine
    if (auto err = setupTargetMachine()) {
        return err;
    }

    // Setup optimization pipeline
    if (auto err = setupOptimizationPipeline()) {
        return err;
    }

    isInitialized_ = true;
    return llvm::Error::success();
}

bool AOTCompiler::isReady() const {
    return isInitialized_ && targetMachine_;
}

llvm::Error AOTCompiler::setupTargetMachine() {
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple_, error);
    
    if (!target) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "Target lookup failed: " + error);
    }

    // Setup target options
    llvm::TargetOptions targetOptions;
    targetOptions.EnableFastISel = true;
    
    // Create target machine with appropriate optimization level
    llvm::CodeGenOptLevel codeGenOptLevel = getCodeGenOptLevel();
    
    targetMachine_.reset(target->createTargetMachine(
        targetTriple_,
        "generic",      // CPU
        "",             // Features
        targetOptions,
        llvm::Reloc::PIC_,
        llvm::CodeModel::Small,
        codeGenOptLevel
    ));

    if (!targetMachine_) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "Failed to create target machine");
    }

    return llvm::Error::success();
}

llvm::Error AOTCompiler::setupOptimizationPipeline() {
    // Modern pass pipeline setup will be configured per-module
    return llvm::Error::success();
}

/******************** COMPILATION ********************/

llvm::Error AOTCompiler::compileModule(llvm::Module& module, const std::string& outputPath,
                                      OutputFormat format) {

    if (!isReady()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "AOT compiler not initialized");
    }

    // Verify module first
    if (auto err = verifyModule(module)) {
        return err;
    }

    // Apply optimizations
    if (auto err = applyOptimizations(module)) {
        return err;
    }

    // Open output file
    std::error_code ec;
    llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "Could not open file: " + ec.message());
    }

    // Set up the module's data layout and target triple
    module.setDataLayout(this->targetMachine_->createDataLayout());
    module.setTargetTriple(this->targetTriple_);

    // Create a legacy pass manager for code generation
    llvm::legacy::PassManager passManager;

    // Add target machine passes
    passManager.add(llvm::createTargetTransformInfoWrapperPass(
        targetMachine_->getTargetIRAnalysis()
    ));

    // Generate output based on format
    switch (format) {
        case OutputFormat::LLVM_IR:
            module.print(dest, nullptr);
            dest.close();

            ++modulesCompiled_;
            return llvm::Error::success();
        case OutputFormat::Bitcode:
            llvm::WriteBitcodeToFile(module, dest);
            dest.close();

            ++modulesCompiled_;
            return llvm::Error::success();
        case OutputFormat::Object:
            if (targetMachine_->addPassesToEmitFile(
                    passManager, dest, nullptr, llvm::CodeGenFileType::ObjectFile)
                ) {
                return llvm::createStringError(llvm::inconvertibleErrorCode(),
                    "Target machine cannot emit object files");
            }
            // Run the passes
            passManager.run(module);
            dest.close();

            ++modulesCompiled_;
            return llvm::Error::success();
        case OutputFormat::Assembly:
            if (targetMachine_->addPassesToEmitFile(passManager, dest, nullptr,
                    llvm::CodeGenFileType::AssemblyFile)
                ) {
                return llvm::createStringError(llvm::inconvertibleErrorCode(),
                    "Target machine cannot emit assembly files");
            }
            // Run the passes
            passManager.run(module);
            dest.close();

            ++modulesCompiled_;
            return llvm::Error::success();
        case OutputFormat::Executable:
        /* Executable not supported yet */
            dest.close();
            return llvm::createStringError(llvm::inconvertibleErrorCode(),
                "Executable generation requires system linker integration");
        default:
            dest.close();
            return llvm::createStringError(llvm::inconvertibleErrorCode(),
                "Unsupported output format");
    }
}

llvm::Error AOTCompiler::compileModules(const std::vector<llvm::Module*>& modules,
                                       const std::string& outputPath,
                                       OutputFormat format) {
    if (modules.empty()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "No modules to compile");
    }

    if (modules.size() == 1) {
        return compileModule(*modules[0], outputPath, format);
    }

    // For multiple modules, we need to link them first
    auto mainModule = std::unique_ptr<llvm::Module>(llvm::CloneModule(*modules[0]));
    
    for (size_t i = 1; i < modules.size(); ++i) {
        if (auto err = linkModules(*mainModule, *modules[i])) {
            return err;
        }
    }

    return compileModule(*mainModule, outputPath, format);
}

llvm::Error AOTCompiler::linkModules(llvm::Module& destination, llvm::Module& source) {
    // Create a linker instance
    llvm::Linker linker(destination);
    
    // Clone the source module to avoid modifying the original
    auto sourceClone = llvm::CloneModule(source);
    
    // Link the modules
    if (linker.linkInModule(std::move(sourceClone))) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
            "Failed to link modules");
    }

    return llvm::Error::success();
}

/******************** OPTIMIZATION ********************/

void AOTCompiler::setOptimizationLevel(OptLevel level) {
    optimizationLevel_ = level;
}

OptLevel AOTCompiler::getOptimizationLevel() const {
    return optimizationLevel_;
}

llvm::Error AOTCompiler::verifyModule(llvm::Module& module) {
    if (llvm::verifyModule(module, &llvm::errs())) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Module verification failed");
    }
    return llvm::Error::success();
}

llvm::Error AOTCompiler::applyOptimizations(llvm::Module& module) {
    if (optimizationLevel_ == OptLevel::None) {
        return llvm::Error::success();
    }

    // Set up the module's data layout
    module.setDataLayout(targetMachine_->createDataLayout());
    module.setTargetTriple(targetTriple_);

    // Use the new pass manager
    llvm::PassBuilder passBuilder;
    
    // Register all standard analyses
    llvm::LoopAnalysisManager loopAnalysisManager;
    llvm::FunctionAnalysisManager functionAnalysisManager;
    llvm::CGSCCAnalysisManager cgsccAnalysisManager;
    llvm::ModuleAnalysisManager moduleAnalysisManager;

    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
    passBuilder.registerCGSCCAnalyses(cgsccAnalysisManager);
    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
    passBuilder.registerLoopAnalyses(loopAnalysisManager);
    passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager,
                                    cgsccAnalysisManager, moduleAnalysisManager);

    // Convert our optimization level to LLVM's optimization level
    llvm::OptimizationLevel llvmOptLevel;
    switch (optimizationLevel_) {
        case OptLevel::O1:
            llvmOptLevel = llvm::OptimizationLevel::O1;
            break;
        case OptLevel::O2:
            llvmOptLevel = llvm::OptimizationLevel::O2;
            break;
        case OptLevel::O3:
            llvmOptLevel = llvm::OptimizationLevel::O3;
            break;
        case OptLevel::Os:
            llvmOptLevel = llvm::OptimizationLevel::Os;
            break;
        case OptLevel::Oz:
            llvmOptLevel = llvm::OptimizationLevel::Oz;
            break;
        default:
            llvmOptLevel = llvm::OptimizationLevel::O2;
            break;
    }

    // Build the optimization pipeline
    llvm::ModulePassManager modulePassManager = 
        passBuilder.buildPerModuleDefaultPipeline(llvmOptLevel);

    // Run the optimization passes
    modulePassManager.run(module, moduleAnalysisManager);

    return llvm::Error::success();
}

/******************** CONFIGURATION ********************/

void AOTCompiler::setTargetTriple(const std::string& targetTriple) {
    targetTriple_ = targetTriple;
}

const std::string& AOTCompiler::getTargetTriple() const {
    return targetTriple_;
}

llvm::TargetMachine* AOTCompiler::getTargetMachine() const {
    return targetMachine_.get();
}

/******************** DIAGNOSTICS ********************/

std::string AOTCompiler::getStatistics() const {
    std::ostringstream oss;
    oss << "AOT Compiler Statistics:\n";
    oss << "  Target Triple: " << targetTriple_ << "\n";
    oss << "  Optimization Level: " << static_cast<int>(optimizationLevel_) << "\n";
    oss << "  Modules Compiled: " << modulesCompiled_ << "\n";
    oss << "  Initialized: " << (isInitialized_ ? "Yes" : "No") << "\n";
    return oss.str();
}

void AOTCompiler::dumpCompilerInfo() const {
    std::cout << getStatistics() << std::endl;
}

size_t AOTCompiler::getCompiledModuleCount() const {
    return modulesCompiled_;
}

void AOTCompiler::clearStatistics() {
    modulesCompiled_ = 0;
}

/******************** OPTIMIZATION HELPERS ********************/

void AOTCompiler::addOptimizationPasses(llvm::legacy::PassManager& modulePassManager,
                                        llvm::legacy::FunctionPassManager& functionPassManager) {
    // This method is kept for compatibility but is deprecated
    // Modern optimization should use the new pass manager in applyOptimizations()
}

llvm::CodeGenOptLevel AOTCompiler::getCodeGenOptLevel() const {
    switch (optimizationLevel_) {
        case OptLevel::None:
            return llvm::CodeGenOptLevel::None;
        case OptLevel::O1:
            return llvm::CodeGenOptLevel::Less;
        case OptLevel::O2:
        case OptLevel::Os:
            return llvm::CodeGenOptLevel::Default;
        case OptLevel::O3:
        case OptLevel::Oz:
            return llvm::CodeGenOptLevel::Aggressive;
        default:
            return llvm::CodeGenOptLevel::Default;
    }
}

} // namespace codegen
} // namespace emlang

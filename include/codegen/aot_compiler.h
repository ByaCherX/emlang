//===--- aot_compiler.h - Ahead-of-Time Compiler Interface ------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Ahead-of-time compilation interface for EMLang
//
// This file provides AOT compilation capabilities for EMLang, including
// object file generation, executable creation, and optimization passes.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_AOT_COMPILER_H
#define EM_CODEGEN_AOT_COMPILER_H

#pragma once

#include <emlang_export.h>
#include "context.h"
#include "base.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/Support/Error.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Bitcode/BitcodeWriter.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <memory>
#include <string>
#include <vector>
#include "base.h"

namespace llvm {
    class Module;
}

namespace emlang {
namespace codegen {

/**
 * @class AOTCompiler
 * @brief Ahead-of-time compiler for EMLang
 * 
 * AOTCompiler provides comprehensive ahead-of-time compilation capabilities
 * for EMLang programs. It supports:
 * - Multiple output formats (object files, executables, assembly, IR)
 * - Configurable optimization levels
 * - Multi-platform target support
 * - Module linking and optimization
 * - Comprehensive error reporting
 * 
 * **Compilation Pipeline:**
 * 1. Module verification
 * 2. Optimization passes (based on level)
 * 3. Target-specific code generation
 * 4. Output format generation
 * 5. Optional linking (for executables)
 * 
 * **Usage Example:**
 * ```cpp
 * AOTCompiler compiler();
 * compiler.initialize();
 * compiler.compileModule(module, "output.o", OutputFormat::ObjectFile);
 * ```
 */
class AOTCompiler {
private:
    OptLevel optimizationLevel_;
    std::string targetTriple_;
    std::unique_ptr<llvm::TargetMachine> targetMachine_;

    bool isInitialized_;
    size_t modulesCompiled_;
    
public:
    /******************** CONSTRUCTION AND LIFECYCLE ********************/

    /**
     * @brief Constructs an AOT compiler
     * @param targetTriple Target triple (empty for native)
     */
    explicit AOTCompiler(const std::string& targetTriple = "");

    /**
     * @brief Destructor
     */
    ~AOTCompiler();

    // Disable copy semantics
    AOTCompiler(const AOTCompiler&) = delete;
    AOTCompiler& operator=(const AOTCompiler&) = delete;

    // Enable move semantics
    AOTCompiler(AOTCompiler&&) = default;
    AOTCompiler& operator=(AOTCompiler&&) = default;

    /******************** INITIALIZATION ********************/

    /**
     * @brief Initializes the AOT compiler
     * 
     * Sets up:
     * - LLVM targets and passes
     * - Target machine configuration
     * - Optimization pipeline
     * 
     * @return Error if initialization failed
     */
    llvm::Error initialize();

    /**
     * @brief Checks if compiler is ready for compilation
     * @return true if initialized and ready
     */
    bool isReady() const;


    /******************** COMPILATION ********************/

    /**
     * @brief Compiles a single module
     * @param module Module to compile
     * @param outputPath Output file path
     * @param format Output format
     * @return Error if compilation failed
     */
    llvm::Error compileModule(llvm::Module& module, const std::string& outputPath,
                             codegen::OutputFormat format);

    /**
     * @brief Compiles multiple modules (with linking)
     * @param modules Modules to compile
     * @param outputPath Output file path
     * @param format Output format
     * @return Error if compilation failed
     */
    llvm::Error compileModules(const std::vector<llvm::Module*>& modules,
                              const std::string& outputPath,
                              codegen::OutputFormat format);

    /**
     * @brief Links multiple modules into one
     * @param destination Destination module
     * @param source Source module to link
     * @return Error if linking failed
     */
    llvm::Error linkModules(llvm::Module& destination, llvm::Module& source);

    /******************** OPTIMIZATION ********************/

    
    /** @brief Sets the optimization level */
    void setOptimizationLevel(OptLevel level);

    /** @return Current optimization level */
    OptLevel getOptimizationLevel() const;

    /**
     * @brief Verifies a module for correctness
     * @param module Module to verify
     * @return Error if verification failed
     */
    llvm::Error verifyModule(llvm::Module& module);

    /**
     * @brief Applies optimization passes to a module
     * @param module Module to optimize
     * @return Error if optimization failed
     */
    llvm::Error applyOptimizations(llvm::Module& module);

    /******************** CONFIGURATION ********************/

    /**
     * @brief Sets the target triple
     * @param targetTriple New target triple
     */
    void setTargetTriple(const std::string& targetTriple);

    /**
     * @brief Gets the current target triple
     * @return Current target triple
     */
    const std::string& getTargetTriple() const;

    /**
     * @brief Gets the target machine
     * @return Pointer to target machine or nullptr
     */
    llvm::TargetMachine* getTargetMachine() const;

    /******************** DIAGNOSTICS ********************/

    /**
     * @brief Gets compiler statistics
     * @return Statistics string
     */
    std::string getStatistics() const;

    /**
     * @brief Dumps compiler information to console
     */
    void dumpCompilerInfo() const;

    /**
     * @brief Gets the number of compiled modules
     * @return Number of modules compiled
     */
    size_t getCompiledModuleCount() const;

    /**
     * @brief Clears compilation statistics
     */
    void clearStatistics();

private:
    /******************** INITIALIZATION HELPERS ********************/

    /**
     * @brief Sets up the target machine
     * @return Error if setup failed
     */
    llvm::Error setupTargetMachine();

    /**
     * @brief Sets up the optimization pipeline
     * @return Error if setup failed
     */
    llvm::Error setupOptimizationPipeline();

    /******************** OUTPUT HELPERS ********************/

    /**
     * @brief Links object file to create executable
     * @param objectPath Object file path
     * @param executablePath Executable file path
     * @return Error if linking failed
     */
    llvm::Error linkExecutable(const std::string& objectPath, const std::string& executablePath);

    /******************** OPTIMIZATION HELPERS ********************/

    /**
     * @brief Adds optimization passes to pass managers
     * @param modulePassManager Module pass manager
     * @param functionPassManager Function pass manager
     */
    void addOptimizationPasses(llvm::legacy::PassManager& modulePassManager,
                              llvm::legacy::FunctionPassManager& functionPassManager);    

    /**
     * @brief Gets LLVM code generation optimization level
     * @return LLVM optimization level
     */
    llvm::CodeGenOptLevel getCodeGenOptLevel() const;
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_AOT_COMPILER_H

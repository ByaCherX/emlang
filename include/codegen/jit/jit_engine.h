//===--- jit_engine.h - Just-in-Time Compiler Interface --------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Just-in-time compilation interface for EMLang
//
// This file provides JIT compilation capabilities for EMLang, including
// runtime execution, dynamic loading, and interactive development support.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_JIT_ENGINE_H
#define EM_CODEGEN_JIT_ENGINE_H

#pragma once

#include <emlang_export.h>
#include "../context.h"
#include "../base.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/Support/Error.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace llvm {
    class Module;
    class LLVMContext;
}

namespace emlang {
namespace codegen {
namespace jit {

/**
 * @class JITEngine
 * @brief Just-in-time compiler for EMLang
 * 
 * JITEngine provides comprehensive just-in-time compilation capabilities
 * for EMLang programs. It supports:
 * - Runtime execution of EMLang code
 * - Dynamic symbol resolution
 * - Interactive development support
 * - Function-level compilation
 * - Memory management and cleanup
 * 
 * **Execution Pipeline:**
 * 1. Module compilation to machine code
 * 2. Symbol resolution and linking
 * 3. Runtime execution
 * 4. Result retrieval
 * 
 * **Usage Example:**
 * ```cpp
 * JITEngine engine();
 * engine.initialize();
 * auto result = engine.executeModule(module);
 * ```
 */
class EMLANG_API JITEngine {
private:
    std::unique_ptr<llvm::orc::LLJIT> jit_;
    bool isInitialized_;
    size_t modulesExecuted_;
    
    // Execution context
    std::string targetTriple_;
    
public:
    /******************** CONSTRUCTION AND LIFECYCLE ********************/

    /**
     * @brief Constructs a JIT engine
     * @param targetTriple Target triple (empty for native)
     */
    explicit JITEngine(const std::string& targetTriple = "");

    /**
     * @brief Destructor
     */
    ~JITEngine();

    /**
     * @brief Move constructor
     */
    JITEngine(JITEngine&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    JITEngine& operator=(JITEngine&& other) noexcept;

    // Disable copy constructor and copy assignment
    JITEngine(const JITEngine&) = delete;
    JITEngine& operator=(const JITEngine&) = delete;

    /******************** INITIALIZATION ********************/

    /**
     * @brief Initializes the JIT engine
     * @return Error on failure, success otherwise
     */
    llvm::Error initialize();

    /**
     * @brief Checks if the JIT engine is ready for use
     * @return true if initialized and ready
     */
    bool isReady() const;

    /******************** EXECUTION ********************/

    /**
     * @brief Executes a module and returns the result of main function
     * @param module LLVM module to execute
     * @return Exit code from main function
     */
    llvm::Expected<int> executeModule(llvm::Module& module);

    /**
     * @brief Adds a module to the JIT engine
     * @param module Module to add (takes ownership)
     * @return Error on failure
     */
    llvm::Error addModule(std::unique_ptr<llvm::Module> module);

    /**
     * @brief Looks up a symbol in the JIT
     * @param symbolName Name of the symbol to find
     * @return Symbol address or error
     */
    llvm::Expected<llvm::orc::ExecutorAddr> lookupSymbol(const std::string& symbolName);

    /**
     * @brief Executes a specific function by name
     * @param functionName Name of the function to execute
     * @param args Arguments to pass to the function
     * @return Function result or error
     */
    llvm::Expected<int64_t> executeFunction(const std::string& functionName, 
                                           const std::vector<int64_t>& args = {});

    /******************** CONFIGURATION ********************/

    /**
     * @brief Sets the target triple
     * @param targetTriple Target triple string
     */
    void setTargetTriple(const std::string& targetTriple);

    /**
     * @brief Gets the target triple
     * @return Current target triple
     */
    const std::string& getTargetTriple() const;

    /******************** DIAGNOSTICS ********************/

    /**
     * @brief Gets compilation and execution statistics
     * @return Statistics string
     */
    std::string getStatistics() const;

    /**
     * @brief Prints JIT engine information
     */
    void dumpEngineInfo() const;

    /**
     * @brief Gets the number of executed modules
     * @return Module count
     */
    size_t getExecutedModuleCount() const;

    /**
     * @brief Clears execution statistics
     */
    void clearStatistics();

private:
    /******************** INTERNAL HELPERS ********************/

    /**
     * @brief Sets up the JIT engine with target configuration
     * @return Error on failure
     */
    llvm::Error setupJIT();

    /**
     * @brief Prepares a module for execution
     * @param module Module to prepare
     * @return Error on failure
     */
    llvm::Error prepareModule(llvm::Module& module);
};

} // namespace jit
} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_JIT_ENGINE_H

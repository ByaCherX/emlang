//===--- llvm_context.h - Context Management --------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// context management for EMLang code generation
//
// This file contains the context management infrastructure,
// including type mapping, module management, and optimization control.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_CONTEXT_H
#define EM_CODEGEN_CONTEXT_H

#pragma once

#include <emlang_export.h>
#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>


namespace emlang {
namespace codegen {

/**
 * @enum OptimizationLevel
 * @brief Defines optimization levels for code generation
 */
enum class OptimizationLevel {
    None = 0x0,      ///< No optimizations applied (O0)
    O1   = 0x1,      ///< Basic optimizations (-O1)
    O2   = 0x2,      ///< Standard optimizations (-O2) 
    O3   = 0x3       ///< Aggressive optimizations (-O3)
};

/**
 * @class ContextManager
 * @brief Manages LLVM context, module, and type mapping
 * 
 * ContextManager encapsulates LLVM's core infrastructure
 * and provides type mapping between EMLang and LLVM types.
 * It manages the LLVM context, module, and IR builder lifecycle.
 */
class EMLANG_API ContextManager {
private:
    std::unique_ptr<llvm::LLVMContext> context;  ///< LLVM context managing global state
    std::unique_ptr<llvm::Module> module;        ///< LLVM module containing all generated functions and globals
    std::unique_ptr<llvm::IRBuilder<>> builder;  ///< LLVM IR builder for convenient instruction generation

    OptimizationLevel optimizationLevel;         ///< Current optimization level

    // ======================== INITIALIZATION HELPERS ========================

    void initializeTargets();                    ///< Initializes target-specific components
    void registerBuiltinFunctions();             ///< Registers built-in functions as extern declarations

public:
    /**
     * @brief Constructs LLVM context manager
     * @param moduleName Name for the LLVM module
     * @param optLevel Optimization level
     */
    ContextManager(const std::string& moduleName, OptimizationLevel optLevel = OptimizationLevel::None);

    /**
     * @brief Default destructor
     */
    ~ContextManager() = default;

    // ======================== LLVM COMPONENT ACCESS ========================

    /**
     * @brief Gets the LLVM context
     * @return Reference to LLVM context
     */
    llvm::LLVMContext& getContext() const { return *context; }

    /**
     * @brief Gets the LLVM module
     * @return Pointer to LLVM module
     */
    llvm::Module* getModule() const { return module.get(); }

    /**
     * @brief Gets the LLVM IR builder
     * @return Reference to IR builder
     */
    llvm::IRBuilder<>& getBuilder() const { return *builder; }    
    
    // ======================== ALLOCA HELPER ========================

    /**
     * @brief Creates an alloca instruction in function entry block
     * @param function Function to create alloca in
     * @param varName Variable name for debugging
     * @param type LLVM type of the variable
     * @return LLVM value pointer to allocated memory
     */
    llvm::Value* createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type);

    // ======================== OPTIMIZATION CONTROL ========================

    /**
     * @brief Sets the optimization level
     * @param level New optimization level
     */
    void setOptimizationLevel(OptimizationLevel level);

    /**
     * @brief Gets the current optimization level
     * @return Current optimization level
     */
    OptimizationLevel getOptimizationLevel() const;

    /**
     * @brief Applies optimization passes based on current level
     */
    void runOptimizationPasses();

    // ======================== OUTPUT GENERATION ========================

    /**
     * @brief Prints LLVM IR to standard output
     */
    void printIR() const;

    /**
     * @brief Writes LLVM IR to file
     * @param filename Output file path
     */
    void writeIRToFile(const std::string& filename) const;

    /**
     * @brief Writes object file
     * @param filename Output object file path
     */
    void writeObjectFile(const std::string& filename) const;

};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_LLVM_CONTEXT_H

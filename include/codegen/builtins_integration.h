//===--- builtins_integration.h - Builtins Integration --------*- C++ -*-===//
//
// Part of the EMLang Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Integration with existing builtins.h/cpp system for advanced JIT
//
// This file provides integration interface between the modular codegen system 
// and the existing builtin functions infrastructure, enabling seamless symbol
// resolution and runtime binding for both AOT and JIT compilation.
//===----------------------------------------------------------------------===//

#ifndef EMLANG_BUILTINS_INTEGRATION_H
#define EMLANG_BUILTINS_INTEGRATION_H

#pragma once
#include <emlang_export.h>
#include <string>
#include <vector>

#ifdef EMLANG_ORCJIT_ENABLED
// Forward declarations for LLVM types
namespace llvm {
    class Error;
    class Type;
    class LLVMContext;
}
#endif

namespace emlang {

// Forward declarations
class JITEngine;
class SymbolResolver;
class CodegenError;

/**
 * @namespace builtins
 * @brief Builtin functions integration for codegen system
 */
namespace builtins {

/**
 * @brief Initialize the builtin integration system
 * 
 * This function must be called before any other builtin integration
 * functions are used. It registers all builtin function implementations
 * and prepares them for use with both AOT and JIT compilation.
 */
EMLANG_API void initializeBuiltinIntegration();

/**
 * @brief Get function pointer for a builtin function
 * @param name Name of the builtin function
 * @return Function pointer or nullptr if not found
 */
EMLANG_API void* getBuiltinFunctionPointer(const std::string& name);

/**
 * @brief Check if a builtin function is available
 * @param name Name of the function to check
 * @return true if the function is available, false otherwise
 */
EMLANG_API bool isBuiltinFunctionAvailable(const std::string& name);

/**
 * @brief Get list of all available builtin functions
 * @return Vector of function names
 */
EMLANG_API std::vector<std::string> getAvailableBuiltinFunctions();

#ifdef EMLANG_ORCJIT_ENABLED

/**
 * @brief Register all builtin functions with a JIT engine
 * @param jitEngine JIT engine to register functions with
 * @return Error if registration failed
 */
EMLANG_API llvm::Error registerBuiltinsWithJIT(JITEngine& jitEngine);

/**
 * @brief Register all builtin functions with a symbol resolver
 * @param resolver Symbol resolver to register functions with
 * @return Error if registration failed
 */
EMLANG_API llvm::Error registerBuiltinsWithResolver(SymbolResolver& resolver);

/**
 * @brief Get LLVM function type for a builtin function
 * @param name Name of the builtin function
 * @param context LLVM context for type creation
 * @return LLVM function type or nullptr if not found
 */
EMLANG_API llvm::Type* getBuiltinFunctionType(const std::string& name, llvm::LLVMContext& context);

#endif // EMLANG_ORCJIT_ENABLED

} // namespace builtins

/**
 * @namespace codegen
 * @brief Codegen-specific builtin integration
 */
namespace codegen {

/**
 * @class BuiltinCodegenIntegration
 * @brief High-level integration class for codegen and builtins
 */
class EMLANG_API BuiltinCodegenIntegration {
public:
    /**
     * @brief Initialize builtin integration for codegen
     * @return Success or error status
     */
    static CodegenError initializeBuiltins();
    
    /**
     * @brief Get builtin function address for AOT compilation
     * @param name Name of the builtin function
     * @return Function address or nullptr if not found
     */
    static void* getBuiltinAddress(const std::string& name);
    
    /**
     * @brief Check if a symbol is a builtin function
     * @param name Symbol name to check
     * @return true if it's a builtin function
     */
    static bool isBuiltinSymbol(const std::string& name);
    
#ifdef EMLANG_ORCJIT_ENABLED
    /**
     * @brief Register all builtins with a JIT engine
     * @param jit JIT engine instance
     * @return Success or error status
     */
    static CodegenError registerWithJIT(JITEngine& jit);
    
    /**
     * @brief Register all builtins with a symbol resolver
     * @param resolver Symbol resolver instance
     * @return Success or error status
     */
    static CodegenError registerWithResolver(SymbolResolver& resolver);
#endif
};

} // namespace codegen

} // namespace emlang

/******************** C API COMPATIBILITY ********************/

extern "C" {

/**
 * @brief C API: Initialize builtin integration
 */
EMLANG_API void emlang_initialize_builtin_integration();

/**
 * @brief C API: Get builtin function pointer
 * @param name Function name (null-terminated string)
 * @return Function pointer or NULL if not found
 */
EMLANG_API void* emlang_get_builtin_function_pointer(const char* name);

/**
 * @brief C API: Check if builtin function is available
 * @param name Function name (null-terminated string)
 * @return 1 if available, 0 otherwise
 */
EMLANG_API int emlang_is_builtin_function_available(const char* name);

} // extern "C"

#endif // EMLANG_BUILTINS_INTEGRATION_H

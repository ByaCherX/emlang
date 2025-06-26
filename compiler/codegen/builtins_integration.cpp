//===--- builtins_integration.cpp - Builtins Integration ------*- C++ -*-===//
//
// Part of the EMLang Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Integration with existing builtins.h/cpp system for advanced JIT
//
// This file provides integration between the modular codegen system and
// the existing builtin functions infrastructure, enabling seamless symbol
// resolution and runtime binding for both AOT and JIT compilation.
//===----------------------------------------------------------------------===//

#include "codegen/jit/symbol_resolver.h"
#include "codegen/jit/jit_engine.h"
#include "builtins.h"
#include "codegen/codegen_error.h"

// Disable LLVM warnings for includes
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#ifdef EMLANG_ORCJIT_ENABLED
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#endif

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cmath>

namespace emlang {

/******************** BUILTIN FUNCTION IMPLEMENTATIONS ********************/

// External C-style implementations for builtin functions
extern "C" {
    // I/O Functions
    void emlang_print_str(const char* str) {
        if (str) {
            std::cout << str;
        }
    }
    
    void emlang_print_int(int32_t value) {
        std::cout << value;
    }
    
    void emlang_print_char(char c) {
        std::cout << c;
    }
    
    void emlang_print_float(float value) {
        std::cout << value;
    }
    
    void emlang_println() {
        std::cout << std::endl;
    }
    
    // Input Functions
    int32_t emlang_read_int() {
        int32_t value;
        std::cin >> value;
        return value;
    }
    
    char emlang_read_char() {
        char c;
        std::cin >> c;
        return c;
    }
    
    float emlang_read_float() {
        float value;
        std::cin >> value;
        return value;
    }
    
    // Memory Functions
    void* emlang_malloc(int32_t size) {
        return std::malloc(static_cast<size_t>(size));
    }
    
    void emlang_free(void* ptr) {
        if (ptr) {
            std::free(ptr);
        }
    }
    
    void* emlang_memset(void* ptr, int32_t value, int32_t size) {
        return std::memset(ptr, value, static_cast<size_t>(size));
    }
    
    // String Functions
    int32_t emlang_strlen(const char* str) {
        return str ? static_cast<int32_t>(std::strlen(str)) : 0;
    }
    
    int32_t emlang_strcmp(const char* str1, const char* str2) {
        if (!str1 || !str2) return str1 == str2 ? 0 : (str1 ? 1 : -1);
        return std::strcmp(str1, str2);
    }
    
    // Math Functions
    int32_t emlang_pow(int32_t base, int32_t exp) {
        return static_cast<int32_t>(std::pow(base, exp));
    }
    
    int32_t emlang_sqrt(int32_t x) {
        return static_cast<int32_t>(std::sqrt(x));
    }
    
    double emlang_sin(double x) {
        return std::sin(x);
    }
    
    double emlang_cos(double x) {
        return std::cos(x);
    }
    
    int32_t emlang_abs(int32_t x) {
        return std::abs(x);
    }
    
    int32_t emlang_min(int32_t a, int32_t b) {
        return (a < b) ? a : b;
    }
    
    int32_t emlang_max(int32_t a, int32_t b) {
        return (a > b) ? a : b;
    }
}

/******************** BUILTIN INTEGRATION MANAGER ********************/

/**
 * @class BuiltinIntegrationManager
 * @brief Manages integration between builtins and JIT/AOT compilation
 */
class BuiltinIntegrationManager {
private:
    std::unordered_map<std::string, void*> functionMap_;
    bool isInitialized_;
    
public:
    BuiltinIntegrationManager() : isInitialized_(false) {}
    
    /**
     * @brief Initialize the builtin integration system
     */
    void initialize() {
        if (isInitialized_) return;
        
        // Register all builtin function pointers
        functionMap_["emlang_print"] = reinterpret_cast<void*>(emlang_print_str);
        functionMap_["emlang_print_str"] = reinterpret_cast<void*>(emlang_print_str);
        functionMap_["emlang_print_int"] = reinterpret_cast<void*>(emlang_print_int);
        functionMap_["emlang_print_char"] = reinterpret_cast<void*>(emlang_print_char);
        functionMap_["emlang_print_float"] = reinterpret_cast<void*>(emlang_print_float);
        functionMap_["emlang_println"] = reinterpret_cast<void*>(emlang_println);
        
        functionMap_["emlang_read_int"] = reinterpret_cast<void*>(emlang_read_int);
        functionMap_["emlang_read_char"] = reinterpret_cast<void*>(emlang_read_char);
        functionMap_["emlang_read_float"] = reinterpret_cast<void*>(emlang_read_float);
        
        functionMap_["emlang_malloc"] = reinterpret_cast<void*>(emlang_malloc);
        functionMap_["emlang_free"] = reinterpret_cast<void*>(emlang_free);
        functionMap_["emlang_memset"] = reinterpret_cast<void*>(emlang_memset);
        
        functionMap_["emlang_strlen"] = reinterpret_cast<void*>(emlang_strlen);
        functionMap_["emlang_strcmp"] = reinterpret_cast<void*>(emlang_strcmp);
        
        functionMap_["emlang_pow"] = reinterpret_cast<void*>(emlang_pow);
        functionMap_["emlang_sqrt"] = reinterpret_cast<void*>(emlang_sqrt);
        functionMap_["emlang_sin"] = reinterpret_cast<void*>(emlang_sin);
        functionMap_["emlang_cos"] = reinterpret_cast<void*>(emlang_cos);
        functionMap_["emlang_abs"] = reinterpret_cast<void*>(emlang_abs);
        functionMap_["emlang_min"] = reinterpret_cast<void*>(emlang_min);
        functionMap_["emlang_max"] = reinterpret_cast<void*>(emlang_max);
        
        isInitialized_ = true;
    }
    
    /**
     * @brief Get function pointer for a builtin function
     */
    void* getFunctionPointer(const std::string& name) {
        if (!isInitialized_) initialize();
        
        auto it = functionMap_.find(name);
        return (it != functionMap_.end()) ? it->second : nullptr;
    }
    
    /**
     * @brief Get all registered builtin functions
     */
    const std::unordered_map<std::string, void*>& getAllFunctions() {
        if (!isInitialized_) initialize();
        return functionMap_;
    }
    
    /**
     * @brief Check if a function is a registered builtin
     */
    bool hasFunction(const std::string& name) {
        if (!isInitialized_) initialize();
        return functionMap_.find(name) != functionMap_.end();
    }
};

// Global instance
static BuiltinIntegrationManager g_builtinManager;

/******************** PUBLIC INTEGRATION API ********************/

namespace builtins {

void initializeBuiltinIntegration() {
    g_builtinManager.initialize();
}

void* getBuiltinFunctionPointer(const std::string& name) {
    return g_builtinManager.getFunctionPointer(name);
}

bool isBuiltinFunctionAvailable(const std::string& name) {
    return g_builtinManager.hasFunction(name);
}

std::vector<std::string> getAvailableBuiltinFunctions() {
    const auto& functions = g_builtinManager.getAllFunctions();
    std::vector<std::string> names;
    names.reserve(functions.size());
    
    for (const auto& pair : functions) {
        names.push_back(pair.first);
    }
    
    return names;
}

#ifdef EMLANG_ORCJIT_ENABLED

llvm::Error registerBuiltinsWithJIT(JITEngine& jitEngine) {
    // Initialize builtin integration if not already done
    g_builtinManager.initialize();
    
    // Register all builtin functions with the JIT engine
    const auto& functions = g_builtinManager.getAllFunctions();
    
    for (const auto& pair : functions) {
        auto error = jitEngine.addSymbol(pair.first, 
                                       reinterpret_cast<uint64_t>(pair.second));
        if (error) {
            return error;
        }
    }
    
    return llvm::Error::success();
}

llvm::Error registerBuiltinsWithResolver(SymbolResolver& resolver) {
    // Initialize builtin integration if not already done
    g_builtinManager.initialize();
    
    // Register all builtin functions with the symbol resolver
    const auto& functions = g_builtinManager.getAllFunctions();
    
    for (const auto& pair : functions) {
        auto error = resolver.addBuiltinFunction(pair.first, 
                                               reinterpret_cast<uint64_t>(pair.second));
        if (error) {
            return error;
        }
    }
    
    return llvm::Error::success();
}

llvm::Type* getBuiltinFunctionType(const std::string& name, llvm::LLVMContext& context) {
    const BuiltinFunction* builtin = getBuiltinFunction(name);
    if (!builtin) {
        return nullptr;
    }
      // Convert EMLang types to LLVM types
    auto convertType = [&](const std::string& type) -> llvm::Type* {
        if (type == "void") return llvm::Type::getVoidTy(context);
        if (type == "int32") return llvm::Type::getInt32Ty(context);
        if (type == "char") return llvm::Type::getInt8Ty(context);
        if (type == "float") return llvm::Type::getFloatTy(context);
        if (type == "double") return llvm::Type::getDoubleTy(context);
        if (type == "string" || type == "void*") return llvm::PointerType::get(context, 0);
        
        // Default to i8* for unknown types
        return llvm::PointerType::get(context, 0);
    };
    
    // Build parameter types
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : builtin->parameters) {
        paramTypes.push_back(convertType(param.type));
    }
    
    // Build return type
    llvm::Type* returnType = convertType(builtin->returnType);
    
    // Create function type
    return llvm::FunctionType::get(returnType, paramTypes, false);
}

#endif // EMLANG_ORCJIT_ENABLED

} // namespace builtins

/******************** LEGACY COMPATIBILITY ********************/

// Provide backward compatibility with existing builtin system
namespace {

class BuiltinIntegrationBridge {
public:
    static void ensureInitialized() {
        static bool initialized = false;
        if (!initialized) {
            builtins::initializeBuiltinIntegration();
            initialized = true;
        }
    }
    
    static void* getFunctionAddress(const std::string& name) {
        ensureInitialized();
        return builtins::getBuiltinFunctionPointer(name);
    }
};

} // anonymous namespace

/******************** INTEGRATION WITH CODEGEN ********************/

namespace codegen {

class BuiltinCodegenIntegration {
public:
    /**
     * @brief Initialize builtin integration for codegen
     */
    static CodegenError initializeBuiltins() {
        try {
            builtins::initializeBuiltinIntegration();
            //return CodegenError::runtime_error("Builtins initialized successfully");
        } catch (const std::exception& e) {
            //return CodegenError::exception("Failed to initialize builtins: " + std::string(e.what()));
        }
    }
    
    /**
     * @brief Get builtin function address for AOT compilation
     */
    static void* getBuiltinAddress(const std::string& name) {
        return builtins::getBuiltinFunctionPointer(name);
    }
    
    /**
     * @brief Check if a symbol is a builtin function
     */
    static bool isBuiltinSymbol(const std::string& name) {
        return builtins::isBuiltinFunctionAvailable(name);
    }
    
#ifdef EMLANG_ORCJIT_ENABLED
    /**
     * @brief Register all builtins with a JIT engine
     */
    static CodegenError registerWithJIT(JITEngine& jit) {
        try {
            auto error = builtins::registerBuiltinsWithJIT(jit);
            if (error) {
                return CodegenError::createJITError("Failed to register builtins with JIT: " + 
                                                  llvm::toString(std::move(error)));
            }
            return CodegenError::success();
        } catch (const std::exception& e) {
            return CodegenError::createJITError("Exception during builtin JIT registration: " + 
                                              std::string(e.what()));
        }
    }
    
    /**
     * @brief Register all builtins with a symbol resolver
     */
    static CodegenError registerWithResolver(SymbolResolver& resolver) {
        try {
            auto error = builtins::registerBuiltinsWithResolver(resolver);
            if (error) {
                return CodegenError::createSymbolError("Failed to register builtins with resolver: " + 
                                                     llvm::toString(std::move(error)));
            }
            return CodegenError::success();
        } catch (const std::exception& e) {
            return CodegenError::createSymbolError("Exception during builtin resolver registration: " + 
                                                 std::string(e.what()));
        }
    }
#endif
};

} // namespace codegen

} // namespace emlang

/******************** C API COMPATIBILITY ********************/

extern "C" {

EMLANG_API void emlang_initialize_builtin_integration() {
    emlang::builtins::initializeBuiltinIntegration();
}

EMLANG_API void* emlang_get_builtin_function_pointer(const char* name) {
    if (!name) return nullptr;
    return emlang::builtins::getBuiltinFunctionPointer(std::string(name));
}

EMLANG_API int emlang_is_builtin_function_available(const char* name) {
    if (!name) return 0;
    return emlang::builtins::isBuiltinFunctionAvailable(std::string(name)) ? 1 : 0;
}

} // extern "C"

//===--- base.h - Codegen base ----------------------------------*- C++ -*-===//
//
// Part of the EMLang Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Base codegen interface for EMLang
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_BASE_H
#define EM_CODEGEN_BASE_H
#pragma once

namespace emlang {
namespace codegen {

/**
 * @enum OptimizationLevel
 * @brief Defines optimization levels for code generation
 * 
 * EXPERIMENTAL: This enum is experimental and may change in future versions.
 */
enum class OptLevel {
    None = 0,      ///< No optimizations applied (O0)
    O1   = 1,      ///< Basic optimizations (-O1)
    O2   = 2,      ///< Standard optimizations (-O2) 
    O3   = 3,      ///< Aggressive optimizations (-O3)
    Os   = 4,      ///< Optimize for size (-Os)
    Oz   = 5       ///< Optimize for size more aggressively (-Oz)
};

/**
 * @brief AOT compilation modes
 */
enum class AOTMode {
    Static,         // Static compilation
    Dynamic,        // Dynamic compilation
    Default         // Default mode
};

/**
 * @brief JIT compilation modes
 */
enum class JITMode {
    Eager,          // Compile immediately when added
    Lazy,           // Compile on first access
    OnDemand,       // Compile per-function on demand
    Adaptive,       // Adaptive compilation based on usage
    Debugging       // Debug-enabled JIT compilation
};

/**
 * @brief Compilation output formats
 * This enum defines the various output formats that can be generated
 */
enum class OutputFormat {
    Object,         // Object file (.o/.obj)
    Executable,     // Executable binary
    SharedLibrary,  // Shared library (.so/.dll)
    StaticLibrary,  // Static library (.a/.lib)
    Assembly,       // Assembly source (.s/.asm)
    LLVM_IR,        // LLVM IR (.ll)
    Bitcode,        // LLVM Bitcode (.bc)
    WebAssembly     // WebAssembly (.wasm)
};

/**
 * @brief Debug information levels
 */
enum class DebugLevel {
    None,           // No debug information
    Minimal,        // Line numbers only
    Default,        // Standard debug info
    Full            // Full debug information with symbols
};

/**
 * @brief Code generation features
 * This struct defines various code generation features for the code generation process.
 * It allows fine-tuning of the generated code based on performance and security requirements.
 * This struct can be used to configure the code generation process and control various 
 * optimizations and features.It is designed to be flexible and extensible,allowing 
 * future additions of new features as needed.
 * 
 * EXPERIMENTAL: This struct is experimental and may change in future versions.
 */
struct CodeGenFeatures {
    bool enableVectorization = true;
    bool enableInlining = true;
    bool enableLoopUnrolling = true;
    bool enableTailCalls = true;
    bool enableProfileGuidedOpt = false;
    bool enableLinkTimeOpt = false;
    bool enableSafeStack = false;
    bool enableCFI = false;         // Control Flow Integrity
    bool enableSanitizers = false;
    
    CodeGenFeatures() = default;
};


} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_BASE_H

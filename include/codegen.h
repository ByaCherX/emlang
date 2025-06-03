//===--- codegen.h - CodeGen interface --------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// # LLVM-based code generation for the EMLang programming language
//
// This file contains the code generation infrastructure for EMLang, which translates
// the Abstract Syntax Tree (AST) into LLVM Intermediate Representation (IR) and
// subsequently into native machine code. The code generator is the final phase
// of the EMLang compiler pipeline and is responsible for producing executable output.
// 
// **Code Generation Pipeline:**
// ```
// AST → LLVM IR → Optimization Passes → Machine Code → Executable
// ```
// 
// **Key Components:**
// - **CodeGenerator**: Main class implementing AST-to-IR translation
// - **Type System**: Maps EMLang types to LLVM types
// - **Symbol Tables**: Tracks variables and functions during compilation
// - **Optimization**: Configurable optimization levels (O0-O3)
// - **Output Generation**: IR files, object files, and executable generation
// 
// **LLVM Integration:**
// The code generator leverages LLVM's comprehensive infrastructure:
// - IRBuilder for instruction generation
// - Module system for code organization
// - Type system for type safety and optimization
// - Pass manager for optimization pipeline
// - Target-specific code generation
// 
// **Supported Features:**
// - All EMLang primitive types (integers, floats, booleans, strings, pointers)
// - Function definitions with parameters and return types
// - Variable declarations (mutable and immutable)
// - Control flow constructs (if/else, while loops)
// - Expression evaluation with proper precedence
// - Memory management for stack-allocated variables
// - Pointer operations (address-of, dereference)
// - Function calls with argument passing
// 
// **Optimization Support:**
// The code generator supports multiple optimization levels:
// - O0 (None): No optimizations, fastest compilation
// - O1: Basic optimizations, good balance
// - O2: Standard optimizations, production quality
// - O3: Aggressive optimizations, maximum performance
//
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_H
#define EM_CODEGEN_H

#pragma once

/* --- DLL Export/Import Macros for Windows --- */
#ifdef _WIN32
    #ifdef EMLANG_EXPORTS
        #define EMLANG_API __declspec(dllexport)
    #elif defined(EMLANG_DLL)
        #define EMLANG_API __declspec(dllimport)
    #else
        #define EMLANG_API
    #endif
#else
    #define EMLANG_API
#endif
/*----------------------------------------------*/

#include "ast.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include <map>
#include <memory>

namespace emlang {

/**
 * @enum OptimizationLevel
 * @brief Defines optimization levels for code generation
 * 
 * Optimization levels control the trade-off between compilation time and
 * runtime performance. Higher levels apply more aggressive optimizations
 * but require longer compilation times.
 * 
 * **Level Characteristics:**
 * - **None (O0)**: No optimizations, fastest compilation, largest code
 * - **O1**: Basic optimizations, reasonable performance improvement
 * - **O2**: Standard optimizations, good balance for production code
 * - **O3**: Aggressive optimizations, maximum performance, slower compilation
 * 
 * **Optimization Categories:**
 * - Dead code elimination
 * - Constant folding and propagation
 * - Loop optimizations (unrolling, vectorization)
 * - Inlining optimizations
 * - Register allocation improvements
 * - Instruction scheduling and selection
 */
enum class OptimizationLevel {
    None =    0x0,      ///< No optimizations applied (O0)
    O1   =    0x1,      ///< Basic optimizations (-O1)
    O2   =    0x2,      ///< Standard optimizations (-O2) 
    O3   =    0x3       ///< Aggressive optimizations (-O3)
};

/**
 * @class CodeGenerator
 * @brief LLVM-based code generator that translates EMLang AST to executable code
 * 
 * CodeGenerator is the primary class responsible for converting EMLang's Abstract
 * Syntax Tree into LLVM Intermediate Representation (IR) and subsequently into
 * native machine code. It implements the ASTVisitor pattern to traverse the AST
 * and generate corresponding LLVM instructions.
 * 
 * **Architecture Overview:**
 * The code generator follows a single-pass approach where it walks the AST once
 * and generates LLVM IR directly. This design provides:
 * - Simple and straightforward implementation
 * - Good error reporting with source location preservation
 * - Efficient memory usage during compilation
 * - Direct integration with LLVM's optimization pipeline
 * 
 * **Code Generation Process:**
 * 1. **Initialization**: Set up LLVM context, module, and IR builder
 * 2. **Type Registration**: Map EMLang types to LLVM types
 * 3. **Symbol Table Setup**: Prepare tables for variables and functions
 * 4. **AST Traversal**: Visit each node and generate corresponding IR
 * 5. **Optimization**: Apply selected optimization passes
 * 6. **Output Generation**: Produce IR files, object files, or executables
 * 
 * **Memory Management:**
 * - Uses LLVM's automatic memory management for IR objects
 * - Smart pointers for RAII compliance
 * - Proper cleanup of LLVM resources
 * - Stack-based allocation for local variables
 * 
 * **Error Handling:**
 * Comprehensive error detection and reporting for:
 * - Type mismatches during code generation
 * - Undefined symbol references
 * - Invalid operations and expressions
 * - LLVM backend errors
 * 
 * **Integration with LLVM:**
 * - Context: Manages LLVM's global state and type system
 * - Module: Container for all generated functions and globals
 * - IRBuilder: Provides convenient instruction generation API
 * - PassManager: Handles optimization pass execution
 */
class EMLANG_API CodeGenerator : public ASTVisitor {
private:
    // ======================== LLVM CORE COMPONENTS ========================
    
    /**
     * @brief LLVM context managing global state and type system
     * 
     * The LLVM context owns and manages LLVM types, constants, and metadata.
     * Each compilation unit should have its own context to avoid conflicts
     * and enable parallel compilation.
     */
    std::unique_ptr<llvm::LLVMContext> context;
    
    /**
     * @brief LLVM module containing all generated functions and globals
     * 
     * The module is the top-level container for LLVM IR. It holds:
     * - Function definitions and declarations
     * - Global variables and constants
     * - Type definitions and metadata
     * - Target-specific information
     */
    std::unique_ptr<llvm::Module> module;
    
    /**
     * @brief LLVM IR builder for convenient instruction generation
     * 
     * IRBuilder provides a high-level interface for creating LLVM instructions.
     * It maintains an insertion point and automatically handles:
     * - Instruction insertion at the correct location
     * - Basic block and function context
     * - Type inference for many operations
     * - Debug information attachment
     */
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
    /**
     * @brief Current expression type during code generation
     * 
     * Tracks the EMLang type of the currently evaluated expression.
     * This is essential for proper type handling in multi-level pointers
     * and complex expressions where LLVM type information alone is insufficient.
     */
    std::string currentExpressionType;
    
    // ======================== SYMBOL MANAGEMENT ========================
    
    /**
     * @brief Symbol table mapping variable names to LLVM values
     * 
     * Tracks local variables, parameters, and their corresponding LLVM values
     * during code generation. This enables:
     * - Variable lookup during expression evaluation
     * - Scope management for lexical scoping
     * - Type information preservation
     * - Alloca instruction tracking for optimization
     */
    std::map<std::string, llvm::Value*> namedValues;
    
    /**
     * @brief Symbol type table mapping variable names to their EMLang types
     * 
     * Tracks the EMLang types of variables alongside their LLVM values.
     * This is essential for multi-level pointer operations where we need
     * to know the original type structure to perform correct dereferencing.
     */
    std::map<std::string, std::string> namedTypes;
    
    /**
     * @brief Function table mapping function names to LLVM functions
     * 
     * Maintains references to all declared and defined functions for:
     * - Function call resolution
     * - Forward declaration support
     * - Recursive function handling
     * - External function linkage
     */
    std::map<std::string, llvm::Function*> functions;
    
    // ======================== COMPILATION STATE ========================
    
    /**
     * @brief Currently active function being compiled
     * 
     * Tracks the function currently being generated to:
     * - Provide context for return statements
     * - Manage local variable allocation
     * - Handle function-specific operations
     * - Enable proper cleanup on function exit
     */
    llvm::Function* currentFunction;
    
    /**
     * @brief Current optimization level for code generation
     * 
     * Controls which optimization passes are applied and their aggressiveness.
     * This setting affects:
     * - Compilation time vs. runtime performance trade-offs
     * - Code size optimization decisions
     * - Debug information preservation
     * - Optimization pass selection and ordering
     */
    OptimizationLevel optimizationLevel;
    
    // ======================== TYPE SYSTEM INTEGRATION ========================
    
    /**
     * @brief Maps EMLang type names to corresponding LLVM types
     * @param typeName EMLang type name (e.g., "int32", "str", "bool")
     * @return Corresponding LLVM type pointer
     * 
     * This is the main type mapping function that handles the translation
     * between EMLang's type system and LLVM's type system. It supports:
     * - All primitive types (integers, floats, booleans, characters)
     * - Pointer types (int32*, char*, etc.)
     * - String types with appropriate representation
     * - Unit type for void/no-value contexts
     * 
     * The mapping ensures type safety and enables LLVM's type-based
     * optimizations while preserving EMLang's semantic guarantees.
     */
    llvm::Type* getLLVMType(const std::string& typeName);
    
    /**
     * @brief Returns LLVM type for generic numeric values
     * @return LLVM type for default numeric type (typically double)
     * 
     * Used for numeric literals that don't have explicit type annotations.
     * The default numeric type provides a reasonable balance between
     * precision and performance for general arithmetic operations.
     */
    llvm::Type* getNumberType();
    
    /**
     * @brief Returns LLVM type for string values
     * @return LLVM type representing EMLang strings
     * 
     * EMLang strings are represented as LLVM types that can efficiently
     * handle string operations while maintaining compatibility with
     * C-style string interfaces for library interoperability.
     */
    llvm::Type* getStringType();
    
    /**
     * @brief Returns LLVM type for boolean values
     * @return LLVM type for boolean (typically i1)
     * 
     * Boolean type used for conditional expressions, logical operations,
     * and control flow decisions. LLVM's i1 type provides efficient
     * boolean representation with automatic promotion when needed.
     */
    llvm::Type* getBooleanType();
    
    /**
     * @brief Creates LLVM pointer type for the given base type
     * @param baseTypeName EMLang base type name (e.g., "int32" for "int32*")
     * @return LLVM pointer type pointing to the base type
     * 
     * Handles pointer type creation for EMLang's pointer system. This
     * includes validation of the base type and proper LLVM pointer
     * type construction with correct address space handling.
     */
    llvm::Type* getPointerType(const std::string& baseTypeName);
    
    // ======================== SIGNED INTEGER TYPES ========================
    
    /**
     * @brief Returns LLVM type for 8-bit signed integers (int8)
     * @return LLVM i8 type
     * 
     * Corresponds to EMLang's int8 type, providing 8-bit signed integer
     * values with range -128 to 127.
     */
    llvm::Type* getInt8Type();
    
    /**
     * @brief Returns LLVM type for 16-bit signed integers (int16)
     * @return LLVM i16 type
     * 
     * Corresponds to EMLang's int16 type, providing 16-bit signed integer
     * values with range -32,768 to 32,767.
     */
    llvm::Type* getInt16Type();
    
    /**
     * @brief Returns LLVM type for 32-bit signed integers (int32)
     * @return LLVM i32 type
     * 
     * Corresponds to EMLang's int32 type, providing 32-bit signed integer
     * values with range -2,147,483,648 to 2,147,483,647.
     */
    llvm::Type* getInt32Type();
    
    /**
     * @brief Returns LLVM type for 64-bit signed integers (int64)
     * @return LLVM i64 type
     * 
     * Corresponds to EMLang's int64 type, providing 64-bit signed integer
     * values with range -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807.
     */
    llvm::Type* getInt64Type();
    
    /**
     * @brief Returns LLVM type for pointer-sized signed integers (isize)
     * @return LLVM type matching pointer size on target platform
     * 
     * Corresponds to EMLang's isize type, providing signed integers that
     * match the pointer size of the target platform (32-bit or 64-bit).
     */
    llvm::Type* getIsizeType();
    
    // ======================== UNSIGNED INTEGER TYPES ========================
    
    /**
     * @brief Returns LLVM type for 8-bit unsigned integers (uint8)
     * @return LLVM i8 type (unsigned interpretation)
     * 
     * Corresponds to EMLang's uint8 type, providing 8-bit unsigned integer
     * values with range 0 to 255.
     */
    llvm::Type* getUint8Type();
    
    /**
     * @brief Returns LLVM type for 16-bit unsigned integers (uint16)
     * @return LLVM i16 type (unsigned interpretation)
     * 
     * Corresponds to EMLang's uint16 type, providing 16-bit unsigned integer
     * values with range 0 to 65,535.
     */
    llvm::Type* getUint16Type();
    
    /**
     * @brief Returns LLVM type for 32-bit unsigned integers (uint32)
     * @return LLVM i32 type (unsigned interpretation)
     * 
     * Corresponds to EMLang's uint32 type, providing 32-bit unsigned integer
     * values with range 0 to 4,294,967,295.
     */
    llvm::Type* getUint32Type();
    
    /**
     * @brief Returns LLVM type for 64-bit unsigned integers (uint64)
     * @return LLVM i64 type (unsigned interpretation)
     * 
     * Corresponds to EMLang's uint64 type, providing 64-bit unsigned integer
     * values with range 0 to 18,446,744,073,709,551,615.
     */
    llvm::Type* getUint64Type();
    
    /**
     * @brief Returns LLVM type for pointer-sized unsigned integers (usize)
     * @return LLVM type matching pointer size on target platform
     * 
     * Corresponds to EMLang's usize type, providing unsigned integers that
     * match the pointer size of the target platform (32-bit or 64-bit).
     */
    llvm::Type* getUsizeType();
    
    // ======================== FLOATING-POINT TYPES ========================
    
    /**
     * @brief Returns LLVM type for single-precision floating-point (float)
     * @return LLVM float type (IEEE 754 32-bit)
     * 
     * Corresponds to EMLang's float type, providing 32-bit IEEE 754
     * single-precision floating-point numbers.
     */
    llvm::Type* getFloatType();
    
    /**
     * @brief Returns LLVM type for double-precision floating-point (double)
     * @return LLVM double type (IEEE 754 64-bit)
     * 
     * Corresponds to EMLang's double type, providing 64-bit IEEE 754
     * double-precision floating-point numbers.
     */
    llvm::Type* getDoubleType();
    
    // ======================== OTHER PRIMITIVE TYPES ========================
    
    /**
     * @brief Returns LLVM type for character values (char)
     * @return LLVM type for character representation
     * 
     * Corresponds to EMLang's char type, providing Unicode character
     * support with appropriate encoding for the target platform.
     */
    llvm::Type* getCharType();
    
    /**
     * @brief Returns LLVM type for string values (str)
     * @return LLVM type for string representation
     * 
     * Corresponds to EMLang's str type, providing string values with
     * length information and UTF-8 encoding support.
     */
    llvm::Type* getStrType();
    
    /**
     * @brief Returns LLVM type for unit/void values
     * @return LLVM void type
     * 
     * Corresponds to EMLang's unit type or void contexts where no
     * value is produced or expected.
     */
    llvm::Type* getUnitType();
    
    // ======================== TYPE CLASSIFICATION HELPERS ========================
    
    /**
     * @brief Checks if a type name represents a signed integer type
     * @param typeName EMLang type name to check
     * @return true if the type is a signed integer, false otherwise
     * 
     * Used for determining appropriate arithmetic operations and
     * code generation strategies for signed integer types.
     */
    bool isSignedInteger(const std::string& typeName);
    
    /**
     * @brief Checks if a type name represents an unsigned integer type
     * @param typeName EMLang type name to check
     * @return true if the type is an unsigned integer, false otherwise
     * 
     * Used for determining appropriate arithmetic operations and
     * code generation strategies for unsigned integer types.
     */
    bool isUnsignedInteger(const std::string& typeName);
    
    /**
     * @brief Checks if a type name represents a floating-point type
     * @param typeName EMLang type name to check
     * @return true if the type is a floating-point type, false otherwise
     * 
     * Used for determining appropriate arithmetic operations and
     * code generation strategies for floating-point types.
     */
    bool isFloatingPoint(const std::string& typeName);
    
    /**
     * @brief Checks if a type name represents a primitive type
     * @param typeName EMLang type name to check
     * @return true if the type is primitive, false otherwise
     * 
     * Used for distinguishing between primitive types and complex
     * types like structures, arrays, or user-defined types.
     */
    bool isPrimitiveType(const std::string& typeName);
    
    /**
     * @brief Extracts the element type from a pointer value using source type information
     * @param pointerValue LLVM pointer value to extract element type from
     * @param sourceType Original EMLang source type string (e.g., "int32*", "int32**")
     * @return LLVM type of the pointed-to element, or nullptr if not a pointer type
     * 
     * This helper method is essential for multi-level pointer dereferencing operations.
     * It uses the original EMLang type information to determine the correct pointed-to
     * type, which is crucial for proper LLVM IR generation when dealing with complex
     * pointer hierarchies.
     * 
     * **Multi-level Pointer Support:**
     * - Single pointer: "int32*" → returns i32 type
     * - Double pointer: "int32**" → returns i32* type  
     * - Triple pointer: "int32***" → returns i32** type
     * 
     * **Usage in Code Generation:**
     * - Used by DereferenceExpression visitor for proper load instruction generation
     * - Enables correct type tracking through multiple dereference operations
     * - Maintains type safety in complex pointer arithmetic
     * 
     * **Type Safety:**
     * - Validates that sourceType is actually a pointer type
     * - Returns nullptr for non-pointer types to indicate error condition
     * - Preserves original type semantics during IR generation
     */
    llvm::Type* getElementTypeFromPointer(llvm::Value* pointerValue, const std::string& sourceType);
    
    /**
     * @brief Removes one level of pointer indirection from an EMLang type string
     * @param pointerType EMLang pointer type string (e.g., "int32**", "char*")
     * @return Type string with one less level of indirection (e.g., "int32*", "char")
     * 
     * String manipulation helper for multi-level pointer type processing. This method
     * is used to track type information through multiple levels of pointer dereferencing
     * and ensures proper type propagation in complex pointer expressions.
     * 
     * **Pointer Level Processing:**
     * - "int32***" → "int32**" (triple to double pointer)
     * - "int32**" → "int32*" (double to single pointer)
     * - "int32*" → "int32" (pointer to base type)
     * - "int32" → "int32" (non-pointer unchanged)
     * 
     * **Multi-level Pointer Operations:**
     * - Used in conjunction with getElementTypeFromPointer()
     * - Enables recursive type resolution for nested pointer structures
     * - Maintains type consistency across dereference chains
     * 
     * **Error Handling:**
     * - Returns input unchanged if not a pointer type
     * - Safe for use with any type string (no validation errors)
     * - Gracefully handles edge cases like empty strings
     * 
     * **Integration with LLVM:**
     * - Result can be passed to getLLVMType() for LLVM type conversion
     * - Supports the full EMLang type system including complex pointer hierarchies
     * - Essential for proper SSA form generation in multi-level pointer operations
     */
    std::string getPointeeType(const std::string& pointerType);

    // ======================== CODE GENERATION HELPERS ========================
    
    /**
     * @brief Creates an alloca instruction in the entry block of a function
     * @param function The function to create the alloca in
     * @param varName Name of the variable for debugging information
     * @param type LLVM type of the variable
     * @return LLVM Value pointer to the allocated memory
     * 
     * This helper creates stack-allocated memory for local variables in the
     * function's entry block. Placing allocas in the entry block enables:
     * - LLVM's mem2reg pass to promote allocas to registers
     * - Better optimization opportunities
     * - Simplified control flow analysis
     * - Proper variable lifetime management
     * 
     * The generated alloca instructions are automatically promoted to SSA
     * form by LLVM's optimization passes when beneficial.
     */
    llvm::Value* createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type);
    
    /**
     * @brief Applies optimization passes based on the current optimization level
     * 
     * Runs a series of optimization passes on the generated LLVM IR to improve
     * performance and code quality. The specific passes applied depend on the
     * configured optimization level:
     * 
     * **O0 (None)**: No optimization passes applied
     * **O1 (Basic)**: Essential passes for reasonable performance
     * **O2 (Standard)**: Comprehensive optimization suite for production
     * **O3 (Aggressive)**: Maximum optimization with potential compile-time cost
     * 
     * Common optimization categories:
     * - Dead code elimination
     * - Constant folding and propagation
     * - Loop optimizations (unrolling, vectorization)
     * - Function inlining
     * - Memory-to-register promotion
     * - Instruction combining and simplification
     */
    void runOptimizationPasses();

public:
    // ======================== CONSTRUCTION AND LIFECYCLE ========================
    
    /**
     * @brief Constructs a new CodeGenerator instance
     * @param moduleName Name for the LLVM module (typically source filename)
     * @param optLevel Optimization level to apply (default: None)
     * 
     * Initializes the code generator with a fresh LLVM context, module, and
     * IR builder. The module name is used for:
     * - Debug information and error reporting
     * - Linking and symbol resolution
     * - Output file naming
     * - Development tools integration
     * 
     * The optimization level can be changed later using setOptimizationLevel().
     */
    CodeGenerator(const std::string& moduleName, OptimizationLevel optLevel = OptimizationLevel::None);
    
    /**
     * @brief Default destructor with automatic LLVM resource cleanup
     * 
     * LLVM resources are automatically cleaned up through smart pointers
     * and LLVM's own memory management system.
     */
    ~CodeGenerator() = default;
    
    // ======================== PRIMARY CODE GENERATION ========================
    
    /**
     * @brief Generates LLVM IR from an EMLang program AST
     * @param program Root AST node representing the entire program
     * 
     * This is the main entry point for code generation. It processes the
     * entire program AST and generates corresponding LLVM IR through the
     * visitor pattern. The process includes:
     * 
     * 1. **Initialization**: Set up symbol tables and compilation context
     * 2. **Type Registration**: Process type definitions and mappings
     * 3. **Function Declarations**: Register all function signatures
     * 4. **Code Generation**: Generate IR for all program constructs
     * 5. **Validation**: Verify the generated IR for correctness
     * 6. **Optimization**: Apply configured optimization passes
     * 
     * After successful generation, the IR is ready for further processing
     * such as object file generation or JIT execution.
     */
    void generateIR(Program& program);
    
    // ======================== MODULE ACCESS ========================
    
    /**
     * @brief Returns the generated LLVM module
     * @return Pointer to the LLVM module containing generated IR
     * 
     * Provides access to the underlying LLVM module for:
     * - Integration with LLVM tools and libraries
     * - Custom optimization pass application
     * - Advanced code analysis and transformation
     * - Direct LLVM API usage
     * 
     * The returned module remains owned by the CodeGenerator instance.
     */
    llvm::Module* getModule() const;
    
    // ======================== OUTPUT GENERATION ========================
    
    /**
     * @brief Prints the generated LLVM IR to standard output
     * 
     * Outputs human-readable LLVM IR for:
     * - Debugging and development
     * - Understanding generated code
     * - Verification of correctness
     * - Educational purposes
     * 
     * The output includes all functions, global variables, type definitions,
     * and metadata in LLVM's textual IR format.
     */
    void printIR() const;
    
    /**
     * @brief Writes the generated LLVM IR to a file
     * @param filename Path to the output file (typically .ll extension)
     * 
     * Saves the LLVM IR in textual format to a file for:
     * - Later processing with LLVM tools
     * - Integration with build systems
     * - Archive and version control
     * - Cross-platform IR sharing
     * 
     * The generated .ll files can be processed with standard LLVM tools
     * like llc, opt, and lli.
     */
    void writeIRToFile(const std::string& filename) const;
    
    /**
     * @brief Generates native object file from the LLVM IR
     * @param filename Path to the output object file (typically .o extension)
     * 
     * Compiles the LLVM IR to native machine code and writes it as an
     * object file suitable for linking. The process includes:
     * - Target machine configuration
     * - Instruction selection and scheduling
     * - Register allocation
     * - Assembly generation
     * - Object file format generation
     * 
     * The generated object files can be linked with standard system linkers
     * to create executable programs or shared libraries.
     */
    void writeObjectFile(const std::string& filename) const;
    
    // ======================== EXECUTION ========================
    
    /**
     * @brief JIT compiles and executes the main function
     * @return Exit code returned by the main function
     * 
     * Uses LLVM's JIT compilation infrastructure to compile and execute
     * the generated IR directly in memory. This is useful for:
     * - Interactive development and testing
     * - Script-like execution of EMLang programs
     * - Rapid prototyping and debugging
     * - Integration testing of the compiler
     * 
     * The function looks for a main() function in the generated IR and
     * executes it with appropriate runtime setup and cleanup.
     */
    int executeMain();
    
    // ======================== OPTIMIZATION CONTROL ========================
    
    /**
     * @brief Sets the optimization level for code generation
     * @param level New optimization level to apply
     * 
     * Changes the optimization level that will be applied to subsequently
     * generated code. This affects:
     * - Which optimization passes are run
     * - Compilation time vs. performance trade-offs
     * - Debug information preservation
     * - Code size characteristics
     */
    void setOptimizationLevel(OptimizationLevel level);
    
    /**
     * @brief Gets the current optimization level
     * @return Current optimization level setting
     * 
     * Returns the currently configured optimization level for inspection
     * and conditional behavior in the code generator.
     */
    OptimizationLevel getOptimizationLevel() const;    
    
    // ======================== AST VISITOR IMPLEMENTATION ========================
    // These methods implement the ASTVisitor interface to traverse the AST
    // and generate corresponding LLVM IR for each node type
    
    /**
     * @brief Generates LLVM IR for literal expressions
     * @param node Literal expression node (numbers, strings, booleans, etc.)
     * 
     * Creates LLVM constant values for literal expressions:
     * - Numeric literals: ConstantInt or ConstantFP based on type
     * - String literals: Global string constants with appropriate linkage
     * - Boolean literals: ConstantInt with i1 type
     * - Character literals: ConstantInt with appropriate character encoding
     * - Null literals: Null pointer constants
     * 
     * The generated constants are stored in currentValue for use by
     * parent expressions or statements.
     */
    void visit(LiteralExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for identifier expressions
     * @param node Identifier expression node (variable/function references)
     * 
     * Resolves identifier references and generates appropriate load instructions:
     * - Variable references: Load from alloca or global variable
     * - Function references: Function pointer for function calls
     * - Parameter references: Load from parameter allocas
     * 
     * Includes symbol table lookup and error handling for undefined identifiers.
     */
    void visit(IdentifierExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for binary operation expressions
     * @param node Binary operation node (+, -, *, /, ==, etc.)
     * 
     * Generates appropriate LLVM instructions for binary operations:
     * - Arithmetic: Add, Sub, Mul, SDiv/UDiv, SRem/URem, FAdd, FSub, FMul, FDiv
     * - Comparison: ICmp/FCmp with appropriate predicates
     * - Logical: And, Or with short-circuit evaluation
     * - Bitwise: And, Or, Xor, Shl, LShr, AShr
     * 
     * Includes type checking, operand processing, and result type inference.
     */
    void visit(BinaryOpExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for unary operation expressions
     * @param node Unary operation node (-, !, &)
     * 
     * Generates LLVM instructions for unary operations:
     * - Arithmetic negation: Sub from zero or FNeg
     * - Logical negation: XOR with true for boolean values
     * - Address-of: Returns alloca pointer without loading
     * 
     * Includes operand type validation and appropriate instruction selection.
     */
    void visit(UnaryOpExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for function call expressions
     * @param node Function call node with arguments
     * 
     * Generates function call instructions with proper argument handling:
     * - Function resolution from symbol table
     * - Argument evaluation and type checking
     * - Call instruction generation with proper calling convention
     * - Return value handling and type propagation
     * 
     * Supports both user-defined and external function calls.
     */
    void visit(FunctionCallExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for pointer dereference expressions
     * @param node Dereference operation node (*ptr)
     * 
     * Generates load instructions for pointer dereferencing:
     * - Pointer type validation
     * - Load instruction generation
     * - Null pointer checking (when enabled)
     * - Result type determination from pointer target type
     */
    void visit(DereferenceExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for address-of expressions
     * @param node Address-of operation node (&var)
     * 
     * Generates instructions to obtain memory addresses:
     * - Addressability validation of operand
     * - Alloca pointer extraction for variables
     * - GEP instructions for complex addressing
     * - Pointer type construction
     */
    void visit(AddressOfExpression& node) override;
    
    /**
     * @brief Generates LLVM IR for variable declarations
     * @param node Variable declaration node (let/const)
     * 
     * Creates local variables and handles initialization:
     * - Alloca instruction in function entry block
     * - Symbol table registration
     * - Initializer evaluation and store instruction
     * - Type checking and validation
     * 
     * Supports both initialized and uninitialized variables.
     */
    void visit(VariableDeclaration& node) override;
    
    /**
     * @brief Generates LLVM IR for function declarations
     * @param node Function declaration node with signature and body
     * 
     * Creates LLVM functions with proper signatures:
     * - Function type construction from parameters and return type
     * - Function creation and registration in module
     * - Parameter setup and entry block creation
     * - Body generation with proper scope management
     * - Return instruction validation and insertion
     */
    void visit(FunctionDeclaration& node) override;
    
    /**
     * @brief Generates LLVM IR for external function declarations
     * @param node External function declaration node with signature (no body)
     * 
     * Creates LLVM function declarations for external library functions:
     * - Function type construction from parameters and return type
     * - External linkage function creation and registration in module
     * - Symbol table registration for function calls
     * - No body generation (declaration only)
     */
    void visit(ExternFunctionDeclaration& node) override;
    
    /**
     * @brief Generates LLVM IR for block statements
     * @param node Block statement containing multiple statements
     * 
     * Handles compound statements and scope management:
     * - Sequential statement processing
     * - Symbol table scope management
     * - Control flow preservation
     * - Proper cleanup and exit handling
     */
    void visit(BlockStatement& node) override;
    
    /**
     * @brief Generates LLVM IR for if statements
     * @param node If statement with condition and branches
     * 
     * Generates conditional control flow with basic blocks:
     * - Condition evaluation and boolean conversion
     * - Basic block creation for then/else branches
     * - Conditional branch instruction
     * - Merge block for control flow continuation
     * - Phi nodes for value merging when needed
     */
    void visit(IfStatement& node) override;
    
    /**
     * @brief Generates LLVM IR for while statements
     * @param node While loop statement with condition and body
     * 
     * Generates loop control flow with basic blocks:
     * - Loop header block for condition evaluation
     * - Loop body block for statement execution
     * - Exit block for loop termination
     * - Unconditional and conditional branch instructions
     * - Proper handling of loop variables and phi nodes
     */
    void visit(WhileStatement& node) override;
    
    /**
     * @brief Generates LLVM IR for return statements
     * @param node Return statement with optional value
     * 
     * Generates function return instructions:
     * - Return value evaluation and type checking
     * - Return instruction generation (ret or ret void)
     * - Function exit validation
     * - Cleanup of local resources
     */
    void visit(ReturnStatement& node) override;
    
    /**
     * @brief Generates LLVM IR for expression statements
     * @param node Expression statement executing expression for side effects
     * 
     * Evaluates expressions used as statements:
     * - Expression evaluation for side effects
     * - Result value discarding
     * - Proper handling of function calls and assignments
     */
    void visit(ExpressionStatement& node) override;
    
    /**
     * @brief Generates LLVM IR for the program root node
     * @param node Program node containing all top-level statements
     * 
     * Handles program-level code generation:
     * - Module initialization and setup
     * - Global scope symbol table initialization
     * - Top-level statement processing in order
     * - Module validation and finalization
     * - Entry point setup for executable programs
     */
    void visit(Program& node) override;

private:
    // ======================== EXPRESSION EVALUATION STATE ========================
    
    /**
     * @brief Current LLVM value result from expression evaluation
     * 
     * This member variable holds the result of the most recent expression
     * evaluation during AST traversal. It serves as a communication mechanism
     * between visitor methods:
     * 
     * - Expression nodes store their result in currentValue
     * - Parent nodes retrieve results from currentValue
     * - Enables composition of complex expressions
     * - Simplifies the visitor pattern implementation
     * 
     * The value is updated by each expression visitor method and consumed
     * by statements or parent expressions that need the computed value.
     */
    llvm::Value* currentValue;
    
    // ======================== ERROR HANDLING ========================
    
    /**
     * @brief Reports code generation errors and terminates compilation
     * @param message Error message describing the problem
     * 
     * Provides centralized error reporting for code generation issues:
     * - Type errors during IR generation
     * - Invalid operations or expressions
     * - Symbol resolution failures
     * - LLVM backend errors
     * 
     * The function outputs the error message and terminates the compilation
     * process to prevent generation of invalid or corrupted IR.
     */
    void error(const std::string& message) const;

}; // Ast Visitor

} // namespace emlang

#endif // EM_CODEGEN_H
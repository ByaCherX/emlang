//===--- value_map.h - LLVM Value Mapping -----------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Value mapping for AST-to-LLVM code generation
//
// This file contains the value mapping infrastructure for converting
// EMLang AST nodes to LLVM values and managing symbol tables.
//===----------------------------------------------------------------------===//

#ifndef EM_CODEGEN_VALUE_MAP_H
#define EM_CODEGEN_VALUE_MAP_H

#pragma once

#include <emlang_export.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

// Forward declarations
namespace llvm {
    class Value;
    class Function;
}

#include <map>
#include <string>

namespace emlang {
namespace codegen {

/**
 * @class ValueMap
 * @brief Manages AST-to-LLVM value mapping and symbol tables
 * 
 * ValueMap provides symbol table management for code generation,
 * tracking variable names, their LLVM values, and EMLang types.
 * This enables proper type handling for multi-level pointers and
 * complex expressions.
 */
class EMLANG_API ValueMap {
public:
    /**
     * @brief Default constructor
     */
    ValueMap();

    /**
     * @brief Default destructor
     */
    ~ValueMap() = default;

    /******************** VARIABLE MANAGEMENT ********************/

    /**
     * @brief Registers a variable with its LLVM value and EMLang type
     * @param name Variable name
     * @param value LLVM value (alloca or global variable)
     * @param type EMLang type string
     */
    void addVariable(const std::string& name, llvm::Value* value, const std::string& type);

    /**
     * @brief Looks up a variable's LLVM value
     * @param name Variable name
     * @return LLVM value pointer or nullptr if not found
     */
    llvm::Value* getVariable(const std::string& name) const;

    /**
     * @brief Looks up a variable's EMLang type
     * @param name Variable name
     * @return EMLang type string or empty string if not found
     */
    std::string getVariableType(const std::string& name) const;

    /**
     * @brief Checks if a variable exists in the symbol table
     * @param name Variable name
     * @return true if variable exists, false otherwise
     */
    bool hasVariable(const std::string& name) const;

    /**
     * @brief Removes a variable from the symbol table
     * @param name Variable name
     */
    void removeVariable(const std::string& name);

    /******************** FUNCTION MANAGEMENT ********************/

    /**
     * @brief Registers a function with its LLVM function
     * @param name Function name
     * @param function LLVM function pointer
     */
    void addFunction(const std::string& name, llvm::Function* function);

    /**
     * @brief Looks up a function's LLVM function
     * @param name Function name
     * @return LLVM function pointer or nullptr if not found
     */
    llvm::Function* getFunction(const std::string& name) const;

    /**
     * @brief Checks if a function exists in the symbol table
     * @param name Function name
     * @return true if function exists, false otherwise
     */
    bool hasFunction(const std::string& name) const;

    /**
     * @brief Removes a function from the symbol table
     * @param name Function name
     */
    void removeFunction(const std::string& name);

    /******************** SCOPE MANAGEMENT ********************/

    /**
     * @brief Saves current variable scope state
     * @return Copy of current variable map for restoration
     */
    std::map<std::string, llvm::Value*> saveScope() const;

    /**
     * @brief Restores previous variable scope state
     * @param savedScope Previously saved variable map
     */
    void restoreScope(const std::map<std::string, llvm::Value*>& savedScope);

    /**
     * @brief Clears all variables (typically for new function scope)
     */
    void clearVariables();

    /**
     * @brief Clears all symbols (variables and functions)
     */
    void clearAll();

    /******************** TYPE MAPPING ********************/

    /**
     * @brief Maps EMLang type name to LLVM type
     * @param typeName EMLang type name
     * @param contextManager LLVM context manager for type creation
     * @return LLVM type pointer or nullptr if unknown
     */
    llvm::Type* getLLVMType(const std::string& typeName, class ContextManager& contextManager);

    /**
     * @brief Creates LLVM pointer type for the given base type
     * @param baseTypeName EMLang base type name
     * @param contextManager LLVM context manager
     * @return LLVM pointer type
     */
    llvm::Type* getPointerType(const std::string& baseTypeName, class ContextManager& contextManager);

    /******************** POINTER TYPE HELPERS ********************/

    /**
     * @brief Gets element type from pointer value
     * @param pointerValue LLVM pointer value
     * @param sourceType EMLang source type string
     * @param contextManager LLVM context manager
     * @return LLVM type of pointed-to element
     */
    llvm::Type* getElementTypeFromPointer(llvm::Value* pointerValue, const std::string& sourceType, class ContextManager& contextManager);

    /**
     * @brief Gets pointee type from pointer type string
     * @param pointerType EMLang pointer type string
     * @return EMLang type of pointed-to element
     */
    std::string getPointeeType(const std::string& pointerType);

private:
    std::map<std::string, llvm::Value*> namedValues;
    std::map<std::string, std::string> namedTypes;
    std::map<std::string, llvm::Function*> functions;
};

} // namespace codegen
} // namespace emlang

#endif // EM_CODEGEN_VALUE_MAP_H

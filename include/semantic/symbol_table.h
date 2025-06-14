//===--- symbol_table.h - Symbol Table --------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Semantic Symbol Table
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_SYMBOL_TABLE_H
#define EM_LANG_SYMBOL_TABLE_H

#include "semantic/semantic_core.h"
#include <string>
#include <map>
#include <memory>

namespace emlang {

/**
 * @class SymbolTable
 * @brief Manages symbol table operations and scope management
 * 
 * The SymbolTable class provides additional utility functions for
 * managing symbols and scopes beyond the basic Scope class functionality.
 * This class can be extended for more advanced symbol table operations.
 */
class EMLANG_API SymbolTable {
public:
    /**
     * @brief Creates a new global scope
     * @return Unique pointer to the new global scope
     */
    static std::unique_ptr<Scope> createGlobalScope();
    
    /**
     * @brief Creates a new scope with the given parent
     * @param parent Parent scope
     * @return Unique pointer to the new scope
     */
    static std::unique_ptr<Scope> createScope(Scope* parent);
    
    /**
     * @brief Validates a symbol definition in the given scope
     * @param scope The scope to validate in
     * @param name Symbol name
     * @param type Symbol type
     * @param isConst Whether symbol is constant
     * @param isFunc Whether symbol is a function
     * @return true if symbol can be defined, false otherwise
     */
    static bool validateSymbolDefinition(
        Scope* scope,
        const std::string& name,
        const std::string& type,
        bool isConst,
        bool isFunc
    );
    
};

} // namespace emlang

#endif // EM_LANG_SYMBOL_TABLE_H

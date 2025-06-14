
#include "semantic/symbol_table.h"
#include <iostream>

namespace emlang {

std::unique_ptr<Scope> SymbolTable::createGlobalScope() {
    return std::make_unique<Scope>(nullptr);
}

std::unique_ptr<Scope> SymbolTable::createScope(Scope* parent) {
    return std::make_unique<Scope>(parent);
}

bool SymbolTable::validateSymbolDefinition(
    Scope* scope,
    const std::string& name,
    const std::string& type,
    bool isConst,
    bool isFunc
) {
    if (!scope) {
        return false;
    }
    
    // Check if symbol already exists in current scope
    if (scope->existsInCurrentScope(name)) {
        return false;
    }
    
    // Additional validation logic can be added here
    // For example, checking for reserved keywords, type validity, etc.
    
    return true;
}

} // namespace emlang

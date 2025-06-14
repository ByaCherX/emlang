// Semantic Analyzer implementation
#include "semantic/semantic_core.h"
#include <iostream>

namespace emlang {

// Scope implementation
Scope::Scope(Scope* parent) : parent(parent) {}

bool Scope::define(const std::string& name, const std::string& type, bool isConst, bool isFunc, size_t line, size_t column) {
    if (symbols.find(name) != symbols.end()) {
        return false; // Symbol already exists in this scope
    }
    
    symbols[name] = std::make_unique<Symbol>(name, type, isConst, isFunc, line, column);
    return true;
}

Symbol* Scope::lookup(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    
    if (parent) {
        return parent->lookup(name);
    }
    
    return nullptr;
}

bool Scope::exists(const std::string& name) {
    return lookup(name) != nullptr;
}

bool Scope::existsInCurrentScope(const std::string& name) {
    return symbols.find(name) != symbols.end();
}

Scope* Scope::getParent() const {
    return parent;
}


} // namespace emlang

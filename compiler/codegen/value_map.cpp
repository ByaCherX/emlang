//===--- value_map.cpp - LLVM Value Mapping implementation ----------------===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of value mapping for AST-to-LLVM code generation
//===----------------------------------------------------------------------===//

#include "codegen/value_map.h"
#include "codegen/context.h"

// Disable LLVM warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4624) // destructor was implicitly deleted
    #pragma warning(disable: 4244) // conversion warnings
    #pragma warning(disable: 4267) // size_t conversion warnings
#endif

#include <llvm/IR/Type.h>

// Re-enable warnings
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace emlang {
namespace codegen {

/// Constructor
ValueMap::ValueMap() {};

/******************** TYPE MAPPING ********************/

llvm::Type* ValueMap::getLLVMType(const std::string& typeName, ContextManager& contextManager) {
    llvm::LLVMContext& ctx = contextManager.getContext();


    // Handle pointer types
    if (typeName.find('*') != std::string::npos) {
        return getPointerType(typeName, contextManager);
    }

    // Handle basic types
    if (typeName == "i8") return llvm::Type::getInt8Ty(ctx);
    if (typeName == "i16") return llvm::Type::getInt16Ty(ctx);
    if (typeName == "i32") return llvm::Type::getInt32Ty(ctx);
    if (typeName == "i64") return llvm::Type::getInt64Ty(ctx);
    if (typeName == "isize") return llvm::Type::getIntNTy(ctx, sizeof(void*) * 8);
    
    if (typeName == "u8") return llvm::Type::getInt8Ty(ctx);
    if (typeName == "u16") return llvm::Type::getInt16Ty(ctx);
    if (typeName == "u32") return llvm::Type::getInt32Ty(ctx);
    if (typeName == "u64") return llvm::Type::getInt64Ty(ctx);
    if (typeName == "usize") return llvm::Type::getIntNTy(ctx, sizeof(void*) * 8);
    
    if (typeName == "f32") return llvm::Type::getFloatTy(ctx);
    if (typeName == "f64") return llvm::Type::getDoubleTy(ctx);
    
    if (typeName == "bool") return llvm::Type::getInt1Ty(ctx);
    if (typeName == "char") return llvm::Type::getInt8Ty(ctx);
    if (typeName == "str") return llvm::PointerType::get(ctx, 0);
    if (typeName == "unit" || typeName == "()") return llvm::Type::getVoidTy(ctx);
    
    // Default to number type for unrecognized numeric types
    if (typeName == "number") return llvm::Type::getDoubleTy(ctx);
    if (typeName == "string") return llvm::PointerType::get(ctx, 0);
    
    return nullptr;
}

llvm::Type* ValueMap::getPointerType(const std::string& baseTypeName, ContextManager& contextManager) {
    // Extract base type (remove *)
    std::string cleanType = getPointeeType(baseTypeName);
    
    llvm::Type* baseType = getLLVMType(cleanType, contextManager);
    if (baseType) {
        return llvm::PointerType::get(baseType, 0);
    }

    // Default to i8* for unknown types
    return llvm::PointerType::get(contextManager.getContext(), 0);
}

/******************** POINTER TYPE HELPERS ********************/

llvm::Type* ValueMap::getElementTypeFromPointer(
    llvm::Value* pointerValue,
    const std::string& sourceType,
    ContextManager& contextManager
) {
    // In LLVM 20+ with opaque pointers, we need to determine type from context
    // Try to determine from source type
    std::string pointeeType = getPointeeType(sourceType);
    llvm::Type* elementType = getLLVMType(pointeeType, contextManager);
    
    // If we couldn't determine the type, default to i8
    if (!elementType) {
        elementType = llvm::Type::getInt8Ty(contextManager.getContext());
    }
    return elementType;
}

std::string ValueMap::getPointeeType(const std::string& pointerType) {
    size_t starPos = pointerType.find('*');
    if (starPos != std::string::npos) {
        return pointerType.substr(0, starPos);
    }
    return pointerType;
}

/******************** VARIABLE MANAGEMENT ********************/

void ValueMap::addVariable(const std::string& name, llvm::Value* value, const std::string& type) {
    namedValues[name] = value;
    namedTypes[name] = type;
}

llvm::Value* ValueMap::getVariable(const std::string& name) const {
    auto it = namedValues.find(name);
    return (it != namedValues.end()) ? it->second : nullptr;
}

std::string ValueMap::getVariableType(const std::string& name) const {
    auto it = namedTypes.find(name);
    return (it != namedTypes.end()) ? it->second : "";
}

bool ValueMap::hasVariable(const std::string& name) const {
    return namedValues.find(name) != namedValues.end();
}

void ValueMap::removeVariable(const std::string& name) {
    namedValues.erase(name);
    namedTypes.erase(name);
}

/******************** FUNCTION MANAGEMENT ********************/

void ValueMap::addFunction(const std::string& name, llvm::Function* function) {
    functions[name] = function;
}

llvm::Function* ValueMap::getFunction(const std::string& name) const {
    auto it = functions.find(name);
    return (it != functions.end()) ? it->second : nullptr;
}

bool ValueMap::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

void ValueMap::removeFunction(const std::string& name) {
    functions.erase(name);
}

/******************** SCOPE MANAGEMENT ********************/

std::map<std::string, llvm::Value*> ValueMap::saveScope() const {
    return namedValues;
}

void ValueMap::restoreScope(const std::map<std::string, llvm::Value*>& savedScope) {
    namedValues = savedScope;
    // Note: We're not restoring types here, which might be needed in some cases
}

void ValueMap::clearVariables() {
    namedValues.clear();
    namedTypes.clear();
}

void ValueMap::clearAll() {
    namedValues.clear();
    namedTypes.clear();
    functions.clear();
}

} // namespace codegen
} // namespace emlang


#include "semantic/type_checker.h"
#include <iostream>

namespace emlang {

// Type checking utility methods implementation

bool TypeChecker::isPrimitiveType(const std::string& type) {
    return type == "int8" || type == "int16" || type == "int32" || type == "int64" ||
           type == "uint8" || type == "uint16" || type == "uint32" || type == "uint64" ||
           type == "float" || type == "double" || type == "bool" || type == "char" ||
           type == "str" || type == "void" || type == "number" || type == "boolean" || type == "string";
}

bool TypeChecker::isNumericType(const std::string& type) {
    // Legacy types
    if (type == "number" || type == "int") {
        return true;
    }
    
    // C-style signed integer types
    if (type == "int8" || type == "int16" || type == "int32" || type == "int64" || type == "isize") {
        return true;
    }
    
    // C-style unsigned integer types
    if (type == "uint8" || type == "uint16" || type == "uint32" || type == "uint64" || type == "usize") {
        return true;
    }
    
    // C-style floating point types
    if (type == "float" || type == "double") {
        return true;
    }
    
    return false;
}

bool TypeChecker::isBooleanType(const std::string& type) {
    return type == "boolean" || type == "bool";
}

bool TypeChecker::isStringType(const std::string& type) {
    return type == "string" || type == "str";
}

bool TypeChecker::isCharType(const std::string& type) {
    return type == "char";
}

bool TypeChecker::isSignedInteger(const std::string& type) {
    return type == "int8" || type == "int16" || type == "int32" || 
           type == "int64" || type == "isize" || type == "int" || type == "number";
}

bool TypeChecker::isUnsignedInteger(const std::string& type) {
    return type == "uint8" || type == "uint16" || type == "uint32" || 
           type == "uint64" || type == "usize";
}

bool TypeChecker::isFloatingPoint(const std::string& type) {
    return type == "float" || type == "double";
}

bool TypeChecker::isUnitType(const std::string& type) {
    return type == "()" || type == "void";
}

bool TypeChecker::isPointerType(const std::string& type) {
    return type.length() > 0 && type.back() == '*';
}

std::string TypeChecker::getPointerBaseType(const std::string& pointerType) {
    if (!isPointerType(pointerType)) {
        return pointerType;
    }
    
    // Remove one level of pointer indirection
    return pointerType.substr(0, pointerType.length() - 1);
}

std::string TypeChecker::makePointerType(const std::string& baseType) {
    return baseType + "*";
}

bool TypeChecker::isCompatibleType(const std::string& expected, const std::string& actual) {
    // Check for exact match
    if (expected == actual) return true;
    
    // Check for implicit conversion
    if (canImplicitlyConvert(actual, expected)) return true;
    
    // Special case: allow "number" literals to be assigned to any numeric type
    if (actual == "number" && isNumericType(expected)) return true;
    
    // Special case: allow "boolean" literals to be assigned to "bool" type
    if (actual == "boolean" && expected == "bool") return true;
    
    // Special case: allow "char" literals to be assigned to "char" type
    if (actual == "char" && expected == "char") return true;
    
    // Special case: allow "string" literals to be assigned to "str" type
    if (actual == "string" && expected == "str") return true;
    
    // Special case: allow "string" literals to be assigned to "char" (single character)
    if (actual == "string" && expected == "char") return true;
    
    // Special case: allow "null" literals to be assigned to any pointer type
    if (actual == "null" && isPointerType(expected)) return true;
    if (expected == "null" && isPointerType(actual)) return true;
    
    return false;
}

bool TypeChecker::canImplicitlyConvert(const std::string& from, const std::string& to) {
    if (from == to) return true;
    
    // Allow "number" literals to be converted to any numeric type
    if (from == "number" && isNumericType(to)) return true;
    
    // Implicit widening conversions for integers
    if (from == "int8" && (to == "int16" || to == "int32" || to == "int64")) return true;
    if (from == "int16" && (to == "int32" || to == "int64")) return true;
    if (from == "int32" && to == "int64") return true;
    
    if (from == "uint8" && (to == "uint16" || to == "uint32" || to == "uint64")) return true;
    if (from == "uint16" && (to == "uint32" || to == "uint64")) return true;
    if (from == "uint32" && to == "uint64") return true;
    
    // Implicit conversion for floating point
    if (from == "float" && to == "double") return true;
    
    // Legacy compatibility (for backward compatibility with old naming)
    if (from == "int" && to == "int32") return true;
    if (from == "i8" && to == "int8") return true;
    if (from == "i16" && to == "int16") return true;
    if (from == "i32" && to == "int32") return true;
    if (from == "i64" && to == "int64") return true;
    
    // Boolean compatibility
    if (from == "boolean" && to == "bool") return true;
    
    // String compatibility
    if (from == "string" && to == "str") return true;
    
    // Null can be implicitly converted to any pointer type
    if (from == "null" && isPointerType(to)) return true;
    
    return false;
}

std::string TypeChecker::getCommonType(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return type1;
    
    // If one can be implicitly converted to the other, use the target type
    if (canImplicitlyConvert(type1, type2)) return type2;
    if (canImplicitlyConvert(type2, type1)) return type1;
    
    // For numeric operations, promote to larger type
    if (isSignedInteger(type1) && isSignedInteger(type2)) {
        if (type1 == "int64" || type2 == "int64") return "int64";
        if (type1 == "int32" || type2 == "int32") return "int32";
        if (type1 == "int16" || type2 == "int16") return "int16";
        return "int8";
    }
    
    if (isUnsignedInteger(type1) && isUnsignedInteger(type2)) {
        if (type1 == "uint64" || type2 == "uint64") return "uint64";
        if (type1 == "uint32" || type2 == "uint32") return "uint32";
        if (type1 == "uint16" || type2 == "uint16") return "uint16";
        return "uint8";
    }
    
    if (isFloatingPoint(type1) && isFloatingPoint(type2)) {
        if (type1 == "double" || type2 == "double") return "double";
        return "float";
    }
    
    // Handle pointer and null compatibility
    if ((isPointerType(type1) && type2 == "null") || (type1 == "null" && isPointerType(type2))) {
        return isPointerType(type1) ? type1 : type2;
    }
    
    // No common type found
    return "";
}

std::string TypeChecker::promoteNumericTypes(const std::string& type1, const std::string& type2) {
    // Simple promotion rules - return the "larger" type
    if (type1 == "double" || type2 == "double") return "double";
    if (type1 == "float" || type2 == "float") return "float";
    if (type1 == "int64" || type2 == "int64") return "int64";
    if (type1 == "int32" || type2 == "int32") return "int32";
    if (type1 == "int16" || type2 == "int16") return "int16";
    if (type1 == "int8" || type2 == "int8") return "int8";
    
    // Default fallback
    return type1;
}

bool TypeChecker::areTypesCompatible(const std::string& type1, const std::string& type2) {
    return isCompatibleType(type1, type2);
}

} // namespace emlang

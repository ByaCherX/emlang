//===--- type_checker.h - Sem Type Checker ----------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Semantic Type Checker
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_TYPE_CHECKER_H
#define EM_LANG_TYPE_CHECKER_H

#include "semantic.h"
#include <string>

namespace emlang {

/**
 * @class TypeChecker
 * @brief Handles type checking and type inference for EMLang expressions
 * 
 * The TypeChecker class provides comprehensive type analysis functionality
 * for the EMLang semantic analyzer. It handles type compatibility checking,
 * type inference, and type validation operations.
 */
class EMLANG_API TypeChecker {
public:
    /**
     * @brief Checks if two types are compatible for assignment or comparison
     * @param expected The expected/target type
     * @param actual The actual/source type
     * @return true if types are compatible, false otherwise
     */
    static bool isCompatibleType(const std::string& expected, const std::string& actual);
    
    /**
     * @brief Checks if a type is a numeric type (integer or floating-point)
     * @param type The type string to check
     * @return true if type is numeric, false otherwise
     */
    static bool isNumericType(const std::string& type);
    
    /**
     * @brief Checks if a type is the boolean type
     * @param type The type string to check
     * @return true if type is bool, false otherwise
     */
    static bool isBooleanType(const std::string& type);
    
    /**
     * @brief Checks if a type is a pointer type
     * @param type The type string to check
     * @return true if type is a pointer (ends with '*'), false otherwise
     */
    static bool isPointerType(const std::string& type);
    
    /**
     * @brief Extracts the base type from a pointer type
     * @param pointerType The pointer type (e.g., "int32*")
     * @return The base type (e.g., "int32")
     */
    static std::string getPointerBaseType(const std::string& pointerType);
    
    /**
     * @brief Creates a pointer type from a base type
     * @param baseType The base type (e.g., "int32")
     * @return The pointer type (e.g., "int32*")
     */
    static std::string makePointerType(const std::string& baseType);
    
    /**
     * @brief Checks if a type is the string type
     * @param type The type string to check
     * @return true if type is str, false otherwise
     */
    static bool isStringType(const std::string& type);
    
    /**
     * @brief Checks if a type is a signed integer type
     * @param type The type string to check
     * @return true if type is signed integer, false otherwise
     */
    static bool isSignedInteger(const std::string& type);
    
    /**
     * @brief Checks if a type is an unsigned integer type
     * @param type The type string to check
     * @return true if type is unsigned integer, false otherwise
     */
    static bool isUnsignedInteger(const std::string& type);
    
    /**
     * @brief Checks if a type is a floating-point type
     * @param type The type string to check
     * @return true if type is floating-point, false otherwise
     */
    static bool isFloatingPoint(const std::string& type);
    
    /**
     * @brief Checks if a type is the character type
     * @param type The type string to check
     * @return true if type is char, false otherwise
     */
    static bool isCharType(const std::string& type);
    
    /**
     * @brief Checks if a type is the unit type (void equivalent)
     * @param type The type string to check
     * @return true if type represents no value, false otherwise
     */
    static bool isUnitType(const std::string& type);
    
    /**
     * @brief Checks if a type is a built-in primitive type
     * @param type The type string to check
     * @return true if type is primitive, false otherwise
     */
    static bool isPrimitiveType(const std::string& type);
    
    /**
     * @brief Checks if a type can be implicitly converted to another type
     * @param from The source type
     * @param to The target type
     * @return true if implicit conversion is allowed, false otherwise
     */
    static bool canImplicitlyConvert(const std::string& from, const std::string& to);
    
    /**
     * @brief Determines the common type for binary operations
     * @param type1 First operand type
     * @param type2 Second operand type
     * @return The result type for the operation
     */
    static std::string getCommonType(const std::string& type1, const std::string& type2);
    
    /**
     * @brief Promotes numeric types for arithmetic operations
     * @param type1 First operand type
     * @param type2 Second operand type
     * @return The promoted result type
     */
    static std::string promoteNumericTypes(const std::string& type1, const std::string& type2);
    
    /**
     * @brief Checks if two types are compatible (alternative name for isCompatibleType)
     * @param type1 First type
     * @param type2 Second type
     * @return true if types are compatible
     */
    static bool areTypesCompatible(const std::string& type1, const std::string& type2);
};

} // namespace emlang

#endif // EM_LANG_TYPE_CHECKER_H

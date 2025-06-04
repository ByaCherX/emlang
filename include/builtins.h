#ifndef EMLANG_BUILTINS_H
#define EMLANG_BUILTINS_H

#pragma once

#include <string>
#include <vector>
#include <map>

namespace emlang {

/**
 * @brief Structure representing a built-in function parameter
 */
struct BuiltinParameter {
    std::string name;
    std::string type;
    
    BuiltinParameter(const std::string& n, const std::string& t) : name(n), type(t) {}
};

/**
 * @brief Structure representing a built-in function signature
 */
struct BuiltinFunction {
    std::string name;
    std::vector<BuiltinParameter> parameters;
    std::string returnType;
    std::string linkName; // The actual C function name to link against
    
    BuiltinFunction(const std::string& n, const std::vector<BuiltinParameter>& params, 
                   const std::string& ret, const std::string& link = "")
        : name(n), parameters(params), returnType(ret), linkName(link.empty() ? n : link) {}
};

/**
 * @brief Get all built-in functions available in EMLang
 * @return Map of function names to their signatures
 */
std::map<std::string, BuiltinFunction> getBuiltinFunctions();

/**
 * @brief Check if a function name is a built-in function
 * @param name Function name to check
 * @return True if it's a built-in function
 */
bool isBuiltinFunction(const std::string& name);

/**
 * @brief Get the signature of a built-in function
 * @param name Function name
 * @return Pointer to BuiltinFunction or nullptr if not found
 */
const BuiltinFunction* getBuiltinFunction(const std::string& name);

} // namespace emlang

#endif // EMLANG_BUILTINS_H

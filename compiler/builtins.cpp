#include "../include/builtins.h"
#include <map>
#include <string>

namespace emlang {

std::map<std::string, BuiltinFunction> getBuiltinFunctions() {
    static std::map<std::string, BuiltinFunction> builtins = {
        // I/O Functions
        {"emlang_print", BuiltinFunction("emlang_print", 
            {BuiltinParameter("str", "string")}, "void", "emlang_print_str")},
        {"emlang_print_int", BuiltinFunction("emlang_print_int", 
            {BuiltinParameter("value", "int32")}, "void")},
        {"emlang_print_str", BuiltinFunction("emlang_print_str", 
            {BuiltinParameter("str", "string")}, "void")},
        {"emlang_print_char", BuiltinFunction("emlang_print_char", 
            {BuiltinParameter("c", "char")}, "void")},
        {"emlang_print_float", BuiltinFunction("emlang_print_float", 
            {BuiltinParameter("value", "float")}, "void")},
        {"emlang_println", BuiltinFunction("emlang_println", {}, "void")},
        
        // Input Functions
        {"emlang_read_int", BuiltinFunction("emlang_read_int", {}, "int32")},
        {"emlang_read_char", BuiltinFunction("emlang_read_char", {}, "char")},
        {"emlang_read_float", BuiltinFunction("emlang_read_float", {}, "float")},
        
        // Memory Functions
        {"emlang_malloc", BuiltinFunction("emlang_malloc", 
            {BuiltinParameter("size", "int32")}, "void*")},
        {"emlang_free", BuiltinFunction("emlang_free", 
            {BuiltinParameter("ptr", "void*")}, "void")},
        {"emlang_memset", BuiltinFunction("emlang_memset", 
            {BuiltinParameter("ptr", "void*"), BuiltinParameter("value", "int32"), 
             BuiltinParameter("size", "int32")}, "void*")},
        
        // String Functions
        {"emlang_strlen", BuiltinFunction("emlang_strlen", 
            {BuiltinParameter("str", "string")}, "int32")},
        {"emlang_strcmp", BuiltinFunction("emlang_strcmp", 
            {BuiltinParameter("str1", "string"), BuiltinParameter("str2", "string")}, "int32")},
        
        // Math Functions
        {"emlang_pow", BuiltinFunction("emlang_pow", 
            {BuiltinParameter("base", "int32"), BuiltinParameter("exp", "int32")}, "int32")},
        {"emlang_sqrt", BuiltinFunction("emlang_sqrt", 
            {BuiltinParameter("x", "int32")}, "int32")},
        {"emlang_sin", BuiltinFunction("emlang_sin", 
            {BuiltinParameter("x", "double")}, "double")},
        {"emlang_cos", BuiltinFunction("emlang_cos", 
            {BuiltinParameter("x", "double")}, "double")},
        {"emlang_abs", BuiltinFunction("emlang_abs", 
            {BuiltinParameter("x", "int32")}, "int32")},
        {"emlang_min", BuiltinFunction("emlang_min", 
            {BuiltinParameter("a", "int32"), BuiltinParameter("b", "int32")}, "int32")},
        {"emlang_max", BuiltinFunction("emlang_max", 
            {BuiltinParameter("a", "int32"), BuiltinParameter("b", "int32")}, "int32")},
    };
    
    return builtins;
}

bool isBuiltinFunction(const std::string& name) {
    auto builtins = getBuiltinFunctions();
    return builtins.find(name) != builtins.end();
}

const BuiltinFunction* getBuiltinFunction(const std::string& name) {
    static auto builtins = getBuiltinFunctions();
    auto it = builtins.find(name);
    return (it != builtins.end()) ? &it->second : nullptr;
}

} // namespace emlang

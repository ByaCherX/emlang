# AST Module CMakeLists.txt
# Static library for AST classes

cmake_minimum_required(VERSION 3.15)

# AST source files
set(AST_SOURCES
    ast_base.cpp
    expr.cpp
    stmt.cpp
    decl.cpp
    visitor.cpp
    dumper.cpp
)

# AST header files
set(AST_HEADERS
    ${CMAKE_SOURCE_DIR}/include/ast.h
    ${CMAKE_SOURCE_DIR}/include/ast/ast_base.h
    ${CMAKE_SOURCE_DIR}/include/ast/expr.h
    ${CMAKE_SOURCE_DIR}/include/ast/stmt.h
    ${CMAKE_SOURCE_DIR}/include/ast/decl.h
    ${CMAKE_SOURCE_DIR}/include/ast/visitor.h
    ${CMAKE_SOURCE_DIR}/include/ast/dumper.h
)

# AST object
add_library(emlang_ast OBJECT ${AST_SOURCES} ${AST_HEADERS})

# Include directories
target_include_directories(emlang_ast PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Export symbols when building DLL
target_compile_definitions(emlang_ast PRIVATE EMLANG_EXPORTS)


# Set target properties
set_target_properties(emlang_ast PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Compiler-specific flags
if(MSVC)
    target_compile_options(emlang_ast PRIVATE /W4)
else()
    target_compile_options(emlang_ast PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation
install(TARGETS emlang_ast
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

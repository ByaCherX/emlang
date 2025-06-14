# EMLang Semantic Analysis Module
# This module handles semantic analysis, type checking, and symbol table management

# Collect all semantic source files
set(SEMANTIC_SOURCES
    semantic.cpp
    analyzer.cpp
    type_checker.cpp
    symbol_table.cpp
    semantic_error.cpp
)

# Collect all semantic header files
set(SEMANTIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/semantic/semantic.h
    ${CMAKE_SOURCE_DIR}/include/semantic/analyzer.h
    ${CMAKE_SOURCE_DIR}/include/semantic/type_checker.h
    ${CMAKE_SOURCE_DIR}/include/semantic/symbol_table.h
    ${CMAKE_SOURCE_DIR}/include/semantic/semantic_error.h
)

# Create the semantic object
add_library(emlang_semantic OBJECT ${SEMANTIC_SOURCES} ${SEMANTIC_HEADERS})

# Set target properties
set_target_properties(emlang_semantic PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(emlang_semantic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Export symbols when building DLL
target_compile_definitions(emlang_semantic PRIVATE EMLANG_EXPORTS)

# Compiler-specific settings
if(MSVC)
    target_compile_options(emlang_semantic PRIVATE 
        /wd4251  # Disable DLL interface warnings
    )
    target_compile_definitions(emlang_semantic PRIVATE
        _CRT_SECURE_NO_WARNINGS
        EMLANG_SEMANTIC_EXPORTS
    )
else()
    target_compile_options(emlang_semantic PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Installation rules
install(TARGETS emlang_semantic
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${SEMANTIC_HEADERS}
    DESTINATION include/semantic
)

# Add semantic module to global targets
add_dependencies(emlang_semantic emlang_ast emlang_lexer)
# CMakeLists.txt for EMLang Lexer Library
# This creates a object code for the lexer component

cmake_minimum_required(VERSION 3.20)

# Lexer source files
set(LEXER_SOURCES
    lexer.cpp
    token.cpp
)

# Lexer header files
set(LEXER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/lexer.h
    ${CMAKE_SOURCE_DIR}/include/lexer/lexer_core.h
    ${CMAKE_SOURCE_DIR}/include/lexer/token.h
)

# Create the lexer OBJECT library
add_library(emlang_lexer OBJECT ${LEXER_SOURCES} ${LEXER_HEADERS})

# Set include directories
target_include_directories(emlang_lexer PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Export symbols when building DLL
target_compile_definitions(emlang_lexer PRIVATE EMLANG_EXPORTS)

# Set properties for the library
set_target_properties(emlang_lexer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Add compiler-specific flags
if(MSVC)
    target_compile_options(emlang_lexer PRIVATE /W4)
else()
    target_compile_options(emlang_lexer PRIVATE -Wall -Wextra -pedantic)
endif()

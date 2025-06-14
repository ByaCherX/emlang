# Add subdirectories for each component as OBJECT libraries
add_subdirectory(lexer)
add_subdirectory(ast)
add_subdirectory(parser)
add_subdirectory(semantic)
add_subdirectory(codegen)

# EMLang Compiler DLL - uses OBJECT libraries for direct symbol embedding
add_library(emlang_compiler SHARED
    # Built-ins and main compiler interface
    builtins.cpp
    
    # Resource file
    emlang_compiler.rc
    
    # Include all object files from the OBJECT libraries
    $<TARGET_OBJECTS:emlang_lexer>
    $<TARGET_OBJECTS:emlang_ast>
    $<TARGET_OBJECTS:emlang_parser>
    $<TARGET_OBJECTS:emlang_semantic>
    $<TARGET_OBJECTS:emlang_codegen>
)

target_include_directories(emlang_compiler PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/lexer
    ${CMAKE_SOURCE_DIR}/include/ast
    ${CMAKE_SOURCE_DIR}/include/parser
    ${CMAKE_SOURCE_DIR}/include/semantic
    ${CMAKE_SOURCE_DIR}/include/codegen
)

# Link with OBJECT libraries (inheritance of include directories)
target_link_libraries(emlang_compiler 
    # No need to link the object libraries, they're already embedded
)

# DLL export definitions
target_compile_definitions(emlang_compiler PRIVATE EMLANG_EXPORTS)
target_compile_definitions(emlang_compiler PUBLIC EMLANG_DLL)

# Compiler settings
if(MSVC)
    target_compile_options(emlang_compiler PRIVATE
        /wd4624  # destructor was implicitly deleted
        /wd4244  # conversion warnings
        /wd4267  # size_t conversion warnings
        /wd4251  # DLL interface warning for STL containers
    )
endif()

# Add LLVM support if available
if(LLVM_ENABLED)
    target_include_directories(emlang_compiler PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(emlang_compiler PRIVATE ${LLVM_LIBRARIES})
    target_compile_definitions(emlang_compiler PRIVATE LLVM_ENABLED)
    message(STATUS "Linking emlang_compiler with LLVM libraries")
else()
    target_compile_definitions(emlang_compiler PRIVATE NO_LLVM_CODEGEN)
    message(STATUS "Building emlang_compiler without LLVM support")
endif()

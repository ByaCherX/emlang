# Compiler library
add_library(compiler_lib
    lexer/lexer.cpp
    ast/ast.cpp
    parser/parser.cpp
    semantic/semantic.cpp
    codegen/codegen.cpp  # LLVM code generation enabled
)

target_include_directories(compiler_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# LLVM bağlantısı varsa ekle
if(LLVM_FOUND)
    target_link_libraries(compiler_lib ${LLVM_LIBRARIES})
endif()

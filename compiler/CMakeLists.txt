add_subdirectory(ast)
add_library(emlang_compiler SHARED
    lexer/lexer.cpp
    ast/ast.cpp
    parser/parser.cpp
    semantic/semantic.cpp
    codegen/codegen.cpp
    builtins.cpp
    $<TARGET_OBJECTS:emlang_ast>
)

target_include_directories(emlang_compiler PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/ast
)

# DLL export definitions
target_compile_definitions(emlang_compiler PRIVATE EMLANG_EXPORTS)
target_compile_definitions(emlang_compiler PUBLIC EMLANG_DLL)

# MSVC warning'lerini ve linker uyarılarını kapat
if(MSVC)
    target_compile_options(emlang_compiler PRIVATE
        /wd4624  # destructor was implicitly deleted
        /wd4244  # conversion warnings
        /wd4267  # size_t conversion warnings
    )
    
    # Linker uyarılarını kapat
    set_target_properties(emlang_compiler PROPERTIES
        LINK_FLAGS "/ignore:4197 /ignore:4199"
    )
endif()

# Clang için özel ayarlar
if(USE_CLANG)
    target_compile_options(emlang_compiler PRIVATE
        -Wno-unused-command-line-argument
        -Wno-deprecated-declarations
    )
    
    # Windows'ta MSVC uyumluluğu
    if(WIN32)
        target_compile_options(emlang_compiler PRIVATE
            -fms-compatibility-version=19.29
            -fms-extensions
            -fdelayed-template-parsing
        )
        target_compile_definitions(emlang_compiler PRIVATE
            _CRT_SECURE_NO_WARNINGS
        )
    endif()
    
    # Debug modunda ekstra bilgi
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(emlang_compiler PRIVATE
            -fcolor-diagnostics
            -fansi-escape-codes
        )
    endif()
endif()

# LLVM bağlantısı varsa ekle
if(LLVM_FOUND)
    target_include_directories(emlang_compiler PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(emlang_compiler ${LLVM_LIBRARIES})
    target_compile_definitions(emlang_compiler PRIVATE LLVM_ENABLED)
else()
    target_compile_definitions(emlang_compiler PRIVATE NO_LLVM_CODEGEN)
endif()

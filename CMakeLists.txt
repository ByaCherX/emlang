cmake_minimum_required(VERSION 3.20)
project(EMLang VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows DLL export/import symbol'ları için
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# LLVM konfigürasyonu (şimdilik opsiyonel)
find_package(LLVM CONFIG)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    
    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})
    
    llvm_map_components_to_libnames(llvm_libs 
        support core analysis target
        native nativecodegen
        mcjit executionengine
        transformutils scalaropts instcombine ipo
        mc asmprinter
    )
    set(LLVM_LIBRARIES ${llvm_libs})
    set(LLVM_ENABLED TRUE)
else()
    message(STATUS "LLVM not found - building without LLVM support")
    set(LLVM_LIBRARIES "")
    set(LLVM_ENABLED FALSE)
endif()

# Proje include dizinleri
include_directories(include)

# Compiler bileşenleri (DLL olarak)
add_subdirectory(compiler)

# EMLang standard library (static library olarak)
add_subdirectory(library)

# Ana executable
add_executable(emlang src/main.cpp emlang.rc)

# Include directories
target_include_directories(emlang PRIVATE library/include)

# Linkage - artık DLL ve library'e bağlı
target_link_libraries(emlang emlang_compiler emlang_lib)

# MSVC linker uyarılarını kapat
if(MSVC)
    set_target_properties(emlang PROPERTIES
        LINK_FLAGS "/ignore:4197 /ignore:4199"
    )
endif()

# DLL'yi executable yanına kopyala
add_custom_command(TARGET emlang POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:emlang_compiler>
    $<TARGET_FILE_DIR:emlang>
    COMMENT "Copying emlang_compiler.dll to executable directory"
)

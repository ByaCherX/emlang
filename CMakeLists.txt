cmake_minimum_required(VERSION 3.20)
project(EMLang VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# LLVM konfigürasyonu (şimdilik opsiyonel)
find_package(LLVM CONFIG)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    
    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})
      # LLVM linkage
    llvm_map_components_to_libnames(llvm_libs 
        support core irreader
        x86codegen
        x86asmparser
        x86desc
        x86disassembler
        x86info
        mcjit
        native
        executionengine
        interpreter
        runtimedyld
        object
    )
    set(LLVM_LIBRARIES ${llvm_libs})
else()
    message(STATUS "LLVM not found - building without LLVM support")
    set(LLVM_LIBRARIES "")
endif()

# Proje include dizinleri
include_directories(include)

# Compiler bileşenleri
add_subdirectory(compiler)

# Ana executable
add_executable(emlang src/main.cpp)

# Linkage
target_link_libraries(emlang compiler_lib ${LLVM_LIBRARIES})

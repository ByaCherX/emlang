cmake_minimum_required(VERSION 3.20)
project(EMLang VERSION 1.0.0 LANGUAGES CXX)

option(USE_CLANG "Use Clang compiler instead of default" OFF)
option(BUILD_LIBRARY "Build EMLang standard library" OFF)

# Compiler selection (must be before project() call)
if(USE_CLANG)
    # Set environment variables for Clang on Windows
    if(WIN32)
        # Find Clang
        find_program(CLANG_CXX_COMPILER NAMES clang++)
        find_program(CLANG_C_COMPILER NAMES clang)
        
        if(CLANG_CXX_COMPILER AND CLANG_C_COMPILER)
            set(CMAKE_C_COMPILER ${CLANG_C_COMPILER})
            set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
            
            # Use MinGW Makefiles generator (if Ninja is not available)
            find_program(NINJA_EXECUTABLE ninja)
            if(NINJA_EXECUTABLE)
                set(CMAKE_GENERATOR "Ninja" CACHE STRING "CMake generator" FORCE)
                message(STATUS "Using Clang compiler with Ninja generator")
            else()
                set(CMAKE_GENERATOR "MinGW Makefiles" CACHE STRING "CMake generator" FORCE)
                message(STATUS "Using Clang compiler with MinGW Makefiles generator")
            endif()
        else()
            message(FATAL_ERROR "Clang compiler not found! Please install Clang or set USE_CLANG=OFF")
        endif()
    else()
        # Standard Clang on Unix systems
        set(CMAKE_C_COMPILER clang)
        set(CMAKE_CXX_COMPILER clang++)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For Windows DLL export/import symbols
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    # Enable global symbol export for DLL
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    # Set the DLL API calling convention
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# LLVM configuration
find_package(LLVM REQUIRED CONFIG)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    
    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})
    
    # Map LLVM components to libraries - simplified for compatibility
    llvm_map_components_to_libnames(llvm_libs 
        support core analysis target
        native codegen
        
        # Target architectures - needed for InitializeAll*Target* functions
        AArch64 ARM BPF WebAssembly RISCV NVPTX X86

        mcjit executionengine runtimedyld
        
        transformutils scalaropts instcombine ipo
        mc asmprinter asmparser
    )
    set(LLVM_LIBRARIES ${llvm_libs})
    set(LLVM_ENABLED TRUE)
else()
    message(FATAL_ERROR "LLVM not found! Please install LLVM or set LLVM_DIR to the correct path")
    set(LLVM_ENABLED FALSE)
endif()

# Project include directories
include_directories(include)

# Compiler components (as DLL)
add_subdirectory(compiler)

# ===========================
# EMLang Executables
# ===========================

# EMLang main executable
add_executable(emlang src/main.cpp emlang.rc)

# EMLang Check Tool (AST and Token analysis)
add_executable(emlang_check src/emlang_check.cpp)

# ===========================

# Compiler DLL linkage for emlang_check
target_link_libraries(emlang PRIVATE emlang_compiler)
target_link_libraries(emlang_check PRIVATE emlang_compiler)

# Define EMLANG_DLL for importing symbols from emlang_compiler.dll
target_compile_definitions(emlang PRIVATE EMLANG_DLL)
target_compile_definitions(emlang_check PRIVATE EMLANG_DLL)

# Linkage - compiler DLL always, library conditional
if(BUILD_LIBRARY)
    message(STATUS "Building EMLang standard library")
    add_subdirectory(library)

    target_include_directories(emlang PRIVATE library/include)
    target_include_directories(emlang_check PRIVATE library/include)
    target_link_libraries(emlang PRIVATE emlang_lib)
    target_link_libraries(emlang_check PRIVATE emlang_lib)
else()
    message(STATUS "Skipping EMLang standard library build")
endif()

# Copy DLL next to executable
add_custom_command(TARGET emlang POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:emlang_compiler>
    $<TARGET_FILE_DIR:emlang>
    COMMENT "Copying emlang_compiler.dll to executable directory"
)

# Copy DLL for emlang_check as well
add_custom_command(TARGET emlang_check POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:emlang_compiler>
    $<TARGET_FILE_DIR:emlang_check>
    COMMENT "Copying emlang_compiler.dll to emlang_check directory"
)

# Disable MSVC linker warnings
if(MSVC)
    set_target_properties(emlang PROPERTIES
        LINK_FLAGS "/ignore:4197 /ignore:4199"
    )
endif()
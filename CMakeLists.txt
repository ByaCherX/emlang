# Clang compiler selection (cmake .. -DUSE_CLANG=ON)
option(USE_CLANG "Use Clang compiler instead of default" OFF)

# Library build selection (cmake .. -DBUILD_LIBRARY=ON)
option(BUILD_LIBRARY "Build EMLang standard library" ON)

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

cmake_minimum_required(VERSION 3.20)
project(EMLang VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Special compiler flags for Clang
if(USE_CLANG)
    message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
    message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
    
    # General compiler flags for Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    
    # For MSVC compatibility on Windows
    if(WIN32)
        # Use MSVC C++ standard library, in MSVC compatible mode
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-compatibility-version=19.29")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdelayed-template-parsing")
        # Use Visual Studio C runtime
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
    endif()
    
    # Better error messages in debug mode
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics -fansi-escape-codes")
    endif()
endif()

# For Windows DLL export/import symbols
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# LLVM configuration (optional for now)
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

# Project include directories
include_directories(include)

# Compiler components (as DLL)
add_subdirectory(compiler)

# EMLang standard library (optional - as static library)
if(BUILD_LIBRARY)
    message(STATUS "Building EMLang standard library")
    add_subdirectory(library)
    set(LIBRARY_TARGET emlang_lib)
else()
    message(STATUS "Skipping EMLang standard library build")
    set(LIBRARY_TARGET "")
endif()

# ===========================
# EMLang Executables
# ===========================

# EMLang main executable
add_executable(emlang src/main.cpp emlang.rc)

# EMLang Check Tool (AST and Token analysis)
add_executable(emlang_check src/emlang_check.cpp)

# ===========================

# Include directories (conditional)
if(BUILD_LIBRARY)
    target_include_directories(emlang PRIVATE library/include)
    target_include_directories(emlang_check PRIVATE library/include)
endif()

# Compiler DLL linkage for emlang_check
target_link_libraries(emlang_check emlang_compiler)

# Linkage - compiler DLL always, library conditional
if(BUILD_LIBRARY)
    target_link_libraries(emlang emlang_compiler ${LIBRARY_TARGET})
else()
    target_link_libraries(emlang emlang_compiler)
endif()

# Disable MSVC linker warnings
if(MSVC)
    set_target_properties(emlang PROPERTIES
        LINK_FLAGS "/ignore:4197 /ignore:4199"
    )
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

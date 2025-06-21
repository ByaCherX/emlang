# EMLang Codegen Module CMakeLists.txt
# Advanced LLVM OrcJIT v2 Integration with Modular Architecture

# Minimum LLVM version required for OrcJIT v2
set(MINIMUM_LLVM_VERSION "14.0")

# Check LLVM version for OrcJIT v2 support
if(LLVM_FOUND)
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})

    if(LLVM_PACKAGE_VERSION VERSION_LESS ${MINIMUM_LLVM_VERSION})
        message(WARNING "LLVM ${LLVM_PACKAGE_VERSION} found, but OrcJIT v2 requires LLVM ${MINIMUM_LLVM_VERSION}+")
        set(ORCJIT_ENABLED FALSE)
    else()
        set(ORCJIT_ENABLED TRUE)
    endif()
else()
    set(ORCJIT_ENABLED FALSE)
    message(STATUS "LLVM not found")
endif()

# Codegen library source files
set(CODEGEN_SOURCES
    # main codegen entry point
    codegen.cpp

    # Core infrastructure
    value_map.cpp
    context.cpp
    codegen_error.cpp
    #target_config.cpp
      
    # LLVM OrcJIT v2 engine and layers
    #jit_engine.cpp
    #symbol_resolver.cpp
        
    # JIT compilation support
    #module_manager.cpp
    
    # Advanced features
    #target_manager.cpp
    #memory_manager.cpp
    #symbol_interposer.cpp
    
    # Final integration
    #builtins_integration.cpp
)

set(CODEGEN_HEADERS
    # main codegen header
    ${CMAKE_SOURCE_DIR}/include/codegen/codegen.h

    # Core infrastructure headers
    ${CMAKE_SOURCE_DIR}/include/codegen/value_map.h
    ${CMAKE_SOURCE_DIR}/include/codegen/context.h
    ${CMAKE_SOURCE_DIR}/include/codegen/codegen_error.h
    #target_config.h
    
    # LLVM OrcJIT v2 engine and layers headers
    #jit_engine.h
    #symbol_resolver.h

    # JIT compilation support headers
    #module_manager.h
    
    # Advanced features headers
    #target_manager.h
    #memory_manager.h
    #symbol_interposer.h
    
    # Final integration headers
    #builtins_integration.h
)

# Create the codegen library
add_library(emlang_codegen OBJECT ${CODEGEN_SOURCES} ${CODEGEN_HEADERS})

# Include directories
target_include_directories(emlang_codegen PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/codegen
)

# Export symbols when building DLL
target_compile_definitions(emlang_codegen PRIVATE EMLANG_EXPORTS)

# Conditional LLVM linkage and features
if(ORCJIT_ENABLED)
    # LLVM include directories
    target_include_directories(emlang_codegen PRIVATE ${LLVM_INCLUDE_DIRS})
    
    # Extended LLVM components for OrcJIT v2 - simplified for compatibility
    llvm_map_components_to_libnames(llvm_orcjit_libs
        # Core components
        support core analysis target
        
        # JIT and execution engine components
        orcjit orctargetprocess orcshared
        mcjit executionengine runtimedyld
        
        # Code generation components
        transformutils scalaropts instcombine ipo
        mc asmprinter asmparser
        
        # Native target only (automatically detected)
        native
        
        # Additional JIT utilities
        interpreter
    )
    set(LLVM_LIBRARIES ${llvm_orcjit_libs})

    # Link LLVM libraries
    target_link_libraries(emlang_codegen PRIVATE ${llvm_orcjit_libs})
    
    # LLVM definitions and flags
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    target_compile_definitions(emlang_codegen PRIVATE ${LLVM_DEFINITIONS_LIST})
    
    # Enable OrcJIT features
    target_compile_definitions(emlang_codegen PUBLIC
        EMLANG_ORCJIT_ENABLED
        EMLANG_LLVM_VERSION_MAJOR=${LLVM_VERSION_MAJOR}
        EMLANG_LLVM_VERSION_MINOR=${LLVM_VERSION_MINOR}
    )

    message(STATUS "Codegen module configuration:")
    message(STATUS "  LLVM Version: ${LLVM_PACKAGE_VERSION}")
    message(STATUS "  OrcJIT v2: Enabled")
    message(STATUS "  Target Platform: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
endif()

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(emlang_codegen PRIVATE
        EMLANG_PLATFORM_WINDOWS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Additional Windows libraries for JIT
    if(ORCJIT_ENABLED)
        target_link_libraries(emlang_codegen PRIVATE
            kernel32 user32 advapi32 shell32
        )
    endif()
    
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_compile_definitions(emlang_codegen PRIVATE EMLANG_PLATFORM_LINUX)
    
    # Linux libraries for JIT
    if(ORCJIT_ENABLED)
        target_link_libraries(emlang_codegen PRIVATE
            dl pthread rt
        )
    endif()
    
elseif(APPLE)
    # macOS-specific settings
    target_compile_definitions(emlang_codegen PRIVATE EMLANG_PLATFORM_MACOS)
    
    # macOS frameworks for JIT
    if(ORCJIT_ENABLED)
        target_link_libraries(emlang_codegen PRIVATE
            "-framework Foundation"
            "-framework CoreFoundation"
        )
    endif()
endif()

# Compiler-specific flags
if(MSVC)
    # MSVC-specific optimizations and warnings
    target_compile_options(emlang_codegen PRIVATE
        /wd4624  # destructor was implicitly deleted
        /wd4244  # conversion warnings
        /wd4267  # size_t conversion warnings
        /wd4291  # no matching operator delete
    )
    
    # Enable LTCG for release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(emlang_codegen PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE
        )
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang-specific settings
    target_compile_options(emlang_codegen PRIVATE
        -Wno-unused-command-line-argument
        -Wno-deprecated-declarations
        -Wno-unused-variable
    )
    
    # Clang-specific optimizations
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(emlang_codegen PRIVATE
            -flto=thin
            -fuse-ld=lld
        )
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC-specific settings
    target_compile_options(emlang_codegen PRIVATE
        -Wno-unused-variable
        -Wno-unused-function
        -Wno-deprecated-declarations
    )
endif()

# Debug configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(emlang_codegen PRIVATE
        EMLANG_DEBUG_MODE
    )
endif()

# Installation rules
install(TARGETS emlang_codegen
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/codegen/
    DESTINATION include/codegen
    FILES_MATCHING PATTERN "*.h"
)

# Performance profiling support (optional)
option(EMLANG_ENABLE_PROFILING "Enable performance profiling in codegen" OFF)
if(EMLANG_ENABLE_PROFILING)
    target_compile_definitions(emlang_codegen PRIVATE EMLANG_PROFILING_ENABLED)
    
    if(UNIX)
        target_link_libraries(emlang_codegen PRIVATE profiler)
    endif()
endif()

# Testing support
option(EMLANG_BUILD_CODEGEN_TESTS "Build codegen unit tests" OFF)
if(EMLANG_BUILD_CODEGEN_TESTS AND ORCJIT_ENABLED)
    # Test-specific configurations will be added here
    target_compile_definitions(emlang_codegen PRIVATE EMLANG_TESTING_ENABLED)
endif()
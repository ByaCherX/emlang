# EMLang Standard Library CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library source files
set(LIBRARY_SOURCES
    src/math.cpp
    src/io.cpp
    src/string.cpp
    src/memory.cpp
    src/utility.cpp
)

# Library header files
set(LIBRARY_HEADERS
    include/emlang_lib.h
    include/math.h
    include/io.h
    include/string.h
    include/memory.h
    include/utility.h
)

# Create static library
add_library(emlang_lib STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

# Include directories
target_include_directories(emlang_lib PUBLIC 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set C++ standard
target_compile_features(emlang_lib PUBLIC cxx_std_17)

# Windows specific settings
if(WIN32)
    # Ensure C linkage for exported functions
    target_compile_definitions(emlang_lib PRIVATE EMLANG_LIB_EXPORTS)
endif()

# Clang special settings
if(USE_CLANG)
    target_compile_options(emlang_lib PRIVATE
        -Wno-unused-command-line-argument
        -Wno-deprecated-declarations
    )
    
    # Windows MSVC compatibility
    if(WIN32)
        target_compile_options(emlang_lib PRIVATE
            -fms-compatibility-version=19.29
            -fms-extensions
            -fdelayed-template-parsing
        )
        target_compile_definitions(emlang_lib PRIVATE
            _CRT_SECURE_NO_WARNINGS
        )
    endif()
    
    # Debug mode settings
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(emlang_lib PRIVATE
            -fcolor-diagnostics
            -fansi-escape-codes
        )
    endif()
endif()

# Set output directory
set_target_properties(emlang_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Install targets
install(TARGETS emlang_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${LIBRARY_HEADERS} DESTINATION include/emlang)

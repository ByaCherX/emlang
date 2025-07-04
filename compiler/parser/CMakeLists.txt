cmake_minimum_required(VERSION 3.15)

# Parser source files
set(PARSER_SOURCES
    parser.cpp
    parser_error.cpp
)

# Parser header files
set(PARSER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/parser/parser.h
    ${CMAKE_SOURCE_DIR}/include/parser/parser_error.h
)

# Create parser object
add_library(emlang_parser OBJECT ${PARSER_SOURCES} ${PARSER_HEADERS})

# Set include directories for the parser library
target_include_directories(emlang_parser PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Export symbols when building DLL
target_compile_definitions(emlang_parser PRIVATE EMLANG_EXPORTS)

# Set C++ standard
set_target_properties(emlang_parser PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Enable comprehensive warnings
if(MSVC)
    target_compile_options(emlang_parser PRIVATE 
        /W4          # High warning level
        /wd4251      # Disable DLL interface warning for STL types
        /wd4275      # Disable DLL interface warning for base classes
    )
else()
    target_compile_options(emlang_parser PRIVATE 
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter  # Many parsing methods have unused params
    )
endif()

# Install the library and headers
install(TARGETS emlang_parser
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/parser/
    DESTINATION include/parser
    FILES_MATCHING PATTERN "*.h"
)
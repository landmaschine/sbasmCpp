cmake_minimum_required(VERSION 3.10)
project(sbasmCpp LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ASSEMBLER_SOURCES
    "assembler/*.c"
    "assembler/*.cpp"
    "assembler/*.h"
    "assembler/*.hpp"
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${ASSEMBLER_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/assembler
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -O0
    -g
)
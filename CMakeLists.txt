cmake_minimum_required(VERSION 3.18)
project(sbasmCpp LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ASSEMBLER_LIB_SOURCES
    "assembler/Lexer/*.cpp"
    "assembler/Parser/*.cpp"
    "assembler/InstructionEncoder/*.cpp"
    "assembler/*.h"
    "assembler/*.hpp"
)

add_library(assembler_lib STATIC
    ${ASSEMBLER_LIB_SOURCES}
)

target_include_directories(assembler_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/assembler
)

add_executable(${PROJECT_NAME}
  "assembler/main.cpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  assembler_lib
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -g
)

target_link_options(${PROJECT_NAME} PRIVATE 
    -static-libstdc++
    -static-libgcc
    -static
)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

foreach(TEST_FILE lexer_tests.cpp parser_tests.cpp encoder_tests.cpp)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/${TEST_FILE}")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/tests/${TEST_FILE}" "#include <gtest/gtest.h>\n\n// Placeholder for ${TEST_FILE}\n")
    endif()
endforeach()

add_executable(sbasmCpp_tests
    tests/lexer_tests.cpp
    tests/parser_tests.cpp
    tests/encoder_tests.cpp
)

target_link_libraries(sbasmCpp_tests
    gtest_main
    assembler_lib
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(sbasmCpp_tests)


include(CPack)
set(CPACK_PACKAGE_NAME "sbasmCpp")
set(CPACK_PACKAGE_VENDOR "LeonW")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An assembler written in C++, for the qCore architecture")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "sbasmCpp")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

set(CPACK_GENERATOR "NSIS")

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://github.com/landmaschine/sbasmCpp")
set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/landmaschine/sbasmCpp")


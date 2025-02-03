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
cmake_minimum_required(VERSION 3.10)
project(RegexLib
        VERSION 0.1
        DESCRIPTION "Regex Library for C in C"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(str str/str.c str/strbuf.c str/common.c str/fmt.c)
add_library(re re/re.c)
add_library(regex INTERFACE)
target_link_libraries(regex INTERFACE re str)

# Start testing
enable_testing()
set(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests")
set(CODEGEN_DIR "${CMAKE_BINARY_DIR}/tests")

# Add test for str library
add_custom_command(
        OUTPUT "${CODEGEN_DIR}/test-str.c"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMAND /usr/bin/python3 "${CMAKE_SOURCE_DIR}/gentests.py"
                --tests "${TESTS_DIR}"
                --codegen "${CODEGEN_DIR}"
                -I "str/str.h"
                -i "str.tdata"
                -o "test-str.c"
        DEPENDS "${TESTS_DIR}/str.tdata"
)
add_executable(test-str "${CODEGEN_DIR}/test-str.c")
target_link_libraries(test-str regex)
add_test(NAME "test-str" COMMAND test-str)

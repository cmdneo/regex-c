cmake_minimum_required(VERSION 3.10)
project(
	RegexLib
	VERSION 0.1
	DESCRIPTION "Regex Library for C in C"
	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -Wnull-dereference -Wshadow -Wformat=2 -pedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(strlx strlx/str.c strlx/strbuf.c strlx/common.c strlx/fmt.c)
add_library(re re/re.c)
add_library(regex INTERFACE)
target_link_libraries(regex INTERFACE re strlx)

# Start testing
include(CTest)
set(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests")
set(CODEGEN_DIR "${CMAKE_BINARY_DIR}/tests")

# Add test for str library
add_custom_command(
	OUTPUT "${CODEGEN_DIR}/test-strlx.c"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	COMMAND /usr/bin/python3 "${CMAKE_SOURCE_DIR}/gentests.py"
		--tests "${TESTS_DIR}"
		--codegen "${CODEGEN_DIR}"
		-I "strlx/strlx.h"
		-i "str.tdata"
		-o "test-strlx.c"
	DEPENDS "${TESTS_DIR}/str.tdata"
)
add_executable(test-strlx "${CODEGEN_DIR}/test-strlx.c")
target_link_libraries(test-strlx regex)
add_test(NAME "test-strlx" COMMAND test-strlx)
set_tests_properties(test-strlx PROPERTIES TIMEOUT 5)

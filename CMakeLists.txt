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

include_directories("${CMAKE_SOURCE_DIR}/include")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# set(PUBLIC_HEADERS "strlx/strlx.h" "regex/regex.h" "regex/errors.h")
set(STRLX_SRCS "strlx/str.c" "strlx/strbuf.c" "strlx/common.c")
# set(REGEX_SRCS "regex/dsa.c" "regex/parser.c")

add_library(strlx ${STRLX_SRCS})
# add_library(regex ${STRLX_SRCS} ${REGEX_SRCS})

# Start testing
include(CTest)

# TEST strlx library
set(STRLX_TEST_SRC "${CMAKE_BINARY_DIR}/tests/test-strlx.c")
add_custom_command(
	OUTPUT ${STRLX_TEST_SRC}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMAND /usr/bin/python3 "gentests.py"
		-I "strlx/strlx.h"
		-i "tests/str.tdata"
		-o ${STRLX_TEST_SRC}
	DEPENDS "tests/str.tdata"
)
add_executable(test-strlx ${STRLX_TEST_SRC})
target_link_libraries(test-strlx strlx)
add_test(NAME test-strlx COMMAND test-strlx)

# TEST regex library
# add_executable(test-regex tests/test-regex.c)
# target_link_libraries(test-regex regex)
# add_test(NAME test-regex COMMAND test-regex)

# set_tests_properties(test-strlx test-regex PROPERTIES TIMEOUT 5)

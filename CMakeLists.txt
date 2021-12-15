cmake_minimum_required(VERSION 3.10)
project(RegexLib
        VERSION 0.1
        DESCRIPTION "Regex Library for C in C"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib")

include_directories("include")
add_library(str str/str.c str/strbuf.c str/common.c)

# Testing, TODO Use a more standard testing(unit testing)
add_executable(test_main tests/test-str.c)
target_link_libraries(test_main str)
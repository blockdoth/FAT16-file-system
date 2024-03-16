cmake_minimum_required(VERSION 3.22)
project(file_system C)

set(CMAKE_C_STANDARD 23)

# Define sources
file(GLOB_RECURSE FILE_SYSTEM_SOURCES
        "file_system/*.c"
        "file_system/*.h"
)

file(GLOB_RECURSE TEST_SOURCES
        "file_system/*.c"
        "file_system/*.h"
        "tests/*.c"
        "tests/*.h"
)

list(FILTER TEST_SOURCES EXCLUDE REGEX "main.c")



add_executable(main ${FILE_SYSTEM_SOURCES})
add_executable(test ${TEST_SOURCES})

cmake_minimum_required(VERSION 3.22)
project(file_system C)

set(CMAKE_C_STANDARD 23)

add_executable(file_system
        main.c
        include/file_system.h
        file_system/file_system.c
        include/volume.h
        file_system/volume.c
        file_system/volume_test.c
        include/FAT16.h

)

cmake_minimum_required(VERSION 3.22)
project(file_system C)

set(CMAKE_C_STANDARD 23)

add_executable(file_system
        main.c
        file_system/file_system.h
        file_system/file_system.c
        file_system/volume_management/volume.h
        file_system/volume_management/volume.c
        file_system/volume_management/volume_test.c
        file_system/file_system_implementations/FAT16.h
        file_system/file_system_implementations/FAT16.c
        file_system/file_system_implementations/FAT16_debug.c
        file_system/file_system_implementations/FAT16_debug.h
        file_system/file_system_implementations/FAT16_debug.h
        file_system/file_system_implementations/FAT16_debug.c

)

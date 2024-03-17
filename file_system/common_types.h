#ifndef FILE_SYSTEM_COMMON_TYPES_H
#define FILE_SYSTEM_COMMON_TYPES_H

#include "stdint.h"
#include "stdbool.h"
typedef uint32_t volume_ptr;
// TODO make merge this with another header without cyclic dependencies
typedef enum FS_STATUS_CODE {
    FS_SUCCES = true,
    FS_OUT_OF_SPACE = false,
    FS_FILE_ALREADY_EXISTS = false,
    FS_FILE_NOT_FOUND = false,
    FS_DIRECTORY_ALREADY_EXISTS,
    FS_DIRECTORY_NOT_FOUND = false,
    FS_FORMAT_NOT_SUPPORTED = false,
    FS_OUT_OF_BOUNDS = false,
    FS_VOLUME_TO_SMALL = false,
    FS_ILLEGAL_STATE = false,
    FS_INVALID_PATH = false,
} FS_STATUS_CODE;


#endif //FILE_SYSTEM_COMMON_TYPES_H

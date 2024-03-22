#ifndef FILE_SYSTEM_COMMON_TYPES_H
#define FILE_SYSTEM_COMMON_TYPES_H

#include "stdint.h"
#include "stdbool.h"
typedef uint16_t sector_ptr;
typedef uint16_t cluster_ptr;
typedef void* Sector;

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
    FS_NOT_IMPLEMENTED = false,
    FS_OPERATION_NOT_SUPPORTED_ON_DIR = false,
    FS_OPERATION_NOT_SUPPORTED_ON_FILE = false,
    FS_SOUGHT_FILE_FOUND_DIR = false,
    FS_SOUGHT_DIR_FOUND_FILE = false,
    FS_DELETION_FAILED = false,
} FS_STATUS_CODE;

typedef enum DriveID {
    DRIVE_D = 0,
    DRIVE_R = 1,
    DRIVE_B = 2,
    INVALID_DRIVE_ID = -1
    // etc etc
} DriveID; // IMPORTANT, keep in sync with the switch statement in parsePath

#define maxDriveCount 2 // This value should be sizeof DriveID



#endif //FILE_SYSTEM_COMMON_TYPES_H

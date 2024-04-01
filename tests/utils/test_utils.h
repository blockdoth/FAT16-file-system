#ifndef FILE_SYSTEM_TEST_UTILS_H
#define FILE_SYSTEM_TEST_UTILS_H
#include <time.h>
#include "stdint.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

#define GiB 1073741823


#define MAX_DIR_DEPTH 10
#define MIN_DIR_DEPTH 2
#define MAX_NAME_LENGTH 10
#define MIN_NAME_LENGTH 3
#define MAX_FILES 10
#define MAX_FILESIZE 1000
#define MIN_FILESIZE 10
#define MAX_SUBFOLDERS 5
#define MIN_SUBFOLDERS 5

char* randomString(uint32_t length);
char* generatePath();
bool memCompare(char* fileA, char* fileB, uint32_t size);

#endif //FILE_SYSTEM_TEST_UTILS_H

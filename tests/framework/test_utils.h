#ifndef FILE_SYSTEM_TEST_UTILS_H
#define FILE_SYSTEM_TEST_UTILS_H
#include <time.h>
#include "stdint.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "../../file_system/volume/volume.h"
#include "../../file_system/file_system_api/file_system_api.h"

#define GiB 1073741823

#define SECTOR_SIZE 512
#define SECTORS_PER_CLUSTER 64

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
bool memCompare(char* memA, char* memB, uint32_t size);
void mergeData(char *initialData, char *updatedData, uint32_t dataSize, uint32_t offset);
void setupFormattedVolume();
#endif //FILE_SYSTEM_TEST_UTILS_H

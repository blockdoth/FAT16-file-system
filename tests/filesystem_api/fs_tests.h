#ifndef FS_API_H
#define FS_API_H

#include "../framework/test_framework.h"
#include "../../file_system/file_system_api/file_system_api.h"
#include "../../file_system/common_types.h"
#include "../../file_system/formats/FAT16/FAT16_IO.h"
#include "../framework/test_utils.h"

#define MAX_NOT_NESTED_FILES 1
#define MAX_NESTED_FILES 3
#define CLUSTER_COUNT 100

//#define TEST_FS

void register_fs_tests();


#endif

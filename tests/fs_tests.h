#ifndef API_TEST_H
#define API_TEST_H

#include "framework/test_framework.h"
#include "../file_system/volume/volume.h"
#include "../file_system/file_system_api.h"
#include "utils/test_utils.h"
#include "../file_system/common_types.h"
#include "../file_system/formats/FAT16/FAT16_IO.h"

#define MAX_NOT_NESTED_FILES 10
#define MAX_NESTED_FILES 3
#define SECTOR_SIZE 512
#define SECTORS_PER_CLUSTER 64
#define CLUSTER_COUNT 100

void register_tests();


#endif //API_TEST_H

#ifndef VOLUME_H
#define VOLUME_H

#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include <malloc.h>
#include "../common_types.h"
//#include "../file_system_api.h"

typedef enum VOLUME_TYPE{
    RAM_DISK,
    FLASH_DRIVE
} VOLUME_TYPE;


typedef struct RawVolume {
    void* volumeData;
    uint32_t volumeSize;
    FS_STATUS_CODE (*init)(struct RawVolume* self, uint32_t volumeSize);
    void (*destroy)(struct RawVolume* self);
    FS_STATUS_CODE (*write)(struct RawVolume* self, void* sourceAddress, uint32_t destinationAddress, uint32_t dataSize);
    void* (*read)(struct RawVolume* self, uint32_t src_addr, uint32_t dataSize);
} RawVolume;

// Initialize the actual rawVolume
RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t volumeSize);

// Util
FS_STATUS_CODE bounds_check(RawVolume* self, uint32_t start_addr, uint32_t dataSize);

#endif //FILE_SYSTEM_VOLUME_H

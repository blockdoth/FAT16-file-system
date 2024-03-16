#ifndef VOLUME_H
#define VOLUME_H

#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include <malloc.h>
#include "volume_ptr.h"

typedef enum VOLUME_TYPE{
    RAM_DISK,
    FLASH_DRIVE
} VOLUME_TYPE;


typedef struct RawVolume {
    void* volumeData;
    uint32_t volumeSize;
    bool (*init)(struct RawVolume* self, uint32_t volumeSize);
    void (*destroy)(struct RawVolume* self);
    bool (*write)(struct RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
    void* (*read)(struct RawVolume* self, volume_ptr src_addr, uint32_t dataSize);
} RawVolume;

// Initialize the actual rawVolume
RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t volumeSize);

// Util
bool bounds_check(RawVolume* self, uint32_t start_addr, uint32_t dataSize);

#endif //FILE_SYSTEM_VOLUME_H

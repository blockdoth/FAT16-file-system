#ifndef VOLUME_H
#define VOLUME_H

#ifndef STDINT_H
#include "stdint.h"
#endif
#ifndef STDBOOL_H
#include "stdbool.h"
#endif

#define DEBUG_VOLUME //Enable debug mode
#define DEBUG_BUFFER_SIZE 50

typedef enum VOLUME_TYPE{
    RAM_DISK,
    FLASH_DRIVE
} VOLUME_TYPE;

typedef uint32_t volume_ptr;

typedef struct RawVolume {
    void* volumeData;
    uint32_t volumeSize;
    bool (*init)(struct RawVolume* self, uint32_t volumeSize);
    void (*destroy)(struct RawVolume* self);
    bool (*write)(struct RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
    void* (*read)(struct RawVolume* self, volume_ptr src_addr, uint32_t dataSize);
} RawVolume;


// Util
bool bounds_check(RawVolume* self, uint32_t start_addr, uint32_t dataSize);

// Initializes the correct Object based on the volume type
RawVolume* prep_volume(VOLUME_TYPE volumeType);
// Initialize the actual volume
RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t volumeSize);

// RAM Disk functions

// Initializes a ran disk volume object
RawVolume* prep_ram_disk();

// Allocate a chunk of memory to create a RAM disk
bool ramdisk_init(RawVolume* self, uint32_t volumeSize);
// Free all memory
void ramdisk_destroy(RawVolume* self);
// Write to the RAM disk
bool ramdisk_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
// Read from a RAM disk
void* ramdisk_read(RawVolume* self, volume_ptr sourceAddress, uint32_t dataSize);

// Flash Drive functions
RawVolume* prep_flash_drive(); // To be implemented
// TODO
bool prep_init(RawVolume* self, uint32_t volumeSize);
void prep_destroy(RawVolume* self);
bool prep_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
void* prep_read(RawVolume* self, volume_ptr sourceAddress, uint32_t dataSize);

void allowtruncate(char buffer[DEBUG_BUFFER_SIZE], uint8_t* string, uint32_t dataSize);
void initDebugLog(uint32_t volumeSize);
void destroyDebugLog(uint32_t volumeSize);
void readDebugLog(volume_ptr sourceAddress, uint32_t dataSize, void *dataAddress);
void writeDebugLog(void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
void clearDebugLog(volume_ptr destinationAddress, uint32_t dataSize);


#endif //FILE_SYSTEM_VOLUME_H

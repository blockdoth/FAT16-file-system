#ifndef VOLUME_H
#define VOLUME_H

#ifndef STDINT_H
#include "stdint.h"
#endif
#ifndef STDBOOL_H
#include "stdbool.h"
#endif

#define DEBUG //Enable debug mode

#ifdef DEBUG
#define DEBUG_BUFFER_SIZE 50
// Allocates and truncates a string
#define ALLOTRUNCATE(string) \
    char buffer[DEBUG_BUFFER_SIZE + 4];                     \
    snprintf(buffer, DEBUG_BUFFER_SIZE + 4, "%s", string);\
    if(data_size > DEBUG_BUFFER_SIZE){             \
        strcpy(buffer+DEBUG_BUFFER_SIZE, "..."); \
    }
// Debug macro's for all supported volume operations
#define INIT_DEBUG_LOG(volume_size) \
    printf("Created a volume of size %u\n", (volume_size))
#define DESTROY_DEBUG_LOG(volume_size) \
    printf("Destroyed a volume of size %u\n", (volume_size))
#define READ_DEBUG_LOG(src_addr, data_size, data_addr) \
    ALLOTRUNCATE(data_addr)\
    printf("Reading [%s] of size %u from address %u\n", buffer, data_size, src_addr)
#define WRITE_DEBUG_LOG(src_addr, dest_addr, data_size) \
    ALLOTRUNCATE(src_addr)\
    printf("Writing [%s] of size %u to address %u\n", buffer, data_size, dest_addr)
#else
#define INIT_DEBUG_LOG(x) ((void)0)
#define DESTROY_DEBUG_LOG(x) ((void)0)
#define READ_DEBUG_LOG(x,y,z) ((void)0)
#define WRITE_DEBUG_LOG(x,y,z) ((void)0)
#endif


typedef enum VOLUME_TYPE{
    RAM_DISK,
    FLASH_DRIVE
} VOLUME_TYPE;

typedef uint32_t volume_ptr;

typedef struct raw_volume {
    void* volume_data;
    uint32_t size;
    bool (*init)(struct raw_volume* self, uint32_t volume_size);
    void (*destroy)(struct raw_volume* self);
    bool (*write)(struct raw_volume* self, uint32_t* src_addr, volume_ptr dest_addr, uint32_t data_size);
    void* (*read)(struct raw_volume* self, volume_ptr src_addr, uint32_t data_size);
} raw_volume;


// Util
bool bounds_check(raw_volume* self, uint32_t start_addr, uint32_t data_size);

// Initializes the correct Object based on the volume type
raw_volume* prep_volume(VOLUME_TYPE volumeType);
// Initialize the actual volume
raw_volume* mount_volume(VOLUME_TYPE volumeType, uint64_t size);

// RAM Disk functions

// Initializes a ran disk volume object
raw_volume* prep_ram_disk();

// Allocate a chunk of memory to create a RAM disk
bool ramdisk_init(raw_volume* self, uint32_t volume_size);
// Free all memory
void ramdisk_destroy(raw_volume* self);
// Write to the RAM disk
bool ramdisk_write(raw_volume* self, uint32_t* src_addr, volume_ptr dest_addr, uint32_t data_size);
// Read from a RAM disk
void* ramdisk_read(raw_volume* self, volume_ptr src_addr, uint32_t data_size);

// Flash Drive functions
raw_volume* prep_flash_drive(); // To be implemented
// TODO
bool prep_init(raw_volume* self, uint32_t volume_size);
void prep_destroy(raw_volume* self);
bool prep_write(raw_volume* self, uint32_t* src_addr, volume_ptr dest_addr, uint32_t data_size);
void* prep_read(raw_volume* self, volume_ptr src_addr, uint32_t data_size);

#endif //FILE_SYSTEM_VOLUME_H

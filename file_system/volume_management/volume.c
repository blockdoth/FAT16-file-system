#include "volume.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>


RawVolume* mount_volume(VOLUME_TYPE volumeType, uint64_t size){
    RawVolume* volume = prep_volume(volumeType);
    if(volume->init(volume, size)){
        printf("Failed to mount rawVolume\n");
    }
    printf("Successfully mounted rawVolume\n");
    return volume;
}

bool bounds_check(RawVolume* self, uint32_t start_addr, uint32_t size){
    if((start_addr + size) > self->volumeSize){
        if (start_addr > self->volumeSize){
            printf("ERROR:\t Address %u out of bounds of rawVolume\n", start_addr);
        }else{
            printf("ERROR:\t Allocation of %u to large from start address %u\n", size, start_addr);
        }
        return 1;
    }
    return 0;
}

RawVolume* prep_volume(VOLUME_TYPE volumeType){
    switch (volumeType) {
        case RAM_DISK:
            return prep_ram_disk();
        case FLASH_DRIVE:
            return prep_flash_drive();
        default:
    }
}

// === RAM setup ===

RawVolume* prep_ram_disk() {
    RawVolume* volume = (RawVolume*)malloc(sizeof(RawVolume));
    if(volume == NULL){
        return NULL;
    }
    volume->volumeData = 0;
    volume->volumeSize = 0;
    volume->init = ramdisk_init;
    volume->destroy = ramdisk_destroy;
    volume->write= ramdisk_write;
    volume->read = ramdisk_read;
    return volume;
}

bool ramdisk_init(RawVolume* self, uint32_t volume_size){
    self->volumeData = malloc(volume_size);
    if(self->volumeData == NULL){
        printf("ERROR:\t Failed to allocate ram disk");
        return 1;
    }
    self->volumeSize = volume_size;
    initDebugLog(volume_size);
    return 0;
}
void ramdisk_destroy(RawVolume* self){
    initDebugLog(self->volumeSize);
    free(self->volumeData);
    free(self);
}

bool ramdisk_write(RawVolume* self, void* sourceAddress, volume_ptr destinationAddress, uint32_t size){
    if(bounds_check(self, destinationAddress, size)){
        return 1;
    }
    uint32_t* destination = (uint32_t*) (self->volumeData + destinationAddress);
    if(sourceAddress == NULL){
        clearDebugLog(destinationAddress, size);
        memset(destination,0, size);
    }else{
        writeDebugLog((char*) sourceAddress, destinationAddress, size);
        memcpy(destination,sourceAddress,size);
    }
    return 0;
}
void* ramdisk_read(RawVolume* self, volume_ptr sourceAddress, uint32_t size){
    if(bounds_check(self, sourceAddress, size)){
        return NULL;
    }
    void* chunk = malloc(size);
    if(chunk == NULL){
        printf("ERROR:\t Failed to allocate memory for read\n");
    }

    uint32_t* source = (uint32_t*) (self->volumeData + sourceAddress);
    memcpy(chunk,source,size);

    readDebugLog(sourceAddress, size, (char*) chunk);
    return chunk;
}

// === FLASH drive setup ===
RawVolume* prep_flash_drive(){
    assert(0 && "TODO");
}


void allowtruncate(char buffer[DEBUG_BUFFER_SIZE], uint8_t * string, uint32_t dataSize) {
    for (uint8_t i = 0; i * 2 < DEBUG_BUFFER_SIZE && i < dataSize; i++) {
        snprintf(buffer + i * 2,3,"%02x",string[i] );
    }
    if (dataSize > DEBUG_BUFFER_SIZE) {
        strncpy(buffer + DEBUG_BUFFER_SIZE - 4, "...", 3);
        buffer[DEBUG_BUFFER_SIZE - 1] = '\0';
    }
}

void initDebugLog(uint32_t volumeSize) {
    #ifdef DEBUG_VOLUME
    printf("Created a rawVolume of size %u\n", volumeSize);
    #endif
}

void destroyDebugLog(uint32_t volumeSize) {
    #ifdef DEBUG_VOLUME
    printf("Destroyed a rawVolume of size %u\n", volumeSize);
    #endif
}

void readDebugLog(volume_ptr sourceAddress, uint32_t dataSize, void *dataAddress) {
    #ifdef DEBUG_VOLUME
    char buffer[DEBUG_BUFFER_SIZE];
    allowtruncate(buffer, dataAddress, dataSize);
    printf("Reading [0x%s] of size %u from address %u\n",buffer, dataSize, sourceAddress);
    #endif
}

void writeDebugLog(void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize) {
    #ifdef DEBUG_VOLUME
    char buffer[DEBUG_BUFFER_SIZE];
    allowtruncate(buffer, sourceAddress, dataSize);
    printf("Writing [0x%s] of size %u to address %u\n", buffer, dataSize, destinationAddress);
    #endif
}

void clearDebugLog(volume_ptr destinationAddress, uint32_t dataSize) {
    #ifdef DEBUG_VOLUME
    printf("Clearing memory from address %u to %u\n", destinationAddress, destinationAddress + dataSize);
    #endif
}

#undef DEBUG_VOLUME
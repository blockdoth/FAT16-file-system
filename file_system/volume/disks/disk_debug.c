#include "disk_debug.h"

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

void readDebugLog(sector_ptr sourceAddress, uint32_t dataSize, void *dataAddress) {
#ifdef DEBUG_VOLUME
    char buffer[DEBUG_BUFFER_SIZE];
    allowtruncate(buffer, dataAddress, dataSize);
    printf("Reading [0x%s] of size %u from address %u\n",buffer, dataSize, sourceAddress);
#endif
}

void writeDebugLog(void* sourceAddress, sector_ptr destinationAddress, uint32_t dataSize) {
#ifdef DEBUG_VOLUME
    char buffer[DEBUG_BUFFER_SIZE];
    allowtruncate(buffer, sourceAddress, dataSize);
    printf("Writing [0x%s] of size %u to address %u\n", buffer, dataSize, destinationAddress);
#endif
}

void clearDebugLog(sector_ptr destinationAddress, uint32_t dataSize) {
#ifdef DEBUG_VOLUME
    printf("Clearing memory from address %u to %u\n", destinationAddress, destinationAddress + dataSize);
#endif
}

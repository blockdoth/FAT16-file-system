#ifndef FILE_SYSTEM_DISK_DEBUG_H
#define FILE_SYSTEM_DISK_DEBUG_H
#include "string.h"
#include "stdint.h"
#include "stdio.h"
#include "../volume_ptr.h"


#define DEBUG_BUFFER_SIZE 50

void allowtruncate(char buffer[DEBUG_BUFFER_SIZE], uint8_t* string, uint32_t dataSize);
void initDebugLog(uint32_t volumeSize);
void destroyDebugLog(uint32_t volumeSize);
void readDebugLog(volume_ptr sourceAddress, uint32_t dataSize, void *dataAddress);
void writeDebugLog(void* sourceAddress, volume_ptr destinationAddress, uint32_t dataSize);
void clearDebugLog(volume_ptr destinationAddress, uint32_t dataSize);

#endif //FILE_SYSTEM_DISK_DEBUG_H

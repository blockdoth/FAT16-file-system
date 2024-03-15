#ifndef FILE_SYSTEM_FAT16_UTILITY_H
#define FILE_SYSTEM_FAT16_UTILITY_H
#include "FAT16.h"

#include <string.h>
#include <malloc.h>

FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata);
uint8_t convertToDirAttributes(FileMetadata* file);

bool check_FAT16_formattible(RawVolume *raw_volume);

FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo *volumeInfo);
BootSector initBootSector(uint32_t volumeSize);

FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

unsigned char directoryNameChecksum(unsigned char *name);

Path parsePath(char* path);
void destroyPath(Path path);


#endif //FILE_SYSTEM_FAT16_UTILITY_H

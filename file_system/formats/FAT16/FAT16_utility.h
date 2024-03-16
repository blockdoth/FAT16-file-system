#ifndef FILE_SYSTEM_FAT16_UTILITY_H
#define FILE_SYSTEM_FAT16_UTILITY_H
#include "FAT16.h"

#include <string.h>
#include <malloc.h>

typedef struct Path {
    char** path;
    uint8_t depth;
} Path;


// === IO operations ===
void writeSector(FormattedVolume* self, void* data, volume_ptr sector, uint32_t dataSize);
void* readSector(FormattedVolume* self, volume_ptr sector);

void writeFileEntry(FormattedVolume* self, FAT16File fileMetadata, volume_ptr tableStart, volume_ptr startSector, volume_ptr endSector);
FAT16File readFileEntry(FormattedVolume* self, volume_ptr tableStart, uint32_t index);

void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector);
uint16_t readFATS(FormattedVolume* self, uint32_t index);

// === Resolving ===
volume_ptr resolveFile(FormattedVolume* self, char* path, char* fileName);
volume_ptr findFreeCluster(FormattedVolume* self);
FAT16File findEntryInTable(FormattedVolume* self, char* fileName, volume_ptr startTable);
Path parsePath(char* path);

// === Conversions ===
FAT16File convertMetadataToFAT16File(FileMetadata* fileMetadata);
uint8_t convertToDirAttributes(FileMetadata* file);

// === initializations ===
BootSector initBootSector(uint32_t volumeSize);
FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// === Misc ===
bool checkFAT16Compatible(RawVolume *raw_volume);
unsigned char directoryNameChecksum(unsigned char *name);
// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

#endif //FILE_SYSTEM_FAT16_UTILITY_H

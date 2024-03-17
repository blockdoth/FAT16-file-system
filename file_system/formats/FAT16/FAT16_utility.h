#ifndef FILE_SYSTEM_FAT16_UTILITY_H
#define FILE_SYSTEM_FAT16_UTILITY_H
#include "FAT16.h"

#include <string.h>
#include <malloc.h>



// === IO operations ===
void writeSector(FormattedVolume *self, volume_ptr sector, void *data, uint32_t dataSize);
void writePartialSector(FormattedVolume* self, volume_ptr sector, uint32_t bytesOffset, void* data, uint32_t dataSize);
void* readSector(FormattedVolume* self, volume_ptr sector, uint32_t size);

void writeFileEntry(FormattedVolume* self, FAT16File fileMetadata, volume_ptr entryTable, volume_ptr startSector, volume_ptr endSector);
FAT16File readFileEntry(FormattedVolume* self, volume_ptr tableStart, uint32_t index);
void updateFAT16Entry(FormattedVolume* self, volume_ptr entryTable, FAT16File fat16File);

void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector);
uint16_t readFATS(FormattedVolume* self, uint32_t index);

// === Resolving ===
volume_ptr resolveFileTable(FormattedVolume *self, Path path);
volume_ptr findFreeCluster(FormattedVolume* self);
FAT16File findEntryInTable(FormattedVolume *self, volume_ptr startTable, char* name);
Path parsePath(char* path);
bool checkNamingCollusion(FormattedVolume* self, volume_ptr entryTable, char* name, bool lookingForDir);

// === Conversions ===
FAT16File convertMetadataToFAT16File(FileMetadata* fileMetadata);
uint8_t convertToDirAttributes(FileMetadata* file);

// === initializations ===
BootSector initBootSector(uint32_t volumeSize);
FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// === Misc ===
uint32_t calculateMaxEntries(FormattedVolume* self, volume_ptr entryTable);
bool checkFAT16Compatible(RawVolume *raw_volume);
unsigned char directoryNameChecksum(unsigned char *name);
// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

#endif //FILE_SYSTEM_FAT16_UTILITY_H

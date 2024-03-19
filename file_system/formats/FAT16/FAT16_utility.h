#ifndef FILE_SYSTEM_FAT16_UTILITY_H
#define FILE_SYSTEM_FAT16_UTILITY_H
#include "FAT16.h"

#include <string.h>
#include <malloc.h>



// === IO operations ===
FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size);
FS_STATUS_CODE updateSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size, uint32_t offset);
FS_STATUS_CODE writeDataSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size);

void* readSector(FormattedVolume* self, sector_ptr sector);
void *readClusterSector(FormattedVolume *self, cluster_ptr cluster, sector_ptr sector);
FS_STATUS_CODE clearSectors(FormattedVolume* self, sector_ptr startSector, uint32_t count);

FS_STATUS_CODE
writeFileEntry(FormattedVolume *self, FAT16File fileEntry, cluster_ptr entryTable);
FAT16File readFileEntry(FormattedVolume* self, sector_ptr tableStart, uint32_t index);
FS_STATUS_CODE updateEntry(FormattedVolume* self, sector_ptr entryTable, FAT16File fat16File);
FS_STATUS_CODE deleteEntry(FormattedVolume *self, cluster_ptr entryTable, char *name, bool lookingForDir);


FS_STATUS_CODE writeFATS(FormattedVolume* self, sector_ptr index, void *nextSector);
uint16_t readFATS(FormattedVolume* self, uint16_t index);

// === Resolving ===
sector_ptr resolveFileTable(FormattedVolume *self, Path path);
sector_ptr findFreeCluster(FormattedVolume* self);
FAT16File findEntryInTable(FormattedVolume *self, cluster_ptr entryTable, char* name);
Path parsePath(char* path);
FS_STATUS_CODE checkNamingCollusion(FormattedVolume* self, cluster_ptr entryTable, char* name, bool lookingForDir);

// === Conversions ===
FAT16File convertMetadataToFAT16File(FileMetadata* fileMetadata);
uint8_t convertToDirAttributes(FileMetadata* file);

// === initializations ===
BootSector initBootSector(uint32_t volumeSize);
FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// === Misc ===
uint32_t calculateMaxEntries(FormattedVolume* self, sector_ptr entryTable);
FS_STATUS_CODE checkFAT16Compatible(RawVolume *raw_volume);
unsigned char directoryNameChecksum(unsigned char *name);
bool isDir(FAT16File entry);
// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

#endif //FILE_SYSTEM_FAT16_UTILITY_H

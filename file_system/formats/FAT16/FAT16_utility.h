#ifndef FILE_SYSTEM_FAT16_UTILITY_H
#define FILE_SYSTEM_FAT16_UTILITY_H
#include "FAT16.h"
#include "FAT16_IO.h"

#include <string.h>
#include <malloc.h>

// Kinda an awkward struct,
// but I really needed all this information to be returned in the same place
typedef struct Entry {
    FAT16File entry;
    sector_ptr sectorPtr;
    void* sector;
    uint32_t inSectorOffset;
} Entry;

// === IO operations ===
// Read and write are in FAT16_IO.h
// Reads, updates and writes back a sector
FS_STATUS_CODE updateSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size, uint32_t offset);

// Prevents unnecessary developer thunking
FS_STATUS_CODE writeClusterSector(FormattedVolume *self, cluster_ptr cluster, sector_ptr sector, void *data, uint32_t size);
void readClusterSector(FormattedVolume *self, cluster_ptr cluster, sector_ptr sector, void *buffer, uint32_t readSize);
FS_STATUS_CODE updateClusterSector(FormattedVolume *self, cluster_ptr cluster, sector_ptr sector, void *data, uint32_t size, uint32_t offset);

// Zero's out the whole sector
FS_STATUS_CODE clearSectors(FormattedVolume* self, sector_ptr startSector, uint32_t count);

// Complex method that writes newdata sectors and updates the FAT accordingly
// Only to be used when you want to write completely new sectors starting in a new cluster
void writeAlignedSectors(FormattedVolume *self, void *newData, uint32_t bytesLeftToWrite, uint32_t currentDataPointer,
                         sector_ptr currentCluster);

// === Metadata ===
// Does what it says
FS_STATUS_CODE writeFileEntry(FormattedVolume *self, FAT16File fileEntry, cluster_ptr entryTable);
FS_STATUS_CODE updateFileEntry(FormattedVolume* self, FAT16File fileEntry, cluster_ptr entryTable);
FS_STATUS_CODE renameFileEntry(FormattedVolume* self, FAT16File fileEntry, cluster_ptr entryTable, char* oldName);
FAT16File readFileEntry(FormattedVolume* self, sector_ptr tableStart, uint32_t index);
FS_STATUS_CODE deleteEntry(FormattedVolume *self, cluster_ptr entryTable, char *name, bool lookingForDir);

// Finds sector containing metadata table of a directory
sector_ptr resolveFileTable(FormattedVolume *self, Path* path);
// Find the metadata of a file in a metadata table
FAT16File findEntryInTable(FormattedVolume *self, cluster_ptr entryTable, char* name);
Entry findEntry(FormattedVolume* self, cluster_ptr entryTable, char* name);

// === FAT Manipulation ===
FS_STATUS_CODE deleteFATS(FormattedVolume* self, sector_ptr index);
FS_STATUS_CODE writeFATS(FormattedVolume* self, sector_ptr index, sector_ptr nextSector);
uint16_t readFATS(FormattedVolume* self, uint16_t index);

sector_ptr findFreeClusterInFAT(FormattedVolume* self);
// Finds the cluster right before the EOF marker in the FAT //TODO explain better
sector_ptr findSecondToLastCluster(FormattedVolume *self, sector_ptr fileClusterStart);

// === Conversions ===
FAT16File convertMetadataToFAT16File(FileMetadata* fileMetadata);
FileMetadata* convertFAT16FileToMetadata(FAT16File fat16File);
uint8_t convertToDirAttributes(FileMetadata* file);

// === initializations ===
BootSector initBootSector(uint32_t volumeSize, FAT16Config fat16Config);
FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// === Misc ===
// Parses the path in individual pieces, extracts the name and calculates its depth
Path* parsePath(char* path);
// Checks if a file or dir with the same name already exists in a dir, files and dirs have seperate name spaces
FS_STATUS_CODE checkNamingCollusion(FormattedVolume* self, cluster_ptr entryTable, char* name, bool lookingForDir);
// Calculates the max amount of metadata entries based on if the tabble is in the root sector or not
uint32_t calculateMaxEntries(FormattedVolume* self, sector_ptr entryTable);
// Checks if a volume is compatible with FAT16
FS_STATUS_CODE checkFAT16Compatible(RawVolume *raw_volume);
bool isDir(FAT16File entry);

// Hardpp's spec, needs to be used when implementing longer filenames
unsigned char directoryNameChecksum(unsigned char *name);
// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

#endif //FILE_SYSTEM_FAT16_UTILITY_H

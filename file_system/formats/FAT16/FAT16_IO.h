#ifndef FILE_SYSTEM_FAT16_IO_H
#define FILE_SYSTEM_FAT16_IO_H
#include "FAT16.h"
#include "FAT16_debug.h"

// Inits the cache, allocates memory for each sector in it
void initCache(FormattedVolume* self, uint32_t cacheSize);

// Zero's out the entire cache
void wipeCache(FormattedVolume* self);

// Deallocates all allocated memory of the cache
void destroyCache(FormattedVolume* self);

// Deletes an entry from the cache after it became outdated
void invalidateSectorInCache(FormattedVolume* self, sector_ptr sectorPtr);

// === IO operations ===
// These 2 operation are the only thing the FS see's IO wise

// Writes a single sector, indexed by sector
// size is used to write partial sectors
FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size);

// Reads a single sector, indexed by sector
void* readSector(FormattedVolume* self, sector_ptr sector);


#endif //FILE_SYSTEM_FAT16_IO_H


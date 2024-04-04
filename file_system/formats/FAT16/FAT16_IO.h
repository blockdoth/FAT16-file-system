#ifndef FILE_SYSTEM_FAT16_IO_H
#define FILE_SYSTEM_FAT16_IO_H
#include "FAT16.h"
#include "FAT16_debug.h"

void initCache(FormattedVolume* self, uint32_t cacheSize);
void wipeCache(FormattedVolume* self);
void destroyCache(FormattedVolume* self);
void invalidateSectorInCache(FormattedVolume* self, sector_ptr sectorPtr);

FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size);
void* readSector(FormattedVolume* self, sector_ptr sector);


#endif //FILE_SYSTEM_FAT16_IO_H


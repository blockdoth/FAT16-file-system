#ifndef FILE_SYSTEM_FAT16_IO_H
#define FILE_SYSTEM_FAT16_IO_H
#include "FAT16.h"

void initCache(uint32_t sectorSize);
void wipeCache(uint32_t sectorSize);
void destroyCache();

FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size);
void* readSector(FormattedVolume* self, sector_ptr sector);


#endif //FILE_SYSTEM_FAT16_IO_H


#include "FAT16_IO.h"

#define MAX_PAGES_IN_CACHE 100

typedef struct {
    sector_ptr sectorPtr;
    void* sector;
    uint8_t age;
} CacheEntry;

CacheEntry cache[MAX_PAGES_IN_CACHE];


void initCache(uint32_t sectorSize) {
    for (int i = 0; i < MAX_PAGES_IN_CACHE; ++i) {
        cache[i].age = 0;
        cache[i].sectorPtr = 0;
        cache[i].sector = malloc(sectorSize);
    }
}

void wipeCache(uint32_t sectorSize){
    for (int i = 0; i < MAX_PAGES_IN_CACHE; ++i) {
        cache[i].age = 0;
        cache[i].sectorPtr = 0;
        memset(cache[i].sector,0, sectorSize);
    }
}
void destroyCache(){
    for (int i = 0; i < MAX_PAGES_IN_CACHE; ++i) {
        free(cache[i].sector);
    }
}



// A simple lookup table //TODO  make it a hashtable
void* findSectorInCache(sector_ptr sector){
    for (int i = 0; i < MAX_PAGES_IN_CACHE; ++i) {
        CacheEntry entry = cache[i];
        if(entry.sectorPtr == sector){
            entry.age++;
            return entry.sector;
        }
    }
    return NULL;
}

void insertSectorInCache(sector_ptr sectorPtr, void* sector, uint32_t sectorSize){
    uint32_t oldestEntryAge = -1;
    uint32_t oldestEntryIndex = -1;
    for (int i = 0; i < MAX_PAGES_IN_CACHE; ++i) {
        CacheEntry entry = cache[i];
        if(oldestEntryAge > entry.age){
            oldestEntryAge = entry.age;
            oldestEntryIndex = i;
        }
    }
    cache[oldestEntryIndex].age = 0;
    free(cache[oldestEntryIndex].sector);
    memcpy(cache[oldestEntryIndex].sector,sector, sectorSize);
    cache[oldestEntryIndex].sectorPtr = sectorPtr;
}


void* readSector(FormattedVolume* self, sector_ptr sector){
    void* foundSector = findSectorInCache(sector);
    uint32_t sectorSize = self->info->FAT16.bytesPerSector;
    if(foundSector != NULL){
        void* cachedSector = malloc(sectorSize);
        memcpy(cachedSector, foundSector, sectorSize);
        return cachedSector;
    } else{
        void* loadedSector = self->rawVolume->read(self->rawVolume, sector * sectorSize, sectorSize);
        insertSectorInCache(sector, loadedSector, sectorSize);
        return loadedSector;
    }
}

FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size) {
    if(size > self->info->FAT16.bytesPerSector){
        return FS_OUT_OF_BOUNDS;
    }

    return self->rawVolume->write(self->rawVolume, data,sector * self->info->FAT16.bytesPerSector, size);
}

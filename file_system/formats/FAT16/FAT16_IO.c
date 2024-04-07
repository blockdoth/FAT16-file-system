#include "FAT16_IO.h"


void initCache(FormattedVolume* self, uint32_t cacheSize) {
    FAT16CacheEntry* entries = (FAT16CacheEntry*) malloc(cacheSize * sizeof(FAT16CacheEntry));
    for (int i = 0; i < cacheSize; ++i) {
        entries[i] = (FAT16CacheEntry) {0,0, malloc(self->info->FAT16.bytesPerSector)};
    }
    self->cache.FAT16 = (FAT16Cache){entries, cacheSize, 0};
}

void destroyCache(FormattedVolume* self){

    FAT16CacheEntry* cache = self->cache.FAT16.cache;
    for (int i = 0; i < self->cache.FAT16.size; ++i) {
        free(cache[i].sector);
    }
    free(self->cache.FAT16.cache);
}

void wipeCache(FormattedVolume* self){
    FAT16CacheEntry* cache = self->cache.FAT16.cache;
    for (int i = 0; i < self->cache.FAT16.size; ++i) {
        cache[i].age = 0;
        cache[i].sectorPtr = 0;
    }
    memset(cache->sector,0, self->cache.FAT16.size * self->info->FAT16.bytesPerSector);
}


// A simple lookup table //TODO  make it a hashtable
void* findSectorInCache(FormattedVolume* self, sector_ptr sector){
    FAT16CacheEntry* cache = self->cache.FAT16.cache;
    for (int i = 0; i < self->cache.FAT16.size; ++i) {
        FAT16CacheEntry entry = cache[i];
        if(entry.sectorPtr == sector){
            if(entry.age < (uint16_t) - 1){
                entry.age++;
            }
            self->cache.FAT16.cacheHits++;
            return entry.sector;
        }
    }
    self->cache.FAT16.cacheMisses++;
    return NULL;
}

void invalidateSectorInCache(FormattedVolume* self, sector_ptr sectorPtr){
    FAT16CacheEntry* cache = self->cache.FAT16.cache;
    for (int i = 0; i < self->cache.FAT16.size; ++i) {
        FAT16CacheEntry* entry = &cache[i];
        if(entry->sectorPtr == sectorPtr){
            entry->age = 0;
            entry->sectorPtr = 0;
            memset(entry->sector,0, self->info->FAT16.bytesPerSector);
            return;
        }
    }
}
void insertSectorInCache(FormattedVolume* self, sector_ptr sectorPtr, void* sector){
    uint32_t leastHitEntry = -1;
    uint32_t leastHitEntryIndex = -1;
    FAT16CacheEntry* cache = self->cache.FAT16.cache;
    for (int i = 0; i < self->cache.FAT16.size; ++i) {
        FAT16CacheEntry entry = cache[i];
        if(leastHitEntry > entry.age){
            leastHitEntry = entry.age;
            leastHitEntryIndex = i;
        }
    }
    FAT16CacheEntry* cacheEntry = &cache[leastHitEntryIndex];
    cacheEntry->age = 10;
    cacheEntry->sectorPtr = sectorPtr;
    memcpy(cacheEntry->sector, sector, self->info->FAT16.bytesPerSector);
}

// Read size should never be larger than bytesPerSector
void readSector(FormattedVolume *self, sector_ptr sector, void *buffer, uint32_t readSize) {
    uint32_t sectorSize = self->info->FAT16.bytesPerSector;
    if(readSize > sectorSize){
        printf("Should not happen");
        readSize = sectorSize;
    }
    void* foundSector = findSectorInCache(self, sector);
    if(foundSector != NULL){
        memcpy(buffer, foundSector, readSize);
    } else{
        void* chunk = self->rawVolume->read(self->rawVolume, sector * sectorSize, sectorSize);
        insertSectorInCache(self, sector, chunk);
        memcpy(buffer, chunk, readSize);
    }
//    printf("reading from %u\n", sector * sectorSize);

}
// TODO add COW
FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size) {
    uint32_t sectorSize = self->info->FAT16.bytesPerSector;
    if(size > sectorSize){
        return FS_OUT_OF_BOUNDS;
    }
//    printf("writing to %u\n", sector * sectorSize);
    invalidateSectorInCache(self, sector);
    return self->rawVolume->write(self->rawVolume, data, sector * sectorSize, size);
}

#include "FAT16_utility.h"

typedef struct Entry {
    FAT16File* entry;
    sector_ptr sector;
    uint32_t inSectorOffset;
} Entry;

FS_STATUS_CODE writeSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size) {
    if(size > self->info->bytesPerSector){
        return FS_OUT_OF_BOUNDS;
    }
    return self->rawVolume->write(self->rawVolume, data,sector * self->info->bytesPerSector, size);
}

FS_STATUS_CODE writeDataSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size) {
    return writeSector(self, self->info->dataSectionStart + sector, data, size);
}

void* readSector(FormattedVolume* self, sector_ptr sector){
    return self->rawVolume->read(self->rawVolume, sector * self->info->bytesPerSector, self->info->bytesPerSector);
}

void *readClusterSector(FormattedVolume *self, cluster_ptr cluster, sector_ptr sector) {
    return self->rawVolume->read(self->rawVolume,
                                 cluster * self->info->bytesPerCluster + sector * self->info->bytesPerSector,
                                 self->info->bytesPerSector);
}

FS_STATUS_CODE clearSectors(FormattedVolume* self, sector_ptr startSector, uint32_t count){
    for (uint32_t i = 0; i < count; i++) {
        self->rawVolume->write(self->rawVolume, NULL, (startSector + i) * self->info->bytesPerSector, self->info->bytesPerSector);
    }
    return FS_SUCCES;
}

FS_STATUS_CODE updateSector(FormattedVolume *self, sector_ptr sector, void *data, uint32_t size, uint32_t offset) {
    if(size > self->info->bytesPerSector){
        return FS_OUT_OF_BOUNDS;
    }
    void* chunk = readSector(self, sector);
    memcpy(chunk + offset, data, size);
    writeSector(self, sector, chunk, self->info->bytesPerSector);
    return FS_SUCCES;
}

Entry findFreeEntry(FormattedVolume* self, cluster_ptr entryTable){
    sector_ptr currentSector = entryTable;
    void* sector = readSector(self,currentSector);

    FAT16File* entry = malloc(sizeof(FAT16File));

    uint32_t entriesRead = 0;
    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    uint32_t offset;
    while(entriesRead < maxEntries){
        for (offset = 0; offset < self->info->bytesPerSector && entriesRead < maxEntries; offset++) {
            entry = memcpy(entry, sector + offset * FAT16_ENTRY_SIZE, 2);
            entriesRead++;
            if(entry->name[0] == 0x00 || entry->name[0] == 0xe5){
                break;
            }
        }
        currentSector++;
        sector = readSector(self,ccurrentSector);
    }



}

Entry findEntry(FormattedVolume* self, cluster_ptr entryTable, char* name){
    FAT16File* entry = malloc(sizeof(FAT16File));
    void* sector;

    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    sector_ptr currentSector = entryTable;
    uint32_t entriesRead = 0;
    uint32_t offset;
    while(entriesRead < maxEntries){
        sector = readSector(self,currentSector);
        for (offset = 0; offset < self->info->bytesPerSector && entriesRead < maxEntries; offset++) {
            entry = memcpy(entry, sector + offset * FAT16_ENTRY_SIZE, 2);
            entriesRead++;
            if(strcmp(entry->name, name) == 0){
                return (Entry){
                    entry,
                    currentSector,
                    offset * FAT16_ENTRY_SIZE,
                };
            }
        }
        currentSector++;
    }
    return (Entry) { // TODO check this
        NULL,
        0,
        0
    };
}

FS_STATUS_CODE writeFileEntry(FormattedVolume* self, FAT16File fileEntry, cluster_ptr entryTable) {

    updateSector(self, ,FAT16_ENTRY_SIZE);


    return self->rawVolume->write(self->rawVolume, &fileEntry, adjustedAddress, FAT16_ENTRY_SIZE);
}

// tableStart => Cluster address of entry table
// index => index in said table
FAT16File readFileEntry(FormattedVolume* self, sector_ptr tableStart, uint32_t index){
    uint32_t sectorsOffset = index / self->info->bytesPerSector;
    uint32_t bytesOffset = index % self->info->bytesPerSector;

    uint32_t adjustedAddress = tableStart * self->info->sectorsPerCluster + sectorsOffset;
    void* chunk = readSector(self, adjustedAddress);
    FAT16File* entry = (FAT16File*) malloc(sizeof(FAT16File));
    memcpy(entry, chunk + bytesOffset, 2);
    free(chunk);
    return *entry;
}

FS_STATUS_CODE updateEntry(FormattedVolume* self, sector_ptr entryTable, FAT16File fat16File){
    sector_ptr currentSector = entryTable;
    void* sector = readSector(self,currentSector);

    FAT16File* entry = malloc(sizeof(FAT16File));

    uint32_t entriesRead = 0;
    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    uint32_t offset;
    while(entriesRead < maxEntries){
        for (offset = 0; offset < self->info->bytesPerSector && entriesRead < maxEntries; offset++) {
            entry = memcpy(entry, sector + offset * FAT16_ENTRY_SIZE, 2);
            entriesRead++;
            if(strcmp(entry->name, fat16File.name) == 0){
                break;
            }
        }
        currentSector++;
        sector = readSector(self,currentSector);
    }
    return updateSector(self, currentSector, entry, 2, offset * FAT16_ENTRY_SIZE);
}


FS_STATUS_CODE deleteEntry(FormattedVolume *self, cluster_ptr entryTable, char *name, bool lookingForDir) {
    sector_ptr currentSector = entryTable;
    void* sector = readSector(self,currentSector);

    FAT16File* entry = malloc(sizeof(FAT16File));

    uint32_t entriesRead = 0;
    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    uint32_t offset;
    while(entriesRead < maxEntries){
        for (offset = 0; offset < self->info->bytesPerSector && entriesRead < maxEntries; offset++) {
            entry = memcpy(entry, sector + offset * FAT16_ENTRY_SIZE, 4);
            entriesRead++;
            if(strcmp(entry->name, name) == 0){
                if(isDir(*entry) == lookingForDir){
                    #ifdef DEBUG_FAT16
                    printf("Deleting %s of size %u at sector %u pointing to sector %u\n",
                           entry.name, entry.fileSize, entryTable, entry.FAT32fileClusterStart);
                    #endif

                    return FS_SUCCES;
                } else{
                    if(lookingForDir){
                        return FS_SOUGHT_DIR_FOUND_FILE;
                    } else{
                        return FS_SOUGHT_FILE_FOUND_DIR;
                    }
                }
            }
        }
        currentSector++;
        sector = readSector(self,currentSector);
    }
    if(lookingForDir){
        return FS_DIRECTORY_NOT_FOUND;
    }else{
        return FS_FILE_NOT_FOUND;
    }
}



// nextSector => Cluster address of next entry
FS_STATUS_CODE writeFATS(FormattedVolume* self, sector_ptr index, void *nextSector){
    uint32_t sectorOffset = index / self->info->bytesPerSector;
    uint32_t offsetInSector = index % self->info->bytesPerSector;

    FS_STATUS_CODE fat1 = updateSector(self, self->info->FAT1Start + sectorOffset, nextSector, 2, offsetInSector);
    FS_STATUS_CODE fat2 = updateSector(self, self->info->FAT2Start + sectorOffset, nextSector, 2, offsetInSector);
    return fat1 && fat2;
}

uint16_t readFATS(FormattedVolume* self, uint16_t index){
    uint32_t sectorOffset = index / self->info->bytesPerSector;
    uint32_t offsetInSector = index % self->info->bytesPerSector;

    Sector sectorFAT1 = readSector(self,self->info->FAT1Start + sectorOffset);
    Sector sectorFAT2 = readSector(self,self->info->FAT2Start + sectorOffset);

    uint16_t* FAT1 = (uint16_t*) malloc(sizeof(uint16_t));
    uint16_t* FAT2 = (uint16_t*) malloc(sizeof(uint16_t));
    memcpy(FAT1, sectorFAT1 + offsetInSector, 2);
    memcpy(FAT2, sectorFAT2 + offsetInSector, 2);

    if(*FAT1 != *FAT2){
        printf("FATS are out of sync");
    }
    uint16_t FATEntry = *FAT1;
    free(FAT1);
    free(FAT2);
    return FATEntry;
}


sector_ptr resolveFileTable(FormattedVolume *self, Path path) {
    sector_ptr entryTable = self->info->rootSectionStart;
    FAT16File entry;
    for(int i = 0; i < path.depth;i++){
        entry = findEntryInTable(self, entryTable, path.path[i]);
        entryTable = entry.FAT32fileClusterStart;
        if(entry.name[0] == 0){
            break;
        }
        if(strcmp((char*)entry.name, path.path[i]) != 0){
            #ifdef DEBUG_FAT16
            printf("ERROR: %s does not exist\n", entry.name);
            #endif
            return false;
        }
        if(!isDir(entry)){
            #ifdef DEBUG_FAT16
            printf("ERROR: %s is not a directory\n", entry.name);
            #endif
            return false;
        }
        free(path.path[i]);
    }
    return entryTable;
}


sector_ptr findFreeCluster(FormattedVolume* self){
    uint16_t entry;
    for (uint32_t i = 0; i < self->info->FATEntryCount; i++) {
        entry = readFATS(self,i);
        if(entry == 0x00){
            return self->info->dataSectionStart + i;
        }
    }
    return 0;
}

FAT16File findEntryInTable(FormattedVolume *self, cluster_ptr entryTable, char *name) {
    sector_ptr currentSector = entryTable;
    void* sector = readSector(self,currentSector);

    FAT16File* entry = malloc(sizeof(FAT16File));

    uint32_t entriesRead = 0;
    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    uint32_t offset;
    while(entriesRead < maxEntries){
        for (offset = 0; offset < self->info->bytesPerSector && entriesRead < maxEntries; offset++) {
            entry = memcpy(entry, sector + offset * FAT16_ENTRY_SIZE, 4);
            entriesRead++;
            if(strcmp(entry->name, name) == 0){
                // TODO support the actual FAT16 naming format and longer filenames
                #ifdef DEBUG_FAT16
                printf("Found %s of size %u in the FAT at sector %u pointing to sector %u\n",
                       entry.name, entry.fileSize, entryTable, entry.FAT32fileClusterStart);
                #endif
                FAT16File entryFile = *entry;
                free(entry);
                free(sector);
                return entryFile;
            }
        }
        currentSector++;
        sector = readSector(self,currentSector);
    }
    free(sector);
    free(entry);
    return (FAT16File){
            .reserved = 1
    };
}

uint32_t calculateMaxEntries(FormattedVolume* self, sector_ptr entryTable){
    if(self->info->rootSectionStart == entryTable){
        return (self->info->rootSectorCount * self->info->bytesPerSector) / FAT16_ENTRY_SIZE;
    }else{
        return self->info->bytesPerCluster / FAT16_ENTRY_SIZE;
    }
}

FS_STATUS_CODE checkNamingCollusion(FormattedVolume* self, cluster_ptr entryTable, char* name, bool lookingForDir){
    FAT16File entry = findEntryInTable(self, entryTable, name);
    if(strcmp(entry.name, name) == 0){
        if(isDir(entry) == lookingForDir){
            return FS_SUCCES;
        }
    }
    if(lookingForDir){
        return FS_DIRECTORY_NOT_FOUND;
    }else{
        return FS_FILE_NOT_FOUND;
    }
}

FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata){
    FAT16File fat16File;

    memcpy(fat16File.name, fileMetadata->name, FAT16_ENTRY_BASE_NAME_LENGTH); // TODO support long file names
    fat16File.name[10] = '\0';

    fat16File.attributes = convertToDirAttributes(fileMetadata);
    fat16File.reserved = 0;
    fat16File.creationTimeTenth = fileMetadata->creationTimeTenth;
    fat16File.creationTime = fileMetadata->creationTime;
    fat16File.creationDate = fileMetadata->creationDate;
    fat16File.lastAccessedDate = fileMetadata->creationDate;
    fat16File.timeOfLastWrite = fileMetadata->creationTime;
    fat16File.dateOfLastWrite = fileMetadata->creationDate;
    fat16File.FAT32fileClusterStart = 0;
    fat16File.fileClusterStart = 0;
    fat16File.fileSize = fileMetadata->fileSize;
    return fat16File;
}

uint8_t convertToDirAttributes(FileMetadata* file) {
    uint8_t dirAttributes = 0;
    dirAttributes |= file->read_only ? ATTR_READONLY : 0;
    dirAttributes |= file->hidden ? ATTR_HIDDEN : 0;
    dirAttributes |= file->system ? ATTR_SYSTEM : 0;
    dirAttributes |= file->volume_id ? ATTR_VOLUME_ID : 0;
    dirAttributes |= file->directory ? ATTR_DIRECTORY : 0;
    dirAttributes |= file->archive ? ATTR_ARCHIVE : 0;
    // Assuming ATTR_LONGNAME is a combination of all bits in long_name
    dirAttributes |= file->long_name ? ATTR_LONGNAME : 0;

    return dirAttributes;
}


// Init structs

BootSector initBootSector(uint32_t volumeSize){
    BootSector bootSector = {
            {0xEB, 0x00, 0x90},
            "MSWIN4.1",
            512,
            64,
            1,
            2,
            512,
            0,
            0xfa,
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            0x29,
            0, //TODO Set this field
            "NO NAME    ",
            "FAT16      "
    };
    // Volume size is in bytes
    bootSector.totalSectorCount16 = volumeSize / bootSector.sectorsPerCluster / bootSector.bytesPerSector;
    uint32_t totalClusterCount = (bootSector.totalSectorCount16 - bootSector.reservedSectorCount) / bootSector.sectorsPerCluster;
    bootSector.sectorsPersFAT = (totalClusterCount * 16) / bootSector.bytesPerSector;
    return bootSector;
}

FATVolumeInfo* initFATVolumeInfo(BootSector bootSector) {
    FATVolumeInfo* volumeInfo = (FATVolumeInfo*)malloc(sizeof(FATVolumeInfo));

    sector_ptr rootSectorStart = bootSector.reservedSectorCount + bootSector.sectorsPersFAT * bootSector.numberOfFATs;
    uint32_t rootSectorCount = (bootSector.rootEntryCount * 32) / bootSector.bytesPerSector;

    volumeInfo->FAT1Start = bootSector.reservedSectorCount;
    volumeInfo->FAT2Start = bootSector.reservedSectorCount + bootSector.sectorsPersFAT;
    volumeInfo->rootSectionStart = rootSectorStart;
    volumeInfo->dataSectionStart = rootSectorStart + rootSectorCount;
    volumeInfo->FATTableSectorCount = bootSector.sectorsPersFAT;
    volumeInfo->rootSectorCount = rootSectorCount;
    volumeInfo->totalSectorCount = bootSector.totalSectorCount16;
    volumeInfo->FATEntryCount = bootSector.totalSectorCount16;
    volumeInfo->totalAddressableSize = bootSector.totalSectorCount16 * bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    volumeInfo->bytesPerSector = bootSector.bytesPerSector;
    volumeInfo->sectorsPerCluster = bootSector.sectorsPerCluster;
    volumeInfo->bytesPerCluster = bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    return volumeInfo;
}



FS_STATUS_CODE checkFAT16Compatible(RawVolume *raw_volume) {
    if(raw_volume->volumeSize < FAT16_MINIMUM_SIZE){
        return FS_VOLUME_TO_SMALL;
    }else{
        return FS_SUCCES;
    }
}

bool isDir(FAT16File entry){
    return entry.attributes && ATTR_DIRECTORY;
}


//-----------------------------------------------------------------------------
// directoryNameChecksum()
// Returns an unsigned byte checksum computed on an unsigned byte
// array. The array must be 11 bytes long and is assumed to contain
// a name stored in the format of an MS-DOS directory entry.
// Passed: name Pointer to an unsigned byte array assumed to be
// 11 bytes long.
// Returns: Sum An 8-bit unsigned checksum of the array pointed
// to by name.
//------------------------------------------------------------------------------
unsigned char directoryNameChecksum (unsigned char *name){
    unsigned char sum = 0;
    for (short nameLen = 11; nameLen != 0; nameLen--) {
        // NOTE: The operation is an unsigned char rotate right
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *name++;
    }
    return sum;
}

uint16_t swapEndianness16Bit(uint16_t num) {
    uint16_t b0, b1;
    b0 = (num & 0x00ff) << 8u;
    b1 = (num & 0xff00) >> 8u;
    return b0 | b1;
}



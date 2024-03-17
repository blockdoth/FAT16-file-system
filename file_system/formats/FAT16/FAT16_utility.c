#include "FAT16_utility.h"

void writeSector(FormattedVolume *self, volume_ptr sector, void *data, uint32_t dataSize) {
    volume_ptr adjustedAddress =
            self->volumeInfo->dataSectionStart * self->volumeInfo->bytesPerSector +
            sector * self->volumeInfo->bytesPerSector;
    self->rawVolume->write(self->rawVolume, data,adjustedAddress, dataSize);
}

void writePartialSector(FormattedVolume* self, volume_ptr sector, uint32_t bytesOffset, void* data, uint32_t dataSize){
    volume_ptr adjustedAddress =
            self->volumeInfo->dataSectionStart * self->volumeInfo->bytesPerSector +
            sector * self->volumeInfo->bytesPerSector + bytesOffset;
    self->rawVolume->write(self->rawVolume, data,adjustedAddress, dataSize);
}

void* readSector(FormattedVolume* self, volume_ptr sector, uint32_t size){
    uint32_t adjustedAddress = self->volumeInfo->dataSectionStart * self->volumeInfo->bytesPerSector + sector * self->volumeInfo->bytesPerSector ;
    return self->rawVolume->read(self->rawVolume, adjustedAddress, size);
}

void writeFileEntry(FormattedVolume* self, FAT16File fileMetadata, volume_ptr entryTable, volume_ptr startSector, volume_ptr endSector){
    fileMetadata.fileClusterStart = startSector;
    fileMetadata.fileClusterEnd = endSector;

    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    int32_t i;
    FAT16File entry;
    for(i = 0; i < maxEntries; i++) {
        entry = readFileEntry(self, entryTable, i);
        if(entry.name[0] == 0x00 || entry.name[0] == 0xe5){
            break;
        }
    }
    uint32_t adjustedAddress = entryTable * self->volumeInfo->bytesPerCluster + i * FAT16_ENTRY_SIZE;
    self->rawVolume->write(self->rawVolume, &fileMetadata, adjustedAddress, FAT16_ENTRY_SIZE);
}

// tableStart => Cluster address of entry table
// index => index in said table
FAT16File readFileEntry(FormattedVolume* self, volume_ptr tableStart, uint32_t index){
    uint32_t adjustedAddress = tableStart * self->volumeInfo->bytesPerCluster + index * FAT16_ENTRY_SIZE;
    FAT16File* FileEntryPointer = (FAT16File *) self->rawVolume->read(self->rawVolume, adjustedAddress, FAT16_ENTRY_SIZE);
    FAT16File FileEntry = *FileEntryPointer;
    free(FileEntryPointer);
    return FileEntry;
}

void updateFAT16Entry(FormattedVolume* self, volume_ptr entryTable, FAT16File fat16File){
    uint32_t maxEntries = calculateMaxEntries(self, entryTable);
    for(int32_t i = 0; i < maxEntries; i++) {
        FAT16File entry = readFileEntry(self, entryTable, i);
        if(strcmp(entry.name, fat16File.name) == 0){
            uint32_t adjustedAddress = entryTable * self->volumeInfo->bytesPerCluster + i * FAT16_ENTRY_SIZE;
            self->rawVolume->write(self->rawVolume, &fat16File, adjustedAddress, FAT16_ENTRY_SIZE);
            break;
        }
    }
}


// nextSector => Cluster address of next entry
void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector){
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT1Start * self->volumeInfo->bytesPerCluster + 2 * index, 2);
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT2Start * self->volumeInfo->bytesPerCluster + 2 * index, 2);
}

uint16_t readFATS(FormattedVolume* self, uint32_t index){
    uint16_t* FAT1 = (uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT1Start * self->volumeInfo->bytesPerCluster + index * 2, 2);
    uint16_t* FAT2 = (uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT2Start * self->volumeInfo->bytesPerCluster + index * 2, 2);
    if(*FAT1 != *FAT2){
        printf("FATS are out of sync");
    }
    uint16_t FATEntry = *FAT1;
    free(FAT1);
    free(FAT2);
    return FATEntry;
}


volume_ptr resolveFileTable(FormattedVolume *self, Path path) {
    volume_ptr entryTable = self->volumeInfo->rootSectionStart;
    FAT16File entry;
    for(int i = 0; i < path.depth;i++){
        entry = findEntryInTable(self, entryTable, path.path[i]);
        entryTable = entry.fileClusterStart;
        if(entry.name[0] == 0){
            break;
        }
        if(strcmp((char*)entry.name, path.path[i]) != 0){
            #ifdef DEBUG_FAT16
            printf("ERROR: %s does not exist\n", entry.name);
            #endif
            return false;
        }
        if(entry.attributes != ATTR_DIRECTORY){
            #ifdef DEBUG_FAT16
            printf("ERROR: %s is not a directory\n", entry.name);
            #endif
            return false;
        }
        free(path.path[i]);
    }
    // TODO handle duplication
    return entryTable;
}


volume_ptr findFreeCluster(FormattedVolume* self){
    uint16_t entry;
    for (uint32_t i = 0; i < self->volumeInfo->FATEntryCount; i++) {
        entry = readFATS(self,i);
        if(entry == 0x00){
            return self->volumeInfo->dataSectionStart + i;
        }
    }
    return 0;
}

FAT16File findEntryInTable(FormattedVolume *self, volume_ptr startTable, char *name) {

    for (volume_ptr i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        FAT16File entry = readFileEntry(self, startTable, i);
        if(entry.name[0] != 0x00){
            if(strcmp(entry.name, name) == 0){
                // TODO support the actual FAT16 naming format and longer filenames
                #ifdef DEBUG_FAT16
                printf("Found %s of size %u in the FAT at sector %u pointing to sector %u\n",
                       entry.name, entry.fileSize, startTable, entry.fileClusterStart);
                #endif
                return entry;
            }
        }
    }

    return (FAT16File){
            .reserved = 1
    }; //TODO error handling and make this not hacky
}

uint32_t calculateMaxEntries(FormattedVolume* self, volume_ptr entryTable){
    if(self->volumeInfo->rootSectionStart == entryTable){
        return (self->volumeInfo->rootSectorCount * self->volumeInfo->bytesPerSector) / FAT16_ENTRY_SIZE;
    }else{
        return self->volumeInfo->bytesPerCluster / FAT16_ENTRY_SIZE;
    }
}

bool checkNamingCollusion(FormattedVolume* self, volume_ptr entryTable, char* name, bool lookingForDir){
    FAT16File entry = findEntryInTable(self, entryTable, name);
    if(strcmp(entry.name, name) == 0){
        if((entry.attributes && ATTR_DIRECTORY) == lookingForDir){
            return true;
        }
    }
    return false;
}

FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata){
    FAT16File fat16File;

    memcpy(fat16File.name, fileMetadata->name, FAT16_ENTRY_BASE_NAME_LENGTH); // TODO support long rickRoll names
    fat16File.name[10] = '\0';

    fat16File.attributes = convertToDirAttributes(fileMetadata); //TODO createFile converter
    fat16File.reserved = 0;
    fat16File.creationTimeTenth = fileMetadata->creationTimeTenth;
    fat16File.creationTime = fileMetadata->creationTime;
    fat16File.creationDate = fileMetadata->creationDate;
    fat16File.lastAccessedDate = fileMetadata->creationDate;
    fat16File.fileClusterStart = 0;
    fat16File.timeOfLastWrite = fileMetadata->creationTime;
    fat16File.dateOfLastWrite = fileMetadata->creationDate;
    fat16File.fileClusterEnd = 0; // TODO
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
            0, //TODO
            0, // Not relevant
            0, // Not relevant
            0,
            0,
            0, // TODO
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

    volume_ptr rootSectorStart = bootSector.reservedSectorCount + bootSector.sectorsPersFAT * bootSector.numberOfFATs;
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



bool checkFAT16Compatible(RawVolume *raw_volume) {
    return raw_volume->volumeSize < FAT16_MINIMUM_SIZE;
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



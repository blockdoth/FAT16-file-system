#include "FAT16.h"
#include "FAT16_debug.h"
#include "FAT16_utility.h"


#define DEBUG

FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(check_FAT16_formattible(volume)){
        return (void*)0;
    }
    printf("Formatting a rawVolume of size %u bytes\n", volume->volumeSize);

    BootSector bootSector = initBootSector(volume->volumeSize);
    FATVolumeInfo* volumeInfo = initFATVolumeInfo(bootSector);
    FormattedVolume* formattedVolume = initFormattedVolume(volume, volumeInfo);

    // Write the bootsector to the rawVolume
    volume->write(volume, &bootSector,0, sizeof(bootSector));


    //Zero out FAT tables (both of them)
    volume->write(volume, NULL, volumeInfo->FAT1Start * volumeInfo->bytesPerCluster, 2 * volumeInfo->FATEntryCount);
    //Zero out root directory
    volume->write(volume, NULL, volumeInfo->rootSectionStart * volumeInfo->bytesPerCluster, volumeInfo->rootSectorCount);

    uint16_t firstSectors[] = {
            swapEndianness16Bit(0xFFF8),
            swapEndianness16Bit(0xFFFF)
    };
    // Set the first cluster to 0xFFF8, indicating the first cluster of a file
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian
    volume->write(volume, &firstSectors, volumeInfo->FAT1Start * volumeInfo->bytesPerCluster, 4);
    volume->write(volume, &firstSectors, volumeInfo->FAT2Start * volumeInfo->bytesPerCluster, 4);

    #ifdef DEBUG
    printBootSector((BootSector*) volume->read(volume, 0, sizeof(bootSector)));
    printFATTable(formattedVolume);
    printFAT16Layout(formattedVolume);
    #endif
    return formattedVolume;
}


bool FAT16WriteFile(FormattedVolume * self, FileMetadata* fileMetadata, void* fileData, char* path){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);
    volume_ptr startSector = findFreeCluster(self);
    if(startSector == 0){
        return false;
    }

    volume_ptr currentSector = startSector;
    volume_ptr prevSector;
    volume_ptr currentDataPointer = 0;
    uint32_t bytesLeftToWrite = fat16File.fileSize;
    while (bytesLeftToWrite > self->volumeInfo->bytesPerCluster){
        // Write the data
        writeSector(self,
                    fileData + currentDataPointer,
                    currentSector,
                    self->volumeInfo->bytesPerCluster);
        prevSector = currentSector;

        // Write the address of the next sector to the FATS
        writeFATS(self,prevSector, &currentSector);
        currentSector = findFreeCluster(self);

        currentDataPointer += self->volumeInfo->bytesPerCluster;
        bytesLeftToWrite -= self->volumeInfo->bytesPerCluster;
    }
    if(bytesLeftToWrite > 0){
        writeSector(self,
                    fileData + currentDataPointer,
                    currentSector,
                    bytesLeftToWrite
                    );
    }
    // End cluster of sector in FATS
    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self,currentSector, &endSector);

    // Write to the metadata to the root directory
    fat16File.fileClusterEnd = startSector;
    fat16File.fileClusterEnd = currentSector;
    volume_ptr entryTable = resolveFile(self, path, fileMetadata->name);
    writeFileEntry(self, fat16File, entryTable, startSector, currentSector);

    #ifdef DEBUG
    printf("Wrote %s of size %u bytes from sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startSector, currentSector);
    //printFATTable(self);
    //printRootSectorShort(self);
    printTree(self);
    #endif



    return true;
}

bool FAT16WriteDir(FormattedVolume* self, FileMetadata* fileMetadata, char* path){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);

    volume_ptr entryTable = resolveFile(self, path, fileMetadata->name);

    volume_ptr subDirEntryCluster = findFreeCluster(self);

    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster - self->volumeInfo->dataSectionStart, &endSector);

    writeFileEntry(self, fat16File, entryTable, subDirEntryCluster, subDirEntryCluster); //TODO support multi cluster DIRS

    #ifdef DEBUG
    printf("Created a directory %s at sector %u\n", fat16File.name, entryTable);
    //printFATTable(self);
    printTree(self);
    //printRootSectorShort(self);
    #endif
    return true;
}


void* FAT16ReadFile(FormattedVolume* self, FileMetadata* fileMetadata, char* path) {
    volume_ptr entryTable = resolveFile(self, path, fileMetadata->name);
    FAT16File fat16File = findEntryInTable(self, fileMetadata->name, entryTable);
    if(fat16File.fileClusterStart == 0){
        #ifdef DEBUG
        printf("ERROR: Could not find file with name %s\n", fileMetadata->name);
        #endif
        return NULL;
    }
    char* readFile = malloc(fileMetadata->fileSize);
    void* startOfFile = readFile;

    for (uint16_t i = fat16File.fileClusterStart; i <= fat16File.fileClusterEnd; i++) {
        // Prevent overwritten the mallocéd region
        uint32_t length;
        if(fileMetadata->fileSize < self->volumeInfo->bytesPerCluster){
            length = fileMetadata->fileSize;
        }else{
            length = self->volumeInfo->bytesPerCluster;
        }
        memcpy(readFile, readData(self, i), length);
        readFile += self->volumeInfo->bytesPerCluster;
    }
    return startOfFile; // TODO Add more metadata
}


volume_ptr resolveFile(FormattedVolume* self, char* path, char* fileName){
    Path resolvedPath = parsePath(path);
    volume_ptr entryTable = self->volumeInfo->rootSectionStart;
    FAT16File entry;
    uint8_t pathLength = resolvedPath.depth;
    while (pathLength-- > 0){
        entry = findEntryInTable(self, *resolvedPath.path, entryTable);
        entryTable = entry.fileClusterStart;
        if(entry.name[0] == 0){
            break;
        }
        if(strcmp((char*)entry.name, *resolvedPath.path) != 0){
            #ifdef DEBUG
            printf("ERROR: %s does not exist\n", entry.name);
            #endif
            return false;
        }
        if(entry.attributes != ATTR_DIRECTORY){
            #ifdef DEBUG
            printf("ERROR: %s is not a directory\n", entry.name);
            #endif
            return false;
        }
        resolvedPath.path++;
    }
//    entry = findEntryInTable(self, fileName, entryTable);
//    if(strcmp(entry.name, fileName) == 0){
//        printf("ERROR: %s already exists\n", entry.name);
//        return false;
//    }
    destroyPath(resolvedPath);
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


FAT16File findEntryInTable(FormattedVolume* self, char* fileName, volume_ptr startTable){
    FAT16File entry;
    for (volume_ptr i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = readFileEntry(self, startTable, i);
        if(entry.name[0] != 0x00){
            if(strcmp(entry.name, fileName) == 0){
                // TODO support the actual FAT16 naming format and longer filenames
                #ifdef DEBUG
                printf("Found %s of size %u in the FAT at sector %u pointing to sector %u\n",
                       entry.name, entry.fileSize, startTable, entry.fileClusterStart);
                #endif
                return entry;
            }
        }
    }

    return (FAT16File){
            .fileClusterStart = 0
    }; //TODO error handling and make this not hacky
}

void writeSector(FormattedVolume* self, void* data, volume_ptr sector, uint32_t dataSize){
    volume_ptr adjustedAddress =
            self->volumeInfo->dataSectionStart * self->volumeInfo->bytesPerSector +
            sector * self->volumeInfo->bytesPerSector;
    self->rawVolume->write(self->rawVolume, data,adjustedAddress, dataSize);
}

void* readData(FormattedVolume* self, volume_ptr sector){
    uint32_t adjustedAddress = self->volumeInfo->dataSectionStart * self->volumeInfo->bytesPerSector + sector * self->volumeInfo->bytesPerSector ;
    return self->rawVolume->read(self->rawVolume, adjustedAddress, self->volumeInfo->bytesPerCluster);
}

// nextSector => Cluster address of next entry
void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector){
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT1Start * self->volumeInfo->bytesPerCluster + 2 * index, 2);
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT2Start * self->volumeInfo->bytesPerCluster + 2 * index, 2);
}
uint16_t readFATS(FormattedVolume* self, uint32_t index){
    uint16_t FAT1 = *(uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT1Start * self->volumeInfo->bytesPerCluster + index * 2, 2);
    uint16_t FAT2 = *(uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT2Start * self->volumeInfo->bytesPerCluster + index * 2, 2);
    if(FAT1 != FAT2){
        printf("FATS are out of sync");
    }
    return FAT1;
}


// tableStart => Cluster address of entry table
// index => index in said table
FAT16File readFileEntry(FormattedVolume* self, volume_ptr tableStart, uint32_t index){
    uint32_t adjustedAddress = tableStart * self->volumeInfo->bytesPerCluster + index * FAT16_ENTRY_SIZE;
    return *(FAT16File *) self->rawVolume->read(self->rawVolume, adjustedAddress, FAT16_ENTRY_SIZE);
}

void writeFileEntry(FormattedVolume* self, FAT16File fileMetadata, volume_ptr tableStart, volume_ptr startSector, volume_ptr endSector){
    fileMetadata.fileClusterStart = startSector;
    fileMetadata.fileClusterEnd = endSector;
    FAT16File entry;
    int32_t i;
    for(i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = readFileEntry(self, tableStart, i);
        if(entry.name[0] == 0x00 || entry.name[0] == 0xe5){
            break;
        }
    }
    uint32_t adjustedAddress = tableStart * self->volumeInfo->bytesPerCluster + i * FAT16_ENTRY_SIZE;
    self->rawVolume->write(self->rawVolume, &fileMetadata, adjustedAddress, FAT16_ENTRY_SIZE);
}


#undef DEBUG
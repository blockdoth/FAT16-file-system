#include "FAT16.h"
#include "FAT16_debug.h"
#include "FAT16_utility.h"



FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo* volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->volumeInfo = volumeInfo;
    formattedVolume->readFile = FAT16ReadFile;
    formattedVolume->createFile = FAT16WriteFile;
    formattedVolume->createDir = FAT16WriteDir;
    formattedVolume->checkDir = FAT16CheckDir;
    formattedVolume->checkFile = FAT16CheckFile;
    formattedVolume->updateFile = FAT16UpdateFile;
    return formattedVolume;
}


FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(checkFAT16Compatible(volume)){
        return NULL; //TODO handle
    }

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
    // Set the first cluster to 0xFFF8, indicating the first cluster of a rickRoll
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian
    volume->write(volume, &firstSectors, volumeInfo->FAT1Start * volumeInfo->bytesPerCluster, 4);
    volume->write(volume, &firstSectors, volumeInfo->FAT2Start * volumeInfo->bytesPerCluster, 4);

    #ifdef DEBUG_FAT16
    printf("Formatting a rawVolume of size %u bytes\n", volume->volumeSize);
    printFATTable(formattedVolume);
    printFAT16Layout(formattedVolume);
    #endif
    return formattedVolume;
}




bool FAT16WriteFile(FormattedVolume * self, Path path, FileMetadata* fileMetadata, void* fileData){
    volume_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, false) == true){
        // Naming collision, TODO error handling
        return false;
    }
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata
    volume_ptr startCluster = findFreeCluster(self);
    if(startCluster == 0){
        return false;
    }

    volume_ptr currentCluster = startCluster;
    volume_ptr prevSector;
    volume_ptr currentDataPointer = 0;
    uint32_t bytesLeftToWrite = fat16File.fileSize;
    while (bytesLeftToWrite > self->volumeInfo->bytesPerCluster){
        // Write the data
        writeSector(self,
                    currentCluster,
                    fileData + currentDataPointer,
                    self->volumeInfo->bytesPerCluster);
        prevSector = currentCluster;

        // Write the address of the next sector to the FATS
        writeFATS(self,prevSector, &currentCluster);
        currentCluster = findFreeCluster(self);

        currentDataPointer += self->volumeInfo->bytesPerCluster;
        bytesLeftToWrite -= self->volumeInfo->bytesPerCluster;
    }
    if(bytesLeftToWrite > 0){
        writeSector(self,
                    currentCluster,
                    fileData + currentDataPointer,
                    bytesLeftToWrite
        );
    }
    // End cluster of sector in FATS
    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, currentCluster, &endSector);

    // Write to the metadata to the root directory
    fat16File.fileClusterEnd = startCluster;
    fat16File.fileClusterEnd = currentCluster;
    writeFileEntry(self, fat16File, entryTable, startCluster, currentCluster);

    #ifdef DEBUG_FAT16
    printf("Wrote %s of size %u bytes from sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startCluster, currentCluster);
    //printFATTable(self);
    //printRootSectorShort(self);
    printTree(self);
    #endif
    destroyPath(path);
    return true;
}

uint32_t FAT16UpdateFile(FormattedVolume* self, Path path, void* fileData, uint32_t dataSize){
    volume_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false){
        // File not found // TODO error handling
        return false;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);

    uint32_t clusterCount = fat16File.fileClusterEnd - fat16File.fileClusterStart;
    uint32_t offset = fat16File.fileSize;
    if(clusterCount > 0){
        offset -= (clusterCount - 1) * self->volumeInfo->bytesPerCluster;
    }

    //volume_ptr startPtr = (allocatedBytes - fat16File.fileSize ) % self->volumeInfo->bytesPerCluster;
    volume_ptr currentCluster = fat16File.fileClusterEnd;
    uint32_t bytesLeftInCluster = self->volumeInfo->bytesPerCluster - offset;
    uint32_t bytesLeftToWrite = dataSize;
    uint32_t writeSize = 0;
    if(offset > 0){
        if(dataSize >= bytesLeftInCluster){
            writeSize = bytesLeftInCluster;
        }else{
            writeSize = dataSize;
        }
        writePartialSector(self,
                           currentCluster,
                           offset,
                           fileData,
                           writeSize
        );
        bytesLeftToWrite -= writeSize;
        fat16File.fileSize += writeSize;
    }

    volume_ptr currentDataPointer = writeSize;
    volume_ptr prevSector;
    while (bytesLeftToWrite > self->volumeInfo->bytesPerCluster){
        // Write the data
        writeSector(self,
                    currentCluster,
                    fileData + currentDataPointer,
                    self->volumeInfo->bytesPerCluster);
        prevSector = currentCluster;

        // Write the address of the next sector to the FATS
        writeFATS(self,prevSector, &currentCluster);
        currentCluster = findFreeCluster(self);

        currentDataPointer += self->volumeInfo->bytesPerCluster;
        bytesLeftToWrite -= self->volumeInfo->bytesPerCluster;
    }
    if(bytesLeftToWrite > 0){
        writeSector(self,
                        currentCluster,
                           fileData + currentDataPointer,
                           bytesLeftToWrite
        );
    }
    fat16File.fileClusterEnd = currentCluster;
    updateFAT16Entry(self, entryTable, fat16File);
    destroyPath(path);
    return fat16File.fileSize;
}

bool FAT16WriteDir(FormattedVolume* self, Path path, FileMetadata* fileMetadata){
    volume_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, true) == true){
        // Naming collisions // TODO error handling
        return false;
    }
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    volume_ptr subDirEntryCluster = findFreeCluster(self);

    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster - self->volumeInfo->dataSectionStart, &endSector);

    writeFileEntry(self, fat16File, entryTable, subDirEntryCluster, subDirEntryCluster); //TODO support multi cluster DIRS

    #ifdef DEBUG_FAT16
    printf("Created a directory %s at sector %u\n", fat16File.name, entryTable);
    //printFATTable(self);
    printTree(self);
    printRootSectorShort(self);
    #endif
    free(fileMetadata->name);
    destroyPath(path);
    return true;
}


void* FAT16ReadFile(FormattedVolume* self, Path path) {
    volume_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false){
        // File not found // TODO error handling
        return NULL;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    if(fat16File.fileClusterStart == 0){
        #ifdef DEBUG_FAT16
        printf("ERROR: Could not find rickRoll with name %s\n", name);
        #endif
        return NULL;
    }
    char* readFile = malloc(fat16File.fileSize);
    void* startOfFile = readFile;

    for (uint16_t i = fat16File.fileClusterStart; i <= fat16File.fileClusterEnd; i++) {
        // Prevent overwritten the malloc'ed region
        uint32_t chunkLength;
        if(fat16File.fileSize < self->volumeInfo->bytesPerCluster){
            chunkLength = fat16File.fileSize;
        }else{
            chunkLength = self->volumeInfo->bytesPerCluster;
        }
        void* chunk = readSector(self, i, chunkLength);
        memcpy(readFile, chunk , chunkLength);
        free(chunk);
        readFile += self->volumeInfo->bytesPerCluster;
    }
    destroyPath(path);
    return startOfFile; // TODO Add more metadata
}

bool FAT16CheckFile(FormattedVolume* self, Path path){
    volume_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);
    return checkNamingCollusion(self, entryTable, name, false);
}
bool FAT16CheckDir(FormattedVolume* self, Path path){
    volume_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);
    return checkNamingCollusion(self, entryTable, name, true);
}




#include "FAT16.h"
#include "FAT16_debug.h"
#include "FAT16_utility.h"


FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo* volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->volumeInfo = volumeInfo;
    formattedVolume->read = FAT16ReadFile;
    formattedVolume->writeFile = FAT16WriteFile;
    formattedVolume->writeDir = FAT16WriteDir;
    return formattedVolume;
}


FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(checkFAT16Compatible(volume)){
        return NULL; //TODO handle
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

    #ifdef DEBUG_FAT16
    BootSector* returnedBootSector = (BootSector*) volume->read(volume, 0, sizeof(bootSector));
    printBootSector(returnedBootSector);
    free(returnedBootSector);
    printFATTable(formattedVolume);
    printFAT16Layout(formattedVolume);
    #endif
    return formattedVolume;
}


bool FAT16WriteFile(FormattedVolume * self, FileMetadata* fileMetadata, void* fileData, char* path){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata
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

    #ifdef DEBUG_FAT16
    printf("Wrote %s of size %u bytes from sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startSector, currentSector);
    //printFATTable(self);
    //printRootSectorShort(self);
    printTree(self);
    #endif
    return true;
}

bool FAT16WriteDir(FormattedVolume* self, FileMetadata* fileMetadata, char* path){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    volume_ptr entryTable = resolveFile(self, path, fileMetadata->name);

    volume_ptr subDirEntryCluster = findFreeCluster(self);

    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster - self->volumeInfo->dataSectionStart, &endSector);

    writeFileEntry(self, fat16File, entryTable, subDirEntryCluster, subDirEntryCluster); //TODO support multi cluster DIRS

    #ifdef DEBUG_FAT16
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
        #ifdef DEBUG_FAT16
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
        void* chunk = readSector(self, i);
        memcpy(readFile,chunk , length);
        free(chunk);
        readFile += self->volumeInfo->bytesPerCluster;
    }
    return startOfFile; // TODO Add more metadata
}



#undef DEBUG_FAT16
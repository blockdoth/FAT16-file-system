#include "FAT16.h"
#include "FAT16_debug.h"
#include "FAT16_utility.h"



FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo* volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->info = volumeInfo;
    formattedVolume->readFile = FAT16ReadFile;
    formattedVolume->readFileSection = FAT16ReadFileSection;
    formattedVolume->createFile = FAT16WriteFile;
    formattedVolume->createDir = FAT16WriteDir;
    formattedVolume->checkDir = FAT16CheckDir;
    formattedVolume->checkFile = FAT16CheckFile;
    formattedVolume->updateFile = FAT16UpdateFile;
    formattedVolume->deleteDir = FAT16DeleteDir;
    formattedVolume->deleteFile = FAT16DeleteFile;
    formattedVolume->toString = FAT16ToTreeString;
        ;
    return formattedVolume;
}


FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(!checkFAT16Compatible(volume)){
        return NULL;
    }

    BootSector bootSector = initBootSector(volume->volumeSize);
    FATVolumeInfo* volumeInfo = initFATVolumeInfo(bootSector);
    FormattedVolume* formattedVolume = initFormattedVolume(volume, volumeInfo);

    // Write the bootsector to the rawVolume
    writeSector(formattedVolume, 0, &bootSector, 90);

    //Zero out FAT tables (both of them)
    clearSectors(formattedVolume,volumeInfo->FAT1Start, volumeInfo->FATTableSectorCount);
    clearSectors(formattedVolume,volumeInfo->FAT2Start, volumeInfo->FATTableSectorCount);
    //Zero out root directory
    clearSectors(formattedVolume, volumeInfo->rootSectionStart, volumeInfo->rootSectorCount);

    uint16_t firstSectors[] = {
            swapEndianness16Bit(0xFFF8),
            swapEndianness16Bit(0xFFFF)
    };
    // Set the first cluster to 0xFFF8, indicating the first cluster of a rickRoll
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian

    writeSector(formattedVolume,volumeInfo->FAT1Start, firstSectors, 4);
    writeSector(formattedVolume,volumeInfo->FAT2Start, firstSectors, 4);

    #ifdef DEBUG_FAT16
    printf("Formatting a rawVolume of size %u bytes\n", volume->volumeSize);
    printBootSector((BootSector*)readSectorSize(formattedVolume, 0, sizeof(bootSector)));


    //printBootSector(&bootSector);
    printFATTable(formattedVolume);
    printFAT16Layout(formattedVolume);
    #endif
    return formattedVolume;
}



FS_STATUS_CODE FAT16WriteFile(FormattedVolume * self, Path path, FileMetadata* fileMetadata, void* fileData){
    sector_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, false) == true){
        return FS_FILE_ALREADY_EXISTS;
    }
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata
    sector_ptr startCluster = findFreeCluster(self);
    sector_ptr currentCluster;
    sector_ptr prevCluster = startCluster;
    uint32_t currentDataPointer = 0;
    uint32_t bytesInSector = self->info->bytesPerSector;
    uint32_t bytesLeftToWrite = fat16File.fileSize;
    while (bytesLeftToWrite > bytesInSector){
        // Write the data

        uint8_t sectorInCluster = 0;
        currentCluster = findFreeCluster(self);
        while(bytesLeftToWrite > bytesInSector && sectorInCluster < self->info->sectorsPerCluster){
            writeDataSector(self, currentCluster * self->info->sectorsPerCluster,
                            fileData + currentDataPointer, bytesInSector);
            bytesLeftToWrite -= bytesInSector;
            currentDataPointer += bytesInSector;
            sectorInCluster++;
        }
        writeFATS(self,prevCluster, &currentCluster);
        prevCluster = currentCluster;
        currentCluster++;
    }
    if(bytesLeftToWrite > 0){
        currentCluster = findFreeCluster(self);
        writeDataSector(self, currentCluster * self->info->sectorsPerCluster,
                        fileData + currentDataPointer, bytesInSector);
        bytesLeftToWrite;
        writeFATS(self,prevCluster, &currentCluster);
    }
    uint16_t endOfFile = swapEndianness16Bit(0xFFF8);
    writeFATS(self,currentCluster, &endOfFile);

    // Write to the metadata to the right table
    fat16File.fileClusterStart = startCluster;
    writeFileEntry(self, fat16File, entryTable);

    #ifdef DEBUG_FAT16
    printf("Wrote %s of size %u bytes from data sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startCluster, currentCluster);
    //printFATTable(self);
    //printRootSectorShort(self);
    printTree(self);
    #endif
    destroyPath(path);
    return FS_SUCCES;
}

uint32_t FAT16UpdateFile(FormattedVolume* self, Path path, void* fileData, uint32_t dataSize){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false){
        return -1;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);

    //TODO implement

    return fat16File.fileSize;
}

FS_STATUS_CODE FAT16WriteDir(FormattedVolume* self, Path path, FileMetadata* fileMetadata){
    sector_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, true) == true){
        return FS_DIRECTORY_ALREADY_EXISTS;
    }
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    sector_ptr subDirEntryCluster = findFreeCluster(self);

    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster, &endSector);

    fat16File.fileClusterStart = subDirEntryCluster;
    writeFileEntry(self, fat16File, entryTable); //TODO support multi cluster DIRS

    #ifdef DEBUG_FAT16
    printf("Created a directory %s at sector %u\n", fat16File.name, entryTable);
    //printFATTable(self);
    printTree(self);
    printRootSectorShort(self);
    #endif
    free(fileMetadata->name);
    destroyPath(path);
    return FS_SUCCES;
}


void* FAT16ReadFile(FormattedVolume* self, Path path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == FS_FILE_NOT_FOUND){
        return NULL;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    char* file = malloc(fat16File.fileSize);
    uint32_t dataPointer = 0;
    uint32_t bytesInSector = self->info->bytesPerSector;

    uint16_t currentFATEntry = fat16File.fileClusterStart;
    uint32_t bytesLeftToRead = fat16File.fileSize;
    do{
        uint8_t sectorIndex = 0;
        while(bytesLeftToRead > 0 && sectorIndex < self->info->sectorsPerCluster){
            uint32_t readChunkSize = bytesInSector;
            if(bytesLeftToRead < self->info->bytesPerSector){
                readChunkSize = bytesLeftToRead;
            }
            void* chunk = readClusterSector(self, currentFATEntry, sectorIndex); //TODO directly write from volume
            memcpy(file + dataPointer, chunk, bytesInSector);
            free(chunk);
            dataPointer += bytesInSector;
            bytesLeftToRead -= bytesInSector;
        }
        currentFATEntry = readFATS(self, currentFATEntry);
    } while(currentFATEntry != swapEndianness16Bit(0xFFF8));
    destroyPath(path);
    return file;
}

void* FAT16ReadFileSection(FormattedVolume* self, Path path, uint32_t offset, uint32_t chunkSize){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == FS_FILE_NOT_FOUND){
        return NULL;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    char* fileChunk = malloc(chunkSize);
    uint32_t dataPointer = 0;
    uint32_t bytesInSector = self->info->bytesPerSector;

    uint16_t currentFATEntry = fat16File.fileClusterStart;
    uint16_t prevFATEntry;
    uint32_t bytesLeftToRead = chunkSize;
    uint32_t totalSectorCount = 0;
    uint32_t sectorsOffset = offset / self->info->bytesPerSector;
    uint32_t bytesOffset = offset % self->info->bytesPerSector;
    uint8_t sectorIndex;
    do{
        sectorIndex = 0;
        while(bytesLeftToRead > 0 && sectorIndex < self->info->sectorsPerCluster){
            totalSectorCount++;
            if(totalSectorCount > sectorsOffset){
                uint32_t readChunkSize = bytesInSector;
                if(bytesLeftToRead < self->info->bytesPerSector){
                    readChunkSize = bytesLeftToRead;
                }
                void* chunk = readClusterSector(self, currentFATEntry, sectorIndex); //TODO directly write from volume
                memcpy(fileChunk + dataPointer, chunk + bytesOffset, readChunkSize);
                free(chunk);
                sectorIndex++;
                dataPointer += bytesInSector;
                bytesLeftToRead -= bytesInSector;
            }
        }
        prevFATEntry = currentFATEntry;
        currentFATEntry = readFATS(self, currentFATEntry);
    } while(currentFATEntry != swapEndianness16Bit(0xFFF8));

    if(bytesOffset > 0){
        void* chunk = readClusterSector(self, prevFATEntry, sectorIndex); //TODO directly write from volume
        memcpy(fileChunk + dataPointer, chunk + bytesOffset, bytesOffset);
    }
    destroyPath(path);
    return fileChunk;
}


FS_STATUS_CODE FAT16CheckFile(FormattedVolume* self, Path path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);
    return checkNamingCollusion(self, entryTable, name, false);
}
FS_STATUS_CODE FAT16CheckDir(FormattedVolume* self, Path path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);
    return checkNamingCollusion(self, entryTable, name, true);
}

FS_STATUS_CODE FAT16DeleteFile(FormattedVolume* self, Path path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);

    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, false);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printTree(self);
    #endif
    return statusCode;
}
FS_STATUS_CODE FAT16DeleteDir(FormattedVolume *self, Path path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    destroyPath(path);
    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, true);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printTree(self);
    #endif
    return statusCode;
}

char* FAT16ToTreeString(FormattedVolume* self, Path path){
    return printTreeToString(self);
}

#include "FAT16.h"
#include "FAT16_debug.h"
#include "FAT16_utility.h"



FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo* volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->info = (VolumeInfo*) volumeInfo;
    formattedVolume->readFile = FAT16ReadFile;
    formattedVolume->readFileSection = FAT16ReadFileSection;
    formattedVolume->createFile = FAT16WriteFile;
    formattedVolume->createDir = FAT16WriteDir;
    formattedVolume->checkDir = FAT16CheckDir;
    formattedVolume->checkFile = FAT16CheckFile;
    formattedVolume->updateFile = FAT16UpdateFile;
    formattedVolume->expandFile = FAT16ExpandFile;
    formattedVolume->deleteDir = FAT16DeleteDir;
    formattedVolume->deleteFile = FAT16DeleteFile;
    formattedVolume->toString = FAT16ToTreeString;
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
    printBootSector((BootSector*)readSector(formattedVolume, 0));


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

    sector_ptr startCluster = findFreeClusterInFAT(self);
    sector_ptr currentCluster = startCluster;
    sector_ptr prevCluster;
    uint32_t currentDataPointer = 0;
    uint32_t bytesInSector = self->info->FAT16.bytesPerSector;
    uint32_t bytesLeftToWrite = fat16File.fileSize;
    uint8_t sectorInCluster;
    while (bytesLeftToWrite > 0){
        // Write the data
        sectorInCluster = 0;

        uint32_t writeSize = bytesInSector;
        while(bytesLeftToWrite > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToWrite < bytesInSector){
                writeSize = bytesLeftToWrite; // Prevent unwanted data being written
            }
            writeClusterSector(self, currentCluster, sectorInCluster,
                            fileData + currentDataPointer, writeSize);
            bytesLeftToWrite -= writeSize;
            currentDataPointer += writeSize;
            sectorInCluster++;
        }
        prevCluster = currentCluster;
        currentCluster = findFreeClusterInFAT(self);

        writeFATS(self,prevCluster - self->info->FAT16.dataSectionStart, &currentCluster);
    }

    uint16_t endOfFile = swapEndianness16Bit(0xFFF8);
    writeFATS(self,prevCluster - self->info->FAT16.dataSectionStart, &endOfFile);

    // Write to the metadata to the right table
    fat16File.fileClusterStart = startCluster;
    writeFileEntry(self, fat16File, entryTable);

    #ifdef DEBUG_FAT16
    printf("Wrote %s of size %u bytes from data sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startCluster, currentCluster);
    printFATTable(self);
    printTree(self);
//    printRootSectorShort(self);
    #endif
    destroyPath(path);
    return FS_SUCCES;
}

uint32_t FAT16UpdateFile(FormattedVolume* self, Path path, void* fileData, uint32_t dataSize, uint32_t offset){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false) return -1;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    uint32_t bytesLeftToWrite = dataSize;
    uint32_t currentDataPointer = 0;

    // Fills up the last half written sector
    uint32_t offsetInSector = offset % self->info->FAT16.bytesPerSector;
    uint32_t startSector = (offset / self->info->FAT16.bytesPerSector) % self->info->FAT16.sectorsPerCluster;
    uint32_t startCluster = fat16File.fileClusterStart + (offset / self->info->FAT16.bytesPerCluster);
    uint8_t sectorInCluster = (offset % self->info->FAT16.bytesPerCluster) / self->info->FAT16.bytesPerSector;;
    uint32_t writeSize = self->info->FAT16.bytesPerSector - offsetInSector;
    if(writeSize > dataSize){
        writeSize = dataSize;
    }
    updateClusterSector(self, startCluster, startSector, fileData, writeSize, offsetInSector);
    bytesLeftToWrite -= writeSize;
    currentDataPointer += writeSize;
    sectorInCluster++;

    // Fills up the last half written cluster
    sector_ptr currentCluster = startCluster;
    uint32_t newDataSize = 0;
    if(dataSize > fat16File.fileSize){
        newDataSize = dataSize - fat16File.fileSize;
    }
    while (bytesLeftToWrite > newDataSize){
        // Write the data
        writeSize = self->info->FAT16.bytesPerSector;
        while(bytesLeftToWrite > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToWrite < self->info->FAT16.bytesPerSector){
                writeSize = bytesLeftToWrite; // Prevent unwanted data being written
            }
            updateClusterSector(self, currentCluster, sectorInCluster,
                               fileData + currentDataPointer, writeSize, 0);
            bytesLeftToWrite -= writeSize;
            currentDataPointer += writeSize;
            sectorInCluster++;
        }
        sectorInCluster = 0;
        currentCluster++;
        currentCluster = readFATS(self, currentCluster);
    }
    sector_ptr prevCluster = currentCluster;
    // Write new data
    if(bytesLeftToWrite > 0 ){
        while (bytesLeftToWrite > 0){
            // Write the data
            sectorInCluster = 0;
            currentCluster = findFreeClusterInFAT(self);
            uint32_t writeSize = self->info->FAT16.bytesPerSector;
            while(bytesLeftToWrite > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
                if(bytesLeftToWrite < self->info->FAT16.bytesPerSector){
                    writeSize = bytesLeftToWrite; // Prevent unwanted data being written
                }
                writeClusterSector(self, currentCluster, sectorInCluster,
                                   fileData + currentDataPointer, writeSize);
                bytesLeftToWrite -= writeSize;
                currentDataPointer += writeSize;
                sectorInCluster++;
            }
            writeFATS(self,prevCluster - self->info->FAT16.dataSectionStart, &currentCluster);
            prevCluster = currentCluster;
            currentCluster++;
        }
        uint16_t endOfFile = swapEndianness16Bit(0xFFF8);
        writeFATS(self,currentCluster, &endOfFile);
    }

    // Write to the metadata to the right table
    fat16File.fileClusterStart = startCluster;
    fat16File.fileSize += newDataSize;
    writeFileEntry(self, fat16File, entryTable);

#ifdef DEBUG_FAT16
    printf("Updated %s, new size %u bytes from data sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startCluster, currentCluster);
    printFATTable(self);
    printTree(self);
//    printRootSectorShort(self);
#endif
    destroyPath(path);
    return fat16File.fileSize;
}
uint32_t FAT16ExpandFile(FormattedVolume* self, Path path, void* data, uint32_t dataSize){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false) return -1;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    uint32_t bytesLeftToWrite = dataSize;
    uint32_t currentDataPointer = 0;

    // Fills up the last half written sector
    uint32_t offsetInSector = fat16File.fileSize % self->info->FAT16.bytesPerSector;
    uint32_t startSector = (fat16File.fileSize / self->info->FAT16.bytesPerSector) % self->info->FAT16.sectorsPerCluster;
    uint32_t startCluster = fat16File.fileClusterStart + (fat16File.fileSize / self->info->FAT16.bytesPerCluster);
    uint8_t sectorInCluster = (fat16File.fileSize % self->info->FAT16.bytesPerCluster) / self->info->FAT16.bytesPerSector;;
    uint32_t writeSize = self->info->FAT16.bytesPerSector - offsetInSector;
    if(writeSize > dataSize){
        writeSize = dataSize;
    }
    updateClusterSector(self, startCluster, startSector, data, writeSize, offsetInSector);
    bytesLeftToWrite -= writeSize;
    currentDataPointer += writeSize;
    sectorInCluster++;

    // Fills up the last half written cluster
    sector_ptr currentCluster = startCluster;
    sector_ptr prevCluster = currentCluster;
    if(bytesLeftToWrite > 0){
        while (bytesLeftToWrite > 0){
            // Write the data
            writeSize = self->info->FAT16.bytesPerSector;
            while(bytesLeftToWrite > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
                if(bytesLeftToWrite < self->info->FAT16.bytesPerSector){
                    writeSize = bytesLeftToWrite; // Prevent unwanted data being written
                }
                writeClusterSector(self, currentCluster, sectorInCluster,
                                   data + currentDataPointer, writeSize);
                bytesLeftToWrite -= writeSize;
                currentDataPointer += writeSize;
                sectorInCluster++;
            }
            sectorInCluster = 0;
            currentCluster = findFreeClusterInFAT(self);
            writeFATS(self,prevCluster - self->info->FAT16.dataSectionStart, &currentCluster);
            prevCluster = currentCluster;
            currentCluster++;
        }
        uint16_t endOfFile = swapEndianness16Bit(0xFFF8);
        writeFATS(self,currentCluster, &endOfFile);
    }

    // Write to the metadata to the right table
    fat16File.fileClusterStart = startCluster;
    fat16File.fileSize += dataSize;
    writeFileEntry(self, fat16File, entryTable);

    #ifdef DEBUG_FAT16
    printf("Updated %s, new size %u bytes from data sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startCluster, currentCluster);
    printFATTable(self);
    printTree(self);
//    printRootSectorShort(self);
    #endif
    destroyPath(path);
    return fat16File.fileSize;
}


FS_STATUS_CODE FAT16WriteDir(FormattedVolume* self, Path path, FileMetadata* fileMetadata){
    sector_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, true) == true){
        return FS_DIRECTORY_ALREADY_EXISTS;
    }
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    sector_ptr subDirEntryCluster = findFreeClusterInFAT(self);
    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster - self->info->FAT16.dataSectionStart, &endSector);

    fat16File.fileClusterStart = subDirEntryCluster ;
    writeFileEntry(self, fat16File, entryTable); //TODO support multi cluster DIRS

    #ifdef DEBUG_FAT16
    printf("Created a directory %s at sector %u\n", fat16File.name, entryTable);
//    printFATTable(self);
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

    uint16_t currentFATEntry = fat16File.fileClusterStart;
    uint32_t bytesLeftToRead = fat16File.fileSize;
    uint16_t readSize = self->info->FAT16.bytesPerSector;
    do{
        uint8_t sectorIndex = 0;
        while(bytesLeftToRead > 0 && sectorIndex < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToRead < self->info->FAT16.bytesPerSector){
                readSize = bytesLeftToRead; // Prevent unwanted data being read
            }
            void* chunk = readClusterSector(self, currentFATEntry, sectorIndex); //TODO directly write from volume
            memcpy(file + dataPointer, chunk, readSize);
            free(chunk);
            sectorIndex++;
            dataPointer += readSize;
            bytesLeftToRead -= readSize;
        }
        currentFATEntry = readFATS(self, currentFATEntry - self->info->FAT16.dataSectionStart);
    } while(currentFATEntry != swapEndianness16Bit(0xFFF8));
    destroyPath(path);
    return file;
}

void* FAT16ReadFileSection(FormattedVolume* self, Path path, uint32_t offset, uint32_t chunkSize){
    //TODO
    return NULL;

    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    if(checkNamingCollusion(self, entryTable, name, false) == FS_FILE_NOT_FOUND){
        return NULL;
    }
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    char* fileSection = malloc(fat16File.fileSize);

    uint32_t dataPointer = offset;
    uint32_t bytesInSector = self->info->FAT16.bytesPerSector;

    uint16_t currentFATEntry = fat16File.fileClusterStart;
    uint32_t bytesLeftToRead = chunkSize;
    uint16_t readSize = bytesInSector;
    do{
        uint8_t sectorIndex = 0;
        while(bytesLeftToRead > 0 && sectorIndex < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToRead < bytesInSector){
                readSize = bytesLeftToRead; // Prevent unwanted data being read
            }
            void* chunk = readClusterSector(self, currentFATEntry, sectorIndex); //TODO directly write from volume
            memcpy(fileSection + dataPointer, chunk, readSize);
            free(chunk);
            sectorIndex++;
            dataPointer += readSize;
            bytesLeftToRead -= readSize;
        }
        currentFATEntry = readFATS(self, currentFATEntry);
    } while(currentFATEntry != swapEndianness16Bit(0xFFF8));
    destroyPath(path);
    return fileSection;
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

    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, false);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printFATTable(self);
    printTree(self);
    #endif
    destroyPath(path);
    return statusCode;
}
FS_STATUS_CODE FAT16DeleteDir(FormattedVolume *self, Path path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path.path[path.depth];
    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, true);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printFATTable(self);
    printTree(self);
    #endif
    destroyPath(path);
    return statusCode;
}
char* FAT16ToTreeString(FormattedVolume* self, Path path){
    return printTreeToString(self);
}

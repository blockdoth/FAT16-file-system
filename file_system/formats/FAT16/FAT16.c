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
    formattedVolume->isDir = FAT16IsDir;
    formattedVolume->toString = FAT16ToTreeString;
    formattedVolume->destroy = FAT16Destroy;
    formattedVolume->getMetadata = FAT16GetMetadata;
    formattedVolume->renameFile = FAT16Rename;
    initCache(formattedVolume, MAX_PAGES_IN_CACHE);
    return formattedVolume;
}
FormattedVolume *formatFAT16Volume(RawVolume *volume, FAT16Config fat16Config) {
    if(!checkFAT16Compatible(volume)) return NULL;

    BootSector bootSector = initBootSector(volume->volumeSize, fat16Config);
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

FS_STATUS_CODE FAT16WriteFile(FormattedVolume * self, Path* path, file_metadata* fileMetadata, void* fileData){
    sector_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, false) == true) return FS_FILE_ALREADY_EXISTS;
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    sector_ptr startCluster = findFreeClusterInFAT(self);
    writeAlignedSectors(self, fileData, fat16File.fileSize, 0, startCluster);

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
    return FS_SUCCES;
}

uint32_t FAT16UpdateFile(FormattedVolume* self, Path* path, void* newData, uint32_t updatedDataSize, uint32_t offset){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false) return -1;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    uint32_t bytesLeftToWrite = updatedDataSize;
    uint32_t currentDataPointer = 0;

    uint32_t offsetInSector = offset % self->info->FAT16.bytesPerSector;
    uint32_t offsetInCluster = (offset / self->info->FAT16.bytesPerSector) % self->info->FAT16.sectorsPerCluster;
    uint32_t currentCluster = fat16File.fileClusterStart + (offset / self->info->FAT16.bytesPerCluster);
    uint8_t sectorInCluster = (offset % self->info->FAT16.bytesPerCluster) / self->info->FAT16.bytesPerSector;
    // Update the last potentially halfwritten sector
    uint32_t writeSize = self->info->FAT16.bytesPerSector - offsetInSector;
    if(writeSize > updatedDataSize){
        writeSize = updatedDataSize;
    }
    updateClusterSector(self, currentCluster, offsetInCluster, newData, writeSize, offsetInSector);
    bytesLeftToWrite -= writeSize;
    currentDataPointer += writeSize;
    sectorInCluster++;

    // Update all the data that overlaps
    while(offset + currentDataPointer < fat16File.fileSize && bytesLeftToWrite > 0){
        while(sectorInCluster < self->info->FAT16.sectorsPerCluster && bytesLeftToWrite > 0){
            if(bytesLeftToWrite < self->info->FAT16.bytesPerSector){
                //Prevent overwritten old data
                writeSize = bytesLeftToWrite;
                updateClusterSector(self, currentCluster, sectorInCluster,newData + currentDataPointer, writeSize,0);
            } else{
                writeClusterSector(self, currentCluster, sectorInCluster,newData + currentDataPointer, writeSize);
            }
            bytesLeftToWrite -= writeSize;
            currentDataPointer += writeSize;
            sectorInCluster++;
        }
        sectorInCluster = 0;
        currentCluster = readFATS(self, currentCluster - self->info->FAT16.dataSectionStart);
    }

    // Write new data like normal
    if(bytesLeftToWrite > 0){
        // Updates the tail entry of the FAT linked list to point to the next entry
        uint32_t index = findSecondToLastCluster(self, fat16File.fileClusterStart) - self->info->FAT16.dataSectionStart;
        currentCluster = findFreeClusterInFAT(self);
        writeFATS(self,index, currentCluster);
        writeAlignedSectors(self, newData, bytesLeftToWrite, currentDataPointer, currentCluster);
    }

    // Write to the metadata to the right table
    fat16File.fileSize = offset + updatedDataSize;
    updateFileEntry(self, fat16File, entryTable);

#ifdef DEBUG_FAT16
    printf("Updated %s, new size %u bytes from newData sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, nextFreeCluster, currentCluster);
    printFATTable(self);
    printTree(self);
//    printRootSectorShort(self);
#endif
    return fat16File.fileSize;
}

uint32_t FAT16ExpandFile(FormattedVolume* self, Path* path, void* newData, uint32_t newDataSize){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    if(checkNamingCollusion(self, entryTable, name, false) == false) return -1;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);

    // Important offsets and such
    uint32_t offsetInSector = fat16File.fileSize % self->info->FAT16.bytesPerSector;
    uint32_t offsetInCluster = (fat16File.fileSize / self->info->FAT16.bytesPerSector) % self->info->FAT16.sectorsPerCluster;
    sector_ptr currentCluster = fat16File.fileClusterStart + (fat16File.fileSize / self->info->FAT16.bytesPerCluster);
    uint8_t sectorInCluster = (fat16File.fileSize % self->info->FAT16.bytesPerCluster) / self->info->FAT16.bytesPerSector;

    uint32_t bytesLeftToWrite = newDataSize;
    uint32_t currentDataPointer = 0;
    uint32_t writeSize;

    // Fills up the last half written sector
    if(offsetInSector > 0){
        writeSize = self->info->FAT16.bytesPerSector - offsetInSector;
        if(writeSize > newDataSize){
            writeSize = newDataSize;
        }
        updateClusterSector(self, currentCluster, offsetInCluster, newData, writeSize, offsetInSector);
        bytesLeftToWrite -= writeSize;
        currentDataPointer += writeSize;
        sectorInCluster++;
    }

    //Fill up the empty part of the half written cluster
    writeSize = self->info->FAT16.bytesPerSector;
    while(sectorInCluster != 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster && bytesLeftToWrite > 0){
        if(bytesLeftToWrite < self->info->FAT16.bytesPerSector){
            writeSize = bytesLeftToWrite; // Prevent unwanted data being written
        }
        writeClusterSector(self, currentCluster, sectorInCluster,newData + currentDataPointer, writeSize);
        bytesLeftToWrite -= writeSize;
        currentDataPointer += writeSize;
        sectorInCluster++;
    }

    // Writes new sectors in new clusters
    if(bytesLeftToWrite > 0){
        // Updates the tail entry of the FAT linked list to point to the next entry
        writeFATS(self,findSecondToLastCluster(self, fat16File.fileClusterStart) - self->info->FAT16.dataSectionStart,
                  findFreeClusterInFAT(self));
        writeAlignedSectors(self, newData, bytesLeftToWrite, currentDataPointer, findFreeClusterInFAT(self));
    }

    // Write the metadata to the right table
    fat16File.fileSize += newDataSize;
    updateFileEntry(self, fat16File, entryTable);

    #ifdef DEBUG_FAT16
    printf("Updated %s, new size %u bytes from newData sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, nextFreeCluster, currentCluster);
    printFATTable(self);
    printTree(self);
//    printRootSectorShort(self);
    #endif
    return fat16File.fileSize;
}


FS_STATUS_CODE FAT16WriteDir(FormattedVolume* self, Path* path, file_metadata* fileMetadata){
    sector_ptr entryTable = resolveFileTable(self, path);
    if(checkNamingCollusion(self, entryTable, fileMetadata->name, true) == true) return FS_DIRECTORY_ALREADY_EXISTS;
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata); // Consumes fileMetadata

    // Finds a place for the subdir metadata table
    sector_ptr subDirEntryCluster = findFreeClusterInFAT(self);
    writeFATS(self, subDirEntryCluster - self->info->FAT16.dataSectionStart, FAT16_EOF);

    // Write the metadata to the right table
    fat16File.fileClusterStart = subDirEntryCluster ;
    writeFileEntry(self, fat16File, entryTable); //TODO support multi cluster DIRS

    #ifdef DEBUG_FAT16
    printf("Created a directory %s at sector %u\n", fat16File.name, entryTable);
//    printFATTable(self);
    printTree(self);
    printRootSectorShort(self);
    #endif
    return FS_SUCCES;
}


void* FAT16ReadFile(FormattedVolume* self, Path* path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    if(checkNamingCollusion(self, entryTable, name, false) == FS_FILE_NOT_FOUND) return NULL;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);

    char* file = malloc(fat16File.fileSize);
    uint32_t dataPointer = 0;
    uint16_t currentCluster = fat16File.fileClusterStart;
    uint32_t bytesLeftToRead = fat16File.fileSize;
    uint16_t readSize = self->info->FAT16.bytesPerSector;

    do{ // Do while loop my beloved
        uint8_t sectorInCluster = 0;
        while(bytesLeftToRead > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToRead < self->info->FAT16.bytesPerSector){
                readSize = bytesLeftToRead; // Prevent unwanted data being read
            }
            readClusterSector(self, currentCluster, sectorInCluster, file + dataPointer, readSize);
            sectorInCluster++;
            dataPointer += readSize;
            bytesLeftToRead -= readSize;
        }
        currentCluster = readFATS(self, currentCluster - self->info->FAT16.dataSectionStart);
    } while(currentCluster != FAT16_EOF);
    return file;
}

void* FAT16ReadFileSection(FormattedVolume* self, Path* path, uint32_t offset, uint32_t chunkSize){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    if(checkNamingCollusion(self, entryTable, name, false) == FS_FILE_NOT_FOUND)return NULL;
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    if((offset + chunkSize)> fat16File.fileSize) return NULL; // Out of bounds TODO error handling

    char* file = malloc(chunkSize);

    uint32_t dataPointer = 0;
    uint32_t offsetInSector = offset % self->info->FAT16.bytesPerSector;
    uint32_t currentCluster = fat16File.fileClusterStart + (offset / self->info->FAT16.bytesPerCluster);
    uint8_t sectorInCluster = (offset % self->info->FAT16.bytesPerCluster) / self->info->FAT16.bytesPerSector;
    uint32_t bytesLeftToRead = chunkSize;
    uint16_t readSize = self->info->FAT16.bytesPerSector;

    // Read the first sector with an offset
    // Kinda unelegant but I couldnt make it work otherwise, without changing a lot elsewhere
    // tech debt from when read function returned copies instead of writing to a buffer
    if(bytesLeftToRead < self->info->FAT16.bytesPerSector){
        readSize = bytesLeftToRead; // Prevent unwanted data being read
    }
    Sector chunk = malloc(self->info->FAT16.bytesPerSector);
    readClusterSector(self, currentCluster, sectorInCluster, chunk, self->info->FAT16.bytesPerSector);
    memcpy(file + dataPointer, chunk + offsetInSector, readSize);
    free(chunk);
    sectorInCluster++;
    dataPointer += readSize;
    bytesLeftToRead -= readSize;

    do{
        while(bytesLeftToRead > 0 && sectorInCluster < self->info->FAT16.sectorsPerCluster){
            if(bytesLeftToRead < self->info->FAT16.bytesPerSector){
                readSize = bytesLeftToRead; // Prevent unwanted data being read
            }
            readClusterSector(self, currentCluster, sectorInCluster, file + dataPointer, readSize);
            sectorInCluster++;
            dataPointer += readSize;
            bytesLeftToRead -= readSize;
        }
        sectorInCluster = 0;
        currentCluster = readFATS(self, currentCluster - self->info->FAT16.dataSectionStart);
    } while(currentCluster != FAT16_EOF);
    return file;
}

FS_STATUS_CODE FAT16CheckFile(FormattedVolume* self, Path* path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    return checkNamingCollusion(self, entryTable, name, false);
}
FS_STATUS_CODE FAT16CheckDir(FormattedVolume* self, Path* path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    return checkNamingCollusion(self, entryTable, name, true);
}
FS_STATUS_CODE FAT16DeleteFile(FormattedVolume* self, Path* path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];

    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, false);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printFATTable(self);
    printTree(self);
    #endif
    return statusCode;
}
FS_STATUS_CODE FAT16DeleteDir(FormattedVolume *self, Path* path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    FS_STATUS_CODE statusCode = deleteEntry(self, entryTable, name, true);
    #ifdef DEBUG_FAT16
    printRootSectorShort(self);
    printFATTable(self);
    printTree(self);
    #endif
    return statusCode;
}

FS_STATUS_CODE FAT16IsDir(FormattedVolume *self, Path* path) {
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    FAT16File fileMetadata = findEntryInTable(self, entryTable, name);
    if(fileMetadata.attributes && ATTR_DIRECTORY){
        return FS_SUCCES;
    }
    return FS_IS_FILE;
}
char* FAT16ToTreeString(FormattedVolume* self){
    return printTreeToString(self);
}

file_metadata* FAT16GetMetadata(FormattedVolume *self, Path* path){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    return convertFAT16FileToMetadata(fat16File);

}
FS_STATUS_CODE FAT16Rename(FormattedVolume *self, Path* path, char* newName){
    sector_ptr entryTable = resolveFileTable(self, path);
    char* name = path->path[path->depth];
    FAT16File fat16File = findEntryInTable(self, entryTable, name);
    strncpy(fat16File.name, newName, 11);
    renameFileEntry(self, fat16File, entryTable, name);
    return FS_SUCCES;
}


FS_STATUS_CODE FAT16Destroy(FormattedVolume *self) {
    self->rawVolume->destroy(self->rawVolume);
    destroyCache(self);
    return FS_SUCCES;
}

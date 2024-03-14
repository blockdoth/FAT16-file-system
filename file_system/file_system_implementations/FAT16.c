#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "FAT16.h"
#include "FAT16_debug.h"

#define DEBUG



FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(check_FAT16_formattible(volume)){
        return (void*)0;
    }
    printf("Formatting a rawVolume of size %u bytes\n", volume->volumeSize);

    BootSector bootSector = initBootSector(volume->volumeSize);
    FATVolumeInfo* volumeInfo = initFATVolumeInfo(bootSector);

    // Write the bootsector to the rawVolume
    volume->write(volume, &bootSector,0, sizeof(bootSector));
    #ifdef DEBUG
    printBootSector((BootSector*) volume->read(volume, 0, sizeof(bootSector)));
    #endif

    //Zero out FAT tables (both of them)
    volume->write(volume, NULL, volumeInfo->FAT1Start, 2 * volumeInfo->FATEntryCount);
    //Zero out root directory
    volume->write(volume, NULL, volumeInfo->rootSectionStart, volumeInfo->rootSectorCount);

    uint16_t firstSectors[] = {
            swapEndianness16Bit(0xFFF8),
            swapEndianness16Bit(0xFFFF)
    };
    // Set the first cluster to 0xFFF8, indicating the first cluster of a file
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian
    volume->write(volume, &firstSectors, volumeInfo->FAT1Start, 4);
    volume->write(volume, &firstSectors, volumeInfo->FAT2Start, 4);

    FormattedVolume* formattedVolume = initFormattedVolume(volume, volumeInfo);
    #ifdef DEBUG
    printFATTable(formattedVolume);

    printFAT16Layout(formattedVolume);
    #endif
    return formattedVolume; //TODO make this work once I figured out what to do with formattedVolume
}



bool FAT16WriteFile(FormattedVolume * self, FileMetadata* fileMetadata, void* fileData){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);
    volume_ptr startSector = FATFindNextFreeSector(self);
    if(startSector == 0){
        return false;
    }


    volume_ptr currentSector = startSector;
    volume_ptr prevSector;
    volume_ptr currentDataPointer = 0;
    uint32_t bytesPerCluster = self->volumeInfo->bytesPerSector * self->volumeInfo->sectorsPerCluster;
    uint32_t bytesLeftToWrite = fat16File.fileSize;
    while (bytesLeftToWrite > bytesPerCluster){
        // Write the data
        writeSector(self,
                    fileData + currentDataPointer,
                    currentSector, //TODO make this not hardcoded
                    bytesPerCluster);
        prevSector = currentSector;

        // Write the address of the next sector to the FATS
        writeFATS(self,prevSector, &currentSector);
        currentSector = FATFindNextFreeSector(self);

        currentDataPointer += bytesPerCluster;
        bytesLeftToWrite -= bytesPerCluster;
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
    writeMetaData(self, fat16File, self->volumeInfo->rootSectionStart,startSector, currentSector);
    fat16File.fileClusterEnd = startSector;
    fat16File.fileClusterEnd = currentSector;

    #ifdef DEBUG
    printf("Wrote %s of size %u bytes from sector %u to sector %u\n",
           fat16File.name, fat16File.fileSize, startSector, currentSector);
    printFATTable(self);
    printRootSectorShort(self);
    #endif



    return true;
}

bool FAT16WriteDir(FormattedVolume* self, FileMetadata* fileMetadata, char* path){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);

    Path resolvedPath = parsePath(path);
    volume_ptr entryTable = self->volumeInfo->rootSectionStart;
    FAT16File entry;
    uint8_t pathLength = resolvedPath.depth;
    while (pathLength-- > 0){
        entry = findFileEntry(self, *resolvedPath.path, entryTable);
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
//    volume_ptr newEntryTable; // Table where the new dir entry is going to live
//    if(resolvedPath.depth == 0 ){
//        newEntryTable = entryTable; // Root section;
//    }else{
//        //Finds the next free sector in the table in the data section
//        newEntryTable = entry.fileClusterStart;
//    }
    // DATAFindNextFreeSector(self, entryTable);
//    if(newEntryTable == 0){
//        return false;
//    }
    volume_ptr subDirEntryCluster = FATFindNextFreeSector(self);

    uint16_t endSector = swapEndianness16Bit(0xFFFF);
    writeFATS(self, subDirEntryCluster - self->volumeInfo->dataSectionStart, &endSector);

    writeMetaData(self, fat16File, entryTable, subDirEntryCluster , subDirEntryCluster); //TODO support multi cluster DIRS


    destroyPath(resolvedPath);
#ifdef DEBUG
    printf("Created a directory %s at sector %u\n",
           fat16File.name, entryTable);
    //printFATTable(self);
    printTree(self);
    printRootSectorShort(self);
#endif
    return true;
}


FAT16File findFileEntry(FormattedVolume* self, char* fileName, volume_ptr startCluster){
    FAT16File entry;
    for (volume_ptr i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = *(FAT16File*) self->rawVolume->read(self->rawVolume, startCluster + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
        if(entry.name[0] != 0x00){
            if(strcmp(entry.name, fileName) == 0){
                // TODO support the actual FAT16 naming format and longer filenames
                #ifdef DEBUG
                printf("Found %s of size %u in sector %u to sector %u\n",
                       entry.name, entry.fileSize, entry.fileClusterStart, entry.fileClusterEnd);
                #endif
                return entry;
            }
        }
    }

    return (FAT16File){
            .fileClusterStart = 0
    }; //TODO error handling and make this not hacky
}


Path parsePath(char* path){
    path++; //skip root
    if(*path == '\0'){
        char** resolvedPath = (char**) malloc((1) * sizeof(char **));
        *resolvedPath = "";
        return (Path) {resolvedPath, 0};
    }

    char* tempPath = path;
    uint16_t depth = 0;
    while(*tempPath++ != '\0'){
        if(*tempPath == '|'){
            depth++;
        }
        tempPath++;
    }
    char** resolvedPath = (char**) malloc((depth + 1) * sizeof(char **));

    int i = 0;
    tempPath = path;
    char* start = path;
    // Parsing the path and extracting substrings separated by '|'
    while (*tempPath != '\0') {
        if (*tempPath == '|') {
            uint32_t strlen = tempPath - start;
            resolvedPath[i] = (char*)malloc((strlen + 1) * sizeof(char));
            strncpy(resolvedPath[i], start, strlen);
            resolvedPath[i][strlen] = '\0'; // Null-terminate the string
            start = tempPath + 1;
            i++;
        }
        tempPath++;
    }
    return (Path){
        resolvedPath,
        depth
    };
}

void destroyPath(Path path){
//    while (*path.path != NULL) {
//        free(*path.path);
//        path.path++;
//    }
    //free(path); //Stack allocated
}



void writeSector(FormattedVolume* self, void* data, volume_ptr sector, uint32_t dataSize){
    self->rawVolume->write(self->rawVolume, data, self->volumeInfo->dataSectionStart + sector * self->volumeInfo->bytesPerSector, dataSize);
}

void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector){
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT1Start + 2 * index, 2);
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT2Start + 2 * index, 2);
}

void writeMetaData(FormattedVolume* self, FAT16File fileMetadata,  volume_ptr tableStart, volume_ptr startSector, volume_ptr endSector){
    fileMetadata.fileClusterStart = startSector;
    fileMetadata.fileClusterEnd = endSector;
    FAT16File entry;
    int32_t i;
    for(i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = *(FAT16File *) self->rawVolume->read(self->rawVolume, tableStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
        if(entry.name[0] == 0x00 || entry.name[0] == 0xe5){
            break;
        }
    }
    self->rawVolume->write(self->rawVolume, &fileMetadata, tableStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
    // TODO this is probably wrong
}




void* FAT16Read(FormattedVolume* self,  FileMetadata* fileMetadata) {
    FAT16File fat16File = findFileEntry(self, fileMetadata->name, self->volumeInfo->rootSectionStart);
    if(fat16File.fileClusterStart == 0){
        #ifdef DEBUG
        printf("ERROR: Couldnt find file with name %s\n", fileMetadata->name);
        #endif
        return NULL;
    }
    char* readFile = malloc(fileMetadata->fileSize);
    void* startOfFile = readFile;
    for (uint16_t i = fat16File.fileClusterStart; i <= fat16File.fileClusterEnd; i++) {
        memcpy(readFile, readSector(self,i), self->volumeInfo->bytesPerSector);
        readFile += self->volumeInfo->bytesPerSector;
    }
    return startOfFile; // TODO Add more metadata
}

void* readSector(FormattedVolume* self, volume_ptr sector){
    return self->rawVolume->read(self->rawVolume, self->volumeInfo->dataSectionStart + sector * self->volumeInfo->bytesPerSector , self->volumeInfo->bytesPerSector);
}

FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata){
    FAT16File fat16File;

    memcpy(fat16File.name, fileMetadata->name, FAT16_ENTRY_BASE_NAME_LENGTH); // TODO support long file names
    fat16File.name[10] = '\0';

    fat16File.attributes = convertToDirAttributes(fileMetadata); //TODO writeFile converter
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

//TODO merge these functions

volume_ptr FATFindNextFreeSector(FormattedVolume* self){
    uint16_t entry;
    for (uint32_t i = 0; i < self->volumeInfo->FATEntryCount; i++) {
        entry = *(uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT1Start + i * 2, FAT16_ENTRY_SIZE);
        if(entry == 0x00){
            return self->volumeInfo->dataSectionStart + i;
        }
    }
    return 0;
}

volume_ptr DATAFindNextFreeSector(FormattedVolume* self, volume_ptr tableAddress){
    uint16_t entry;
    for (uint32_t i = 0; i < self->volumeInfo->entriesPerCluster; i++) {
        entry = *(uint16_t*) self->rawVolume->read(self->rawVolume, tableAddress + i * 2, FAT16_ENTRY_SIZE);
        if(entry == 0x00){
            return tableAddress + i;
        }
    }
    return 0;
}


// Debug statements

// Init structs
FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo *volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->volumeInfo = volumeInfo;
    formattedVolume->read = FAT16Read;
    formattedVolume->writeFile = FAT16WriteFile;
    formattedVolume->writeDir = FAT16WriteDir;
    return formattedVolume;
}


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
    volumeInfo->FATEntryCount = bootSector.totalSectorCount16;;
    volumeInfo->totalAddressableSize = bootSector.totalSectorCount16 * bootSector.sectorsPerCluster * bootSector.bytesPerSector;
    volumeInfo->bytesPerSector = bootSector.bytesPerSector;
    volumeInfo->sectorsPerCluster = bootSector.sectorsPerCluster;
    volumeInfo->entriesPerCluster = (bootSector.sectorsPerCluster * bootSector.bytesPerSector) / 32;
    return volumeInfo;
}




bool check_FAT16_formattible(RawVolume *raw_volume) {
    return raw_volume->volumeSize < FAT16_MINIMUM_SIZE;
}

//-----------------------------------------------------------------------------
// directoryNameChecksum()
// Returns an unsigned byte checksum computed on an unsigned byte
// array. The array must be 11 bytes long and is assumed to contain
// a name stored in the format of a MS-DOS directory entry.
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

#undef DEBUG
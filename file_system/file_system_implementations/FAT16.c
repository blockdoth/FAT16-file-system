#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "FAT16.h"

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



bool FAT16Write(FormattedVolume * self, FileMetadata* fileMetadata, void* fileData){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);
    volume_ptr startSector = findNextFreeSector(self);
    if(startSector == -1){
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
        currentSector = findNextFreeSector(self);

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
    writeMetaData(self, fat16File, startSector, currentSector);
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

void writeSector(FormattedVolume* self, void* data, volume_ptr sector, uint32_t dataSize){
    self->rawVolume->write(self->rawVolume, data, self->volumeInfo->dataSectionStart + sector * self->volumeInfo->bytesPerSector, dataSize);
}

void writeFATS(FormattedVolume* self, volume_ptr index, void *nextSector){
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT1Start + 2 * index, 2);
    self->rawVolume->write(self->rawVolume, nextSector, self->volumeInfo->FAT2Start + 2 * index, 2);
}

void writeMetaData(FormattedVolume* self, FAT16File fileMetadata, volume_ptr startSector, volume_ptr endSector){
    fileMetadata.fileClusterStart = startSector;
    fileMetadata.fileClusterEnd = endSector;
    FAT16File entry;
    int32_t i;
    for(i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = *(FAT16File *) self->rawVolume->read(self->rawVolume, self->volumeInfo->rootSectionStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
        if(entry.name[0] == 0x00 || entry.name[0] == 0xe5){
            break;
        }
    }
    self->rawVolume->write(self->rawVolume, &fileMetadata, self->volumeInfo->rootSectionStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
    // TODO this is probably wrong
}


FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata){
    FAT16File fat16File;

    memcpy(fat16File.name, fileMetadata->name, FAT16_ENTRY_BASE_NAME_LENGTH); // TODO support long file names
    fat16File.name[10] = '\0';

    fat16File.attributes = convertToDirAttributes(fileMetadata); //TODO write converter
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

FAT16File FAT16FindFile(FormattedVolume* self, char* fileName){
    FAT16File entry;
    for (volume_ptr i = 0; i < self->volumeInfo->rootSectorCount; i++) {
        entry = *(FAT16File*) self->rawVolume->read(self->rawVolume, self->volumeInfo->rootSectionStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
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
        .fileSize = -1
    }; //TODO error handling and make this not hacky
}

void* FAT16Read(FormattedVolume* self,  FileMetadata* fileMetadata) {
    FAT16File fat16File = FAT16FindFile(self, fileMetadata->name);
    if(fat16File.fileSize == -1){
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


uint8_t convertToDirAttributes(FileMetadata* file) {
    uint8_t dirAttributes = 0;
    dirAttributes |= file->read_only ? DIR_ATTR_READONLY : 0;
    dirAttributes |= file->hidden ? DIR_ATTR_HIDDEN : 0;
    dirAttributes |= file->system ? DIR_ATTR_SYSTEM : 0;
    dirAttributes |= file->volume_id ? DIR_ATTR_VOLUME_ID : 0;
    dirAttributes |= file->directory ? DIR_ATTR_DIRECTORY : 0;
    dirAttributes |= file->archive ? DIR_ATTR_ARCHIVE : 0;
    // Assuming DIR_ATTR_LONGNAME is a combination of all bits in long_name
    dirAttributes |= file->long_name ? DIR_ATTR_LONGNAME : 0;

    return dirAttributes;
}


volume_ptr findNextFreeSector(FormattedVolume* volume){
    uint16_t entry;
    for (uint32_t i = 0; i < volume->volumeInfo->FATEntryCount; i++) {
        entry = *(uint16_t*) volume->rawVolume->read(volume->rawVolume, volume->volumeInfo->FAT1Start + i * 2, FAT16_ENTRY_SIZE);
        if(entry == 0x00){
            return i;
        }
    }
    return -1;
}


// Debug statements


void printRootSectorShort(FormattedVolume* self){
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ Root Sectors                            │\n");
    printf("├─────────────────────────────────────────┤\n");

    FAT16File entry;
    for(int32_t i = 0; i < self->volumeInfo->rootSectorCount; i++){
        entry = *(FAT16File *) self->rawVolume->read(self->rawVolume, self->volumeInfo->rootSectionStart + i * FAT16_ENTRY_SIZE, FAT16_ENTRY_SIZE);
        if(entry.name[0] == 0x00){
            break;
        }else if(entry.name[0] == 0xe5){
            printf("│ %u Deleted file\t%u - %u\t  │\n",i, entry.fileClusterStart, entry.fileClusterEnd);
        } else{
            printf("│ %u %s \t%u bytes\t%u - %u\t  │\n",i,entry.name, entry.fileSize, entry.fileClusterStart, entry.fileClusterEnd);
        }
    }
    printf("└─────────────────────────────────────────┘\n");


}


void printFAT16File(FAT16File *file) {
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ File Metadata                           │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ Name:\t\t\t  %.10s\t  │\n", file->name);
    printf("│ File Size:\t\t  %u\t\t  │\n", file->fileSize);
    printf("│ Attributes\t\t\t\t  │\n");
    for (int i = 7; i >= 0; i--) {
        uint8_t mask = 1 << i;
        uint8_t bit = (file->attributes & mask);
        switch (bit) {
            case DIR_ATTR_READONLY:
                printf("│   └ Read-Only\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_HIDDEN:
                printf("│   └ Hidden\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_SYSTEM:
                printf("│   └ System\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_VOLUME_ID:
                printf("│   └ Volume ID\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_DIRECTORY:
                printf("│   └ Directory\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_ARCHIVE:
                printf("│   └ Archive\t\t  X\t\t  │\n");
                break;
            case DIR_ATTR_LONGNAME:
                printf("│   └ Long Name\t\t  X\t\t  │\n");
                break;
            default:
        }
    }
    printf("│ Creation Date:\t  %u\t\t  │\n", file->creationDate);
    printf("│ Creation Time:\t  %u\t\t  │\n", file->creationTime);
    printf("│ Creation Time Tenth:\t  %u\t\t  │\n", file->creationTimeTenth);
    printf("│ Time of Last Write:\t  %u\t\t  │\n", file->timeOfLastWrite);
    printf("│ Date of Last Write:\t  %u\t\t  │\n", file->dateOfLastWrite);
    printf("│ Last Accessed Date:\t  %u\t\t  │\n", file->lastAccessedDate);
    printf("│ First Cluster Start:\t  %u\t\t  │\n", file->fileClusterStart);
    printf("│ First Cluster End:\t  %u\t\t  │\n", file->fileClusterEnd);
    printf("└─────────────────────────────────────────┘\n");
}

void printFAT16Layout(FormattedVolume *file) {
    FATVolumeInfo* volumeInfo = file->volumeInfo;
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ FAT 16 layout                           │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ Reserved                                │\n");
    printf("│  └ Bootsector\t\t0\t\t  │\n");
    printf("│ FAT'S                                   │\n");
    printf("│  └ FAT 1\t\t%u - %u\t\t  │\n", volumeInfo->FAT1Start, volumeInfo->FAT1Start + volumeInfo->FATTableSectorCount - 1);
    printf("│  └ FAT 2\t\t%u - %u\t\t  │\n", volumeInfo->FAT2Start, volumeInfo->FAT2Start + volumeInfo->FATTableSectorCount - 1);
    printf("│ Data Sector                             │\n");
    printf("│  └ Root Sector\t%u - %u\t\t  │\n", volumeInfo->rootSectionStart, volumeInfo->rootSectionStart + volumeInfo->rootSectorCount - 1);
    printf("│  └ Cluster Area\t%u - %u\t  │\n", volumeInfo->dataSectionStart, volumeInfo->totalSectorCount);
    printf("└─────────────────────────────────────────┘\n");
}


void printFATTable(FormattedVolume* self){
    printf("┌─────────────────────────┐\n");
    printf("│ FAT Table               │\n");
    printf("├─────────────────────────┤\n");
    uint32_t i = 0;
    uint16_t entry;

    bool searching = true;
    while(searching){
        entry = swapEndianness16Bit(*(uint16_t*) self->rawVolume->read(self->rawVolume, self->volumeInfo->FAT1Start + i * 2, 2));
        switch (entry) {
            case 0x0000:
                printf("│ %u \t Free Entry\t  │\n", i);
                searching = false;
                break;
            case 0xFFF7:
                printf("│ %u \t Bad Cluster\t │\n", i);
                break;
            case 0xFFF8:
                printf("│ %u \t Start of Cluster │\n", i);
                break;
            case 0xFFFF:
                printf("│ %u \t End of Cluster\t  │\n", i);
                break;
            default:
                if (entry >= 0x0001 && entry <= 0xFFEF) {
                    printf("│ %u \t 0x%02X\t\t  │\n", i, entry);
                } else if (entry >= 0xFFF0 && entry <= 0xFFF6) {
                    printf("│ %u \t Reserved Value │\n", i);
                }
                break;
        }
        i++;
    }
    printf("│ ⋮         ⋮             │\n");
    printf("│ %u  Total Entries    │\n", self->volumeInfo->FATEntryCount);
    printf("├─────────────────────────┤\n");
    printf("│ FAT Addressed: %luMB   │\n", self->volumeInfo->totalAddressableSize / 1048576);
    printf("└─────────────────────────┘\n");

}

void printBootSector(BootSector *bs) {
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ Boot Sector                             │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ jmpBoot:\t\t  {0x%02X,0x%02X,0x%02X}│\n", bs->jmpBoot[0], bs->jmpBoot[1], bs->jmpBoot[2]);
    printf("│ OEM_Name:\t\t  %.8s\t  │\n", bs->OEM_Name);
    printf("│ Bytes per Sector:\t  %u\t\t  │\n", bs->bytesPerSector);
    printf("│ Sectors per Cluster:\t  %u\t\t  │\n", bs->sectorsPerCluster);
    printf("│ Reserved Sector Count:  %u\t\t  │\n", bs->reservedSectorCount);
    printf("│ Number of FATs:\t  %u\t\t  │\n", bs->numberOfFATs);
    printf("│ Root Entry Count:\t  %u\t\t  │\n", bs->rootEntryCount);
    printf("│ Total Sector Count 16:  %u\t\t  │\n", bs->totalSectorCount16);
    printf("│ Media Type:\t\t  0x%02X\t\t  │\n", bs->media);
    printf("│ Sectors per FAT:\t  %u\t\t  │\n", bs->sectorsPersFAT);
    printf("│ Sectors per Track:\t  %u\t\t  │\n", bs->sectorsPerTrack);
    printf("│ Number of Heads:\t  %u\t\t  │\n", bs->numberOfHeads);
    printf("│ Hidden Sectors:\t  %u\t\t  │\n", bs->hiddenSectors);
    printf("│ Total Sector Count 32:  %u\t\t  │\n", bs->totalSectorCount32);
    printf("│ Drive Number:\t\t  %u\t\t  │\n", bs->driveNumber);
    printf("│ Reserved:\t\t  %u\t\t  │\n", bs->reserved);
    printf("│ Boot Signature:\t  0x%02X\t\t  │\n", bs->bootSignature);
    printf("│ Volume Serial Number:\t  %u\t\t  │\n", bs->volumeSerialNumber);
    printf("│ Volume Label:\t\t  %.11s\t  │\n", bs->volumeLabel);
    printf("│ Filesystem Type:\t  %.11s\t  │\n", bs->filesystemType);
    printf("└─────────────────────────────────────────┘\n");
}

// Init structs
FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo *volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->volumeInfo = volumeInfo;
    formattedVolume->read = FAT16Read;
    formattedVolume->write = FAT16Write;
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
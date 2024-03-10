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
    volume->write(volume, NULL,volumeInfo->FAT1Address, 2 * volumeInfo->FATClusters);
    //Zero out root directory
    volume->write(volume,NULL,volumeInfo->rootSectorAddress,volumeInfo->rootSectorCount);

    uint16_t firstSectors[] = {
            swapEndianness16Bit(0xFFF8),
            swapEndianness16Bit(0xFFFF)
    };
    // Set the first cluster to 0xFFF8, indicating the first cluster of a file
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian
    volume->write(volume, &firstSectors, volumeInfo->FAT1Address, 4);
    volume->write(volume, &firstSectors, volumeInfo->FAT2Address, 4);

    #ifdef DEBUG
    printFATTable(volumeInfo, volume);
    #endif

    FormattedVolume* formattedVolume = initFormattedVolume(volume, volumeInfo);

    return formattedVolume; //TODO make this work once I figured out what to do with formattedVolume
}

FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo *volumeInfo) {
    FormattedVolume* formattedVolume = (FormattedVolume*)malloc(sizeof(FormattedVolume));
    formattedVolume->rawVolume = volume;
    formattedVolume->volumeInfo = volumeInfo;
    formattedVolume->read = ((void*)0);
    formattedVolume->write = FAT16Write;
    return formattedVolume;
}





bool FAT16Write(FormattedVolume * self, FileMetadata* fileMetadata){
    FAT16File fat16File = convertMetadataToFAT16File(fileMetadata);
    volume_ptr nextFree = findNextFreeCluster(self);
    if(nextFree == -1){
        return false;
    }
    #ifdef DEBUG
    printFAT16File(&fat16File);
    printf("Found free cluster at %u\n", nextFree);
    #endif

    uint32_t bytesLeftToWrite = fat16File.fileSize;
    while (bytesLeftToWrite > self->volumeInfo->bytesPerSector){

    }
    //TODO handle left overs

    return true;
}


void printFAT16File(FAT16File *file) {
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ File Metadata                           │\n");
    printf("├─────────────────────────────────────────┤\n");
    printf("│ Name:\t\t\t  %.11s\t\t  │\n", file->name);
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
    printf("│ First Cluster Start:\t  %u\t\t  │\n", file->firstClusterStart);
    printf("│ First Cluster End:\t  %u\t\t  │\n", file->firstClusterEnd);
    printf("└─────────────────────────────────────────┘\n");
}

FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata){
    FAT16File fat16File;

    memcpy(fat16File.name, fileMetadata->name, 11);
    fat16File.name[10] = '\0';

    fat16File.attributes = convertToDirAttributes(fileMetadata); //TODO write converter
    fat16File.reserved = 0;
    fat16File.creationTimeTenth = fileMetadata->creationTimeTenth;
    fat16File.creationTime = fileMetadata->creationTime;
    fat16File.creationDate = fileMetadata->creationDate;
    fat16File.lastAccessedDate = fileMetadata->creationDate;
    fat16File.firstClusterStart = 0;
    fat16File.timeOfLastWrite = fileMetadata->creationTime;
    fat16File.dateOfLastWrite = fileMetadata->creationDate;
    fat16File.firstClusterEnd = 0; // TODO
    fat16File.fileSize = fileMetadata->fileSize;
    return fat16File;
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



volume_ptr findNextFreeCluster(FormattedVolume* volume){
    uint16_t entry;
    for (uint32_t i = 2; i < volume->volumeInfo->FATClusters; i++) {
        entry = *(uint16_t*) volume->rawVolume->read(volume->rawVolume, volume->volumeInfo->FAT1Address + i * 2, 2);
        if(entry == 0x0000){
            return i;
        }
    }
    return -1;
}

void printFATTable(FATVolumeInfo *volumeInfo, RawVolume* volume){
    printf("┌─────────────────────────┐\n");
    printf("│ FAT Table               │\n");
    printf("├─────────────────────────┤\n");
    uint32_t i = 0;
    uint16_t entry;

    bool searching = true;
    while(searching){
        entry = swapEndianness16Bit(*(uint16_t*) volume->read(volume, volumeInfo->FAT1Address + i * 2, 2));
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
    printf("│ %u  Total Clusters\t  │\n", volumeInfo->FATClusters);
    printf("├─────────────────────────┤\n");
    printf("│ FAT Addressed: %luMB   │\n", volumeInfo->totalAddressableSize / 1048576);
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
    // Idk why I need to divide by bootSector.sectorsPerCluster
    bootSector.totalSectorCount16 = volumeSize / bootSector.bytesPerSector / bootSector.sectorsPerCluster ;
    bootSector.sectorsPersFAT = bootSector.totalSectorCount16 - bootSector.reservedSectorCount;
    return bootSector;
}

FATVolumeInfo* initFATVolumeInfo(BootSector bootSector) {
    FATVolumeInfo* volumeInfo = (FATVolumeInfo*)malloc(sizeof(FATVolumeInfo));

    uint32_t FATTotalAddressedClusters = bootSector.totalSectorCount16 - bootSector.reservedSectorCount;
    volume_ptr rootSectorStart = bootSector.reservedSectorCount + FATTotalAddressedClusters * bootSector.numberOfFATs;
    uint32_t rootSectorCount = ((bootSector.rootEntryCount * 32)+(bootSector.bytesPerSector - 1)) /bootSector.bytesPerSector;

    volumeInfo->FAT1Address = bootSector.reservedSectorCount * bootSector.bytesPerSector;
    volumeInfo->FAT2Address = bootSector.reservedSectorCount * bootSector.bytesPerSector + FATTotalAddressedClusters;
    volumeInfo->FATClusters = FATTotalAddressedClusters;
    volumeInfo->rootSectorAddress = rootSectorStart;
    volumeInfo->rootSectorCount = rootSectorCount;
    volumeInfo->dataSectorAddress = rootSectorStart + rootSectorCount;
    volumeInfo->totalAddressableSize = FATTotalAddressedClusters * bootSector.sectorsPerCluster * bootSector.bytesPerSector;
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
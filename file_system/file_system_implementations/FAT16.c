#include <stdio.h>
#include "FAT16.h"
#include "../file_system.h"

#define DEBUG



FormattedVolume* formatFAT16Volume(RawVolume *volume) {
    if(check_FAT16_formattible(volume)){
        return (void*)0;
    }
    printf("Formatting a volume of size %u\n", volume->volumeSize);

    BootSector bootSector = initBootSectorStruct(volume->volumeSize);
    FATVolumeInfo volumeInfo = initFATVolumeInfo(bootSector);

    // Write the bootsector to the volume
    volume->write(volume, &bootSector,0, sizeof(bootSector));
    #ifdef DEBUG
    BootSector* returnedBootSector = (BootSector*) volume->read(volume, 0, sizeof(bootSector));
    printBootSector(returnedBootSector);
    #endif

    //Zero out FAT tables (both of them)
    volume->write(volume, NULL,volumeInfo.FAT1Address, 2 * volumeInfo.FATSize);
    //Zero out root directory
    volume->write(volume,NULL,volumeInfo.rootSectorAddress,volumeInfo.rootSectorCount);


    uint16_t firstSectors[] = {
            swapEndianness16Bit(0xFFF8),
            swapEndianness16Bit(0xFFFF)
    };
    // Set the first cluster to 0xFFF8, indicating the first cluster of a file
    // Set the second cluster to 0xFFFF, indicating the end of a cluster chain
    // IMPORTANT! Must be converted to little endian
    volume->write(volume, &firstSectors, volumeInfo.FAT1Address, 4);
    volume->write(volume, &firstSectors, volumeInfo.FAT2Address, 4);

    #ifdef DEBUG
    printFATTable(&volumeInfo, volume);
    #endif
    FormattedVolume formattedVolume = {
            volume,
            ((void*)0),
            ((void*)0)
    };
    return &formattedVolume; //TODO make this work once I figured out what to do with formattedVolume
}

FATVolumeInfo initFATVolumeInfo(BootSector bootSector) {
    uint32_t FATSize = bootSector.totalSectorCount16 - bootSector.reservedSectorCount;
    volume_ptr rootSectorStart = bootSector.reservedSectorCount + FATSize * bootSector.numberOfFATs;
    uint32_t rootSectorCount = ((bootSector.rootEntryCount * 32)+(bootSector.bytesPerSector - 1)) /bootSector.bytesPerSector;
    uint32_t totalAddressableSize = (FATSize * bootSector.sectorsPerCluster * bootSector.bytesPerSector);
    return (FATVolumeInfo) {
            bootSector.reservedSectorCount,
            bootSector.reservedSectorCount + FATSize,
            FATSize,
            rootSectorStart,
            rootSectorCount,
            rootSectorStart + rootSectorCount,
            totalAddressableSize
    };
}


volume_ptr findNextFreeCluster(RawVolume* volume){

}



void printFATTable(FATVolumeInfo *volumeInfo, RawVolume* volume){
    printf("┌─────────────────────────┐\n");
    printf("│ Entry  Cluster          │\n");
    printf("├─────────────────────────┤\n");
    uint32_t i = 0;
    uint16_t entry;

    bool searching = true;
    while(searching){
        entry = swapEndianness16Bit(
                *(uint16_t*) volume->read(volume, volumeInfo->FAT1Address + i * 2, 2)
                );
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


    printf("│ ...    ...              │\n");
    printf("│ %u  Total Clusters\t  │\n", volumeInfo->FATSize);
    printf("├─────────────────────────┤\n");
    printf("│ Total Addressable %uMB │\n", volumeInfo->totalAddressableSize / 8388608);
    printf("└─────────────────────────┘\n");

}

void printBootSector(BootSector *bs) {
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ Field\t\t\t  Value\t\t  │\n");
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



BootSector initBootSectorStruct(uint32_t volumeSize){
    BootSector bootSector = {
            {0xEB, 0x00, 0x90},
            "MSWIN4.1",
            4096,
            64,
            1,
            2,
            512,
            0, // See totalSectorCount32
            0xfa,
            0, //TODO
            0, // Not relevant
            0, // Not relevant
            0,
            0,//TODO Set this field
            0, // TODO
            1,
            0x29,
            0, //TODO Set this field
            "NO NAME    ",
            "FAT16      "
    };
    bootSector.totalSectorCount16 = 0xFFFF; //volume->volumeSize / bootSector.bytesPerSector;
    bootSector.sectorsPersFAT =
            (bootSector.totalSectorCount16 - bootSector.reservedSectorCount) / bootSector.sectorsPerCluster;
    return bootSector;
}

FAT16File prep_FAT16File_struct(){
    return (FAT16File){
            "",
            0,
            0,
            getCurrentTimeMs(),
            getCurrentTime(),
            getCurrentDate(),
            getCurrentDate(),
            0,
            getCurrentTime(),
            getCurrentDate(),
            0, //TODO
            0
    };
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

//uint32_t bootSectorFATAddress(BootSector *bootSector){
//    return bootSector->reservedSectorCount * bootSector->bytesPerSector;
//}
//
//uint32_t bootSectorRootAddress(BootSector *bootSector){
//    return bootSectorFATAddress(bootSector) +
//           bootSector->numberOfFATs * bootSector->sectorsPersFAT * bootSector->bytesPerSector;
//}
//
//uint32_t bootSectorDataAddress(BootSector *bootSector){
//    return bootSectorRootAddress(bootSector) + bootSector->rootEntryCount * 32;
//}
//
//uint32_t bootSectorDataSectorCount(BootSector *bootSector){
//    return bootSector->totalSectorCount32 - bootSectorDataAddress(bootSector) / bootSector->bytesPerSector;
//}



uint16_t getCurrentTimeMs(){
    return 0; //TODO
}

uint16_t getCurrentTime(){
    return 0; //TODO
}

uint16_t getCurrentDate(){
    return 0; //TODO
}

#undef DEBUG
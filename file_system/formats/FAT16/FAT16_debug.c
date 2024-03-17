#include "FAT16_debug.h"

void printRootSectorShort(FormattedVolume* self){
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ Root Sectors                            │\n");
    printf("├─────────────────────────────────────────┤\n");

    FAT16File entry;
    for(uint32_t i = 0; i < self->volumeInfo->rootSectorCount; i++){
        entry = readFileEntry(self, self->volumeInfo->rootSectionStart, i);
        if(entry.name[0] == 0x00){
            break;
        } else if(entry.name[0] == 0xe5){
            printf("│ %u Deleted rickRoll\t%u - %u\t  │\n",i, entry.fileClusterStart, entry.fileClusterEnd);
        } else if(entry.attributes == ATTR_DIRECTORY){
            printf("│ %u %s \tDirectory\t%u\t  │\n",i,entry.name, entry.fileClusterStart);
        } else{
            printf("│ %u %s \t%u bytes\t%u - %u\t  │\n",i,entry.name, entry.fileSize, entry.fileClusterStart, entry.fileClusterEnd);
        }
    }
    printf("└─────────────────────────────────────────┘\n");
}

void printTree(FormattedVolume* self){
    printf("─────────────────────────────────────────\n");
    printf("Directory structure\n");
    printf("─────────────────────────────────────────\n");
    printf("Root\n");
    printTreeRecursive(self, self->volumeInfo->rootSectionStart, "");
    printf("─────────────────────────────────────────\n");

}

void printTreeRecursive(FormattedVolume* self, volume_ptr tableStart, char* prefix){
    FAT16File entry;
    FAT16File nextEntry;
    bool lastEntry;
    for(uint32_t i = 0; i < self->volumeInfo->bytesPerCluster / FAT16_ENTRY_SIZE; i++) {
        entry = readFileEntry(self, tableStart, i);
        nextEntry = readFileEntry(self, tableStart, i + 1);
        if(nextEntry.name[0] == 0x00){
            lastEntry = true;
        }else{
            lastEntry = false;
        }
        if(entry.name[0] == 0x00){
            break;
        }
        char* pipe;
        char* pipePrefix;
        if(lastEntry){
            pipe = " └─ ";
            pipePrefix = "   ";
        }else{
            pipe = " ├─ ";
            pipePrefix = " │ ";
        }
        char* childPrefix = (char*)malloc( strlen(prefix) + strlen(pipePrefix) + 1); // Make a copy for concat's
        sprintf(childPrefix, "%s%s", prefix, pipePrefix);

        printf("%s%s%s \n",prefix,pipe,entry.name);

        if(entry.attributes == ATTR_DIRECTORY){
            printTreeRecursive(self, entry.fileClusterStart, childPrefix);
        }
        free(childPrefix);
    }
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
            case ATTR_READONLY:
                printf("│   └ Read-Only\t\t  X\t\t  │\n");
                break;
            case ATTR_HIDDEN:
                printf("│   └ Hidden\t\t  X\t\t  │\n");
                break;
            case ATTR_SYSTEM:
                printf("│   └ System\t\t  X\t\t  │\n");
                break;
            case ATTR_VOLUME_ID:
                printf("│   └ Volume ID\t\t  X\t\t  │\n");
                break;
            case ATTR_DIRECTORY:
                printf("│   └ Directory\t\t  X\t\t  │\n");
                break;
            case ATTR_ARCHIVE:
                printf("│   └ Archive\t\t  X\t\t  │\n");
                break;
            case ATTR_LONGNAME:
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
        entry = swapEndianness16Bit(readFATS(self, i));
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

void printBootSector(BootSector* bs) {
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

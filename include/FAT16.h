#ifndef FILE_SYSTEM_FAT16_H
#define FILE_SYSTEM_FAT16_H

#ifndef STDINT_H
#include "stdint.h"
#endif

struct BPB_t {
    uint8_t jmpBoot[3];
    unsigned char OEM_Name[8];
    uint16_t bytesPerSector : 2;
    uint8_t sectorsPerCluster : 1;
    uint16_t reservedSectorCount : 2;
    uint8_t numberOfFATs : 1;
    uint16_t rootEntryCount : 2;
    uint16_t totalSectorCount16 : 2;
    uint8_t media : 1;
    uint16_t sectorsPersFAT : 2;
    uint16_t sectorsPerTrack : 2;
    uint16_t numberOfHeads : 2;
    uint32_t hiddenSectors : 4;
    uint32_t totalSectorCount : 4;

};


#endif //FILE_SYSTEM_FAT16_H

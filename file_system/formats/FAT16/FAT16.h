#ifndef FILE_SYSTEM_FAT16_H

#define FILE_SYSTEM_FAT16_H
#include "../../file_system_api/file_system.h"
#include "../../volume/volume.h"

#include "stdint.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

//#define DEBUG_FAT16

#define FAT16_MINIMUM_SIZE 1024
#define FAT16_ENTRY_SIZE 32
#define FAT16_ENTRY_BASE_NAME_LENGTH 11

#define MAX_PAGES_IN_CACHE 1000

#define FAT16_EOF 0xF8FF

// Based on BigHards spec
typedef struct {
    uint8_t jmpBoot[3];
    unsigned char OEM_Name[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectorCount;
    uint8_t numberOfFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectorCount16;
    uint8_t media;
    uint16_t sectorsPersFAT;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectorCount32;
    uint8_t driveNumber;
    uint8_t reserved;
    uint8_t bootSignature;
    uint32_t volumeSerialNumber;
    unsigned char volumeLabel[11];
    unsigned char filesystemType[11];
} BootSector; // Total volumeSize: 90 bytes

// Based on MediumStiffs spec
typedef struct {
    unsigned char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creationTimeTenth;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t FAT32fileClusterStart;
    uint16_t timeOfLastWrite;
    uint16_t dateOfLastWrite;
    uint16_t fileClusterStart;
    uint32_t fileSize;
} FAT16File; // Total volumeSize: 32 bytes


// Based on MicroSofts spec
typedef struct{
    uint8_t order;
    unsigned char name1[10];
    uint8_t attributes;
    uint8_t type;
    uint8_t checkSum;
    unsigned char name2[12];
    uint16_t firstClusterNumberLowWord;
    unsigned char name3[4];
} FAT16FileNameExtension; // Total volumeSize: 32 bytes

enum DirectoryAttributes {
    ATTR_READONLY  = 1 << 0,
    ATTR_HIDDEN    = 1 << 1,
    ATTR_SYSTEM    = 1 << 2,
    ATTR_VOLUME_ID = 1 << 3,
    ATTR_DIRECTORY = 1 << 4,
    ATTR_ARCHIVE   = 1 << 5,
    ATTR_LONGNAME  = 0xF
};

// ^ Based on "Microsoft Extensible Firmware Initiative FAT32 File System Specification"

#endif //FILE_SYSTEM_FAT16_H

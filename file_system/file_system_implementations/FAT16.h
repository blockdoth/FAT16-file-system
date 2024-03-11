#ifndef FILE_SYSTEM_FAT16_H
#define FILE_SYSTEM_FAT16_H

#ifndef STDINT_H
#include "stdint.h"
#endif

#include "../file_system.h"

#define FAT16_MINIMUM_SIZE 1024
#define FAT16_MAXIMUM_SIZE 17179869184


typedef struct BootSector {
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

typedef struct FAT16File {
    unsigned char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creationTimeTenth;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessedDate;
    uint16_t fileClusterStart;
    uint16_t timeOfLastWrite;
    uint16_t dateOfLastWrite;
    uint16_t fileClusterEnd;
    uint32_t fileSize;
} FAT16File; // Total volumeSize: 32 bytes

typedef struct FAT16FileNameExtension{
    uint8_t order;
    unsigned char name1[10];
    uint8_t attributes;
    uint8_t type;
    uint8_t checkSum;
    unsigned char name2[12];
    uint16_t firstClusterNumberLowWord;
    unsigned char name3[4];
} FAT16FileNameExtension; // Total volumeSize: 32 bytes

// ^ Based on "Microsoft Extensible Firmware Initiative FAT32 File System Specification"

enum DirectoryAttributes {
    DIR_ATTR_READONLY  = 1 << 0,
    DIR_ATTR_HIDDEN    = 1 << 1,
    DIR_ATTR_SYSTEM    = 1 << 2,
    DIR_ATTR_VOLUME_ID = 1 << 3,
    DIR_ATTR_DIRECTORY = 1 << 4,
    DIR_ATTR_ARCHIVE   = 1 << 5,
    DIR_ATTR_LONGNAME  = 0xF
};





BootSector initBootSector(uint32_t volumeSize);

unsigned char directoryNameChecksum(unsigned char *name);



void printBootSector(BootSector *bootSector);
void printFATTable(FATVolumeInfo *volumeInfo, RawVolume* volume);
void printFAT16File(FAT16File *file);

FATVolumeInfo* initFATVolumeInfo(BootSector bootSector);

// TODO remove when this gets added to Delft-OS
uint16_t swapEndianness16Bit(uint16_t num);

bool check_FAT16_formattible(RawVolume *raw_volume);


FormattedVolume * formatFAT16Volume(RawVolume *volume);
FormattedVolume *initFormattedVolume(RawVolume *volume, FATVolumeInfo *volumeInfo);

volume_ptr findNextFreeSector(FormattedVolume* volume);

bool FAT16Write(FormattedVolume* self, FileMetadata* fileMetadata, void* fileData);
void* FAT16Read(FormattedVolume* self,  FileMetadata* fileMetadata);


FAT16File convertMetadataToFAT16File(FileMetadata *fileMetadata);
uint8_t convertToDirAttributes(FileMetadata* file);

void writeMetaData(FormattedVolume* self, FAT16File fileMetadata, volume_ptr startSector, volume_ptr endSector);
void writeSector(FormattedVolume* self, void* data, volume_ptr sector, uint32_t dataSize);
void writeFATS(FormattedVolume* self, volume_ptr FATAddress, void *nextSector);
void* readSector(FormattedVolume* self, volume_ptr sector);


#endif //FILE_SYSTEM_FAT16_H

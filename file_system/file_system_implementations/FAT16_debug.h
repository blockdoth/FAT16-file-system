//
// Created by pepij on 14/03/2024.
//

#ifndef FILE_SYSTEM_FAT16_DEBUG_H
#define FILE_SYSTEM_FAT16_DEBUG_H


#include <stdio.h>
#include "../file_system.h"
#include "FAT16.h"


void printBootSector(BootSector *bootSector);
void printFATTable(FormattedVolume* self);
void printFAT16File(FAT16File *file);
void printRootSectorShort(FormattedVolume* self);
void printFAT16Layout(FormattedVolume *self);
void printTreeSubDir(FormattedVolume* self, volume_ptr tableStart,char* prefix);
void printTree(FormattedVolume* self);

#endif //FILE_SYSTEM_FAT16_DEBUG_H

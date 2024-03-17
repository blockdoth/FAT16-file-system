#ifndef FILE_SYSTEM_FAT16_DEBUG_H
#define FILE_SYSTEM_FAT16_DEBUG_H

#include "FAT16_utility.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>


void printBootSector(BootSector *bootSector);
void printFATTable(FormattedVolume* self);
void printFAT16File(FAT16File *file);
void printRootSectorShort(FormattedVolume* self);
void printFAT16Layout(FormattedVolume *self);
void printTreeRecursive(FormattedVolume* self, volume_ptr tableStart, char* prefix);
void printTree(FormattedVolume* self);

#endif //FILE_SYSTEM_FAT16_DEBUG_H

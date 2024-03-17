#ifndef FILE_SYSTEM_FAT16_DEBUG_H
#define FILE_SYSTEM_FAT16_DEBUG_H

#include "FAT16_utility.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define RED "\033[91m"
#define ORANGE "\033[38;5;208m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PURPLE "\033[95m"
#define CYAN "\033[96m"
#define WHITE "\033[37m"
#define RESET "\033[0m"


void printBootSector(BootSector *bootSector);
void printFATTable(FormattedVolume* self);
void printFAT16File(FAT16File *file);
void printRootSectorShort(FormattedVolume* self);
void printFAT16Layout(FormattedVolume *self);
void printTreeRecursive(FormattedVolume* self, volume_ptr tableStart, char* prefix);
void printTree(FormattedVolume* self);

#endif //FILE_SYSTEM_FAT16_DEBUG_H

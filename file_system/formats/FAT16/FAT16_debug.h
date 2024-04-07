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

// Warning, very leaky code ahead

// Prints the content of the bootsector
void printBootSector(BootSector *bootSector);
// Prints the content of the first FAT table
void printFATTable(FormattedVolume* self);
// Prints metadata of a FAT16 file
void printFAT16File(FAT16File *file);
// Prints a short overview of the rootsector
void printRootSectorShort(FormattedVolume* self);
// Prints the layout of the FAT16 formatted volume, including all offsets
void printFAT16Layout(FormattedVolume *self);
// Prints a recursive tree representation of the filesystem (very leaky)
void printTree(FormattedVolume* self);
// Helper for this ^
char* printTreeToString(FormattedVolume* self);
// Prints the content of the cache TODO make it pretty
void printCache(FormattedVolume* self);

#endif //FILE_SYSTEM_FAT16_DEBUG_H

#ifndef FILE_SYSTEM_RAMDISK_H
#define FILE_SYSTEM_RAMDISK_H
#include "../../volume.h"


#include "malloc.h"
#include "string.h"

// Initializes a memory based rawVolume object
RawVolume* prep_ramdisk();

#endif //FILE_SYSTEM_RAMDISK_H

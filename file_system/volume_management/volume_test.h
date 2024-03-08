#include "volume.h"
#ifndef STDINT_H
#include "stdint.h"
#endif

#ifndef STRING_H
#include "string.h"
#endif

#ifndef STDIO_H
#include "stdio.h"
#endif

typedef struct dataBlock {
    char* data;
    uint32_t size;
} dataBlock;

bool test_volume(RawVolume* volume);


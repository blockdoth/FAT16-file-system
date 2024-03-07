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

typedef struct Block {
    char* data;
    uint32_t size;
} Block;

bool test_volume(raw_volume* volume);


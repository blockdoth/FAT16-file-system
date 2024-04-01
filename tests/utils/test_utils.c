#include "test_utils.h"


char* randomString(uint32_t length){
    char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint8_t charsLength = 52;
    char* randomName = (char*) malloc(length);
    for (int i = 0; i < length; i++) {
        randomName[i] = chars[rand() % charsLength];
    }
    return randomName;
}

char* generatePath() {
    uint8_t depth = rand() % MAX_DIR_DEPTH + 1;
    uint8_t* nameLengths = (uint8_t*) malloc(depth);
    uint8_t totalLength = 1;
    for (int i = 0; i < depth; i++) {
        nameLengths[i] = rand() % (MAX_NAME_LENGTH - MIN_NAME_LENGTH + 1) + MIN_NAME_LENGTH;
        totalLength += nameLengths[i];
    }
    char* path = (char*)malloc(totalLength);
    strcpy(path, "");
    uint8_t offset = 1;
    for (int i = 0; i < depth; ++i) {
        uint8_t len = nameLengths[i];
        char* name = randomString(len);
        strncpy(path + offset, name, len);
        //free(name); // Causes segfault sometimes, idk why
        offset += len;
        strcpy(path + offset++, "|");
    }
    free(nameLengths);
    path[totalLength - 1] = '\0';
    return path;
}

bool memCompare(char* fileA, char* fileB, uint32_t size){
    for (uint32_t i = 0; i < size; ++i) {
        char* a = fileA + i;
        char* b = fileB + i;
        if(*a != *b){
            printf("Found difference at %u", i);
            return false;
        }
    }
    return true;
}

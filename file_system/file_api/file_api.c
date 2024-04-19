//
// Created by pepij on 18/04/2024. Birthday :)
//
#include "file_api.h"
#include "../file_system_api/file_system_api.h"


FD addFileTableEntry(void* file, char* path){
    int i = 0;
    FiletableEntry* entry;
    while(i < FILETABLE_SIZE){
        entry = fileTable[i];
        if(entry->free == true) break;
        i++;
    }
    entry->file = file;
    DFILE* stat = (DFILE*) malloc(sizeof(DFILE));
    stat->file_descriptor = i;
    stat->size = 0;
    stat->pos = 0;
    stat->leftDiffRange = -1;
    stat->rightDiffRange = 0;
    entry->path = strdup(path);
    entry->stat = stat;
    entry->free = false;
    return i;
}


void initFileTable(){
    for (int i = 0; i < FILETABLE_SIZE; ++i) {
        FiletableEntry* entry = (FiletableEntry*)malloc(sizeof(FiletableEntry));
        entry->perms = NOTHING;
        entry->path = NULL;
        if(i < 3) {
            entry->free = false; // For stdin/stdout/stderror
        } else{
            entry->free = true;
        }
        fileTable[i] = entry;
    }
}
void destroyFileTable(){
    for (int i = 3; i < FILETABLE_SIZE; ++i) {
//        free(fileTable[i]->stat);
//        if(fileTable[i]->path != NULL) free(fileTable[i]->path);
//        free(fileTable[i]->file);
//        free(fileTable[i]);
    }
}

void flushFile(FiletableEntry* entry){
    DFILE* stat = entry->stat;
    if(stat->leftDiffRange == -1 || stat->rightDiffRange == 0) return;
    fs_update_file(entry->path, entry->file+stat->leftDiffRange, stat->rightDiffRange,stat->leftDiffRange);
}

void updateDiffRange(FiletableEntry* entry, uint32_t leftRange, uint32_t rightRange){
    DFILE* stat = entry->stat;
    if(stat->leftDiffRange > leftRange){
        stat->leftDiffRange = leftRange;
    }
    if(stat->rightDiffRange < rightRange){
        stat->rightDiffRange = rightRange;
    }
}

FD dfopen(char* path, PERMS perms){
    if(!fs_file_exists(path)) return -1; // TODO error handling
    void* file = fs_read_file(path); //TODO streaming
    return addFileTableEntry(file, path);
}


void dfclose(FD fd){
    FiletableEntry* entry = fileTable[fd];
    flushFile(entry);
    free(entry->stat);
    entry->free = true;
    free(entry->file);
}

void dfflush(FD fd){
    FiletableEntry* entry = fileTable[fd];
    flushFile(entry);
}

void dfrawread(FD fd, void* buffer, uint32_t size, uint32_t offset){
    FiletableEntry* entry = fileTable[fd]; //TODO error handling
    memcpy(buffer, entry->file + offset, size);
}
char dfgetc(FD fd){
    FiletableEntry* entry = fileTable[fd];
    DFILE* stat = entry->stat;
    char temp = ((char*) entry->file)[stat->pos++];
    return temp;
}
//Gets a string of length n or until '\0' when n = -1
char* dfgetstr(FD fd, uint32_t n){
    FiletableEntry* entry = fileTable[fd];
    char* string = (char*) malloc(n + 1);
    memcpy(string, entry->file + entry->stat->pos, n);
    string[n] = '\0';
    entry->stat->pos += n;
    return string;
}

#define MAX_BUFFERED_WRITES 5
uint8_t writeCount;
void dfrawwrite(FD fd, void* src, uint32_t size, uint32_t offset){
    FiletableEntry* entry = fileTable[fd];
    memcpy(entry->file + offset, src, size);
    writeCount++;
    updateDiffRange(entry, offset, offset+size);
    if(writeCount > MAX_BUFFERED_WRITES){
        writeCount = 0;
        flushFile(entry);
    }
}
void dfprintf(FD fd, char* str, uint32_t size){

}
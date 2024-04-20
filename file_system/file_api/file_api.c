//
// Created by pepij on 18/04/2024. Birthday :)
//
#include "file_api.h"
#include "file_table.h"

static FiletableEntry* fileTable[FILETABLE_SIZE];

void flushFile(FiletableEntry* entry){
    DFILE* stat = entry->stat;
    if(stat->leftDiffRange == -1 || stat->rightDiffRange == 0) return;
    uint32_t newDataSize = stat->rightDiffRange - stat->leftDiffRange;
    if(stat->size == stat->leftDiffRange){
        fs_expand_file(entry->path, entry->file,newDataSize);
    }else{
        fs_update_file(entry->path, entry->file + stat->leftDiffRange, newDataSize ,stat->leftDiffRange);
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
    flushFile(fileTable[fd]);
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
//Gets a string of length length or until '\0' when length = -1
char* dfgetstr(FD fd, uint32_t length){
    FiletableEntry* entry = fileTable[fd];
    char* string = (char*) malloc(length + 1);
    memcpy(string, entry->file + entry->stat->pos, length);
    string[length] = '\0';
    entry->stat->pos += length;
    return string;
}

void dfrawwrite(FD fd, void* src, uint32_t size, uint32_t offset){
    FiletableEntry* entry = fileTable[fd];
    memcpy(entry->file + offset, src, size);
    entry->stat->size += size;
    updateDiffRange(entry, offset, offset+size);
    writeFile(entry);
}

void dfcatc(FD fd, char c){
    FiletableEntry* entry = fileTable[fd];
    DFILE* stat = entry->stat;
    resizeFile(entry, stat->size + 1);
    ((char*) entry->file)[stat->pos] = c;
    updateDiffRange(entry, stat->pos, stat->pos + 1);
    stat->pos++;
    writeFile(entry);
}

void dfcatstr(FD fd, char* str, uint32_t length){
    FiletableEntry* entry = fileTable[fd];
    DFILE* stat = entry->stat;
    resizeFile(entry, stat->size + length);
    memcpy(entry->file + stat->pos, str, length);
    updateDiffRange(entry, stat->pos, stat->pos + length);
    stat->pos += length;
    writeFile(entry);
}

void dfprintf(char* str, uint32_t size){
    //TODO
}




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
        entry->bufferedWrites = 0;
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

void updateDiffRange(FiletableEntry* entry, uint32_t leftRange, uint32_t rightRange){
    DFILE* stat = entry->stat;
    if(stat->leftDiffRange > leftRange){
        stat->leftDiffRange = leftRange;
    }
    if(stat->rightDiffRange < rightRange){
        stat->rightDiffRange = rightRange;
    }
}

void writeFile(FiletableEntry* entry){
    if(entry->bufferedWrites++ > MAX_BUFFERED_WRITES){
        entry->bufferedWrites = 0;
        flushFile(entry);
    }
}


void resizeFile(FiletableEntry* entry, uint32_t newSize){
    if (newSize > entry->stat->size){
        void* newFile = malloc(newSize);
        memcpy(newFile, entry->file, entry->stat->size);
        free(entry->file);
        entry->file = newFile; // TODO make this not bad
        entry->stat->size = newSize;
    }
}
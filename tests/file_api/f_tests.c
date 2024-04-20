#undef DEBUG_FAT16
#include "f_tests.h"
#include "../framework/test_framework.h"



void openCloseTest(){
    setupFormattedVolume();
    char* path = "#R|file";
    void* data = randomString(1024);
    fs_create_file(path,data, 1024);

    FD* fds = (FD*) malloc((FILETABLE_SIZE - 3) * sizeof(FD));
    for (int i = 0; i < FILETABLE_SIZE - 3; ++i) {
        fds[i] = dfopen(path, WRITE);
    }
    for (int i = 0; i < FILETABLE_SIZE - 3; ++i) {
        assert_int_equals(i + 3, fds[i]);
        dfclose(fds[i]);
        assert_int_equals(i + 3, fds[i]);
    }
    free(fds);
    fs_destroy(DRIVE_R);
    free(data);
}

void openRawReadClose(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    void* data = randomString(fileSize);
    fs_create_file(path,data, fileSize);


    FD fp = dfopen(path, WRITE);
    char* file = malloc(fileSize);
    dfrawread(fp,file,fileSize,0);
    assert_mem_equals(data, file, fileSize);
    dfclose(fp);
    fs_destroy(DRIVE_R);
    free(data);
}
void getC(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* data = randomString(fileSize);
    fs_create_file(path,data, fileSize);


    FD fp = dfopen(path, WRITE);

    for (int i = 0; i < fileSize; ++i) {
        assert_int_equals(data[i], dfgetc(fp));
    }

    dfclose(fp);
    fs_destroy(DRIVE_R);
    free(data);
}

void getStr(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* data = randomString(fileSize);
    fs_create_file(path,data, fileSize);


    FD fp = dfopen(path, WRITE);
    uint16_t chunkSize = 5;
    uint16_t strSize = fileSize / chunkSize;
    for (int i = 0; i < chunkSize; ++i) {
        char* str = dfgetstr(fp, strSize);
        assert_mem_equals(str, data + i * strSize, strSize);
        free(str);
    }

    dfclose(fp);
    fs_destroy(DRIVE_R);
    free(data);

}

void writeCloseOpen(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* initialData = randomString(fileSize);
    char* writeData = randomString(fileSize);
    fs_create_file(path,initialData, fileSize);

    FD fp = dfopen(path, WRITE);
    dfrawwrite(fp, writeData, fileSize, 0);
    char* returnedData = (char*) malloc(fileSize);
    dfrawread(fp,returnedData,fileSize,0);
    memCompare(writeData, returnedData, fileSize);
    dfclose(fp);
    fs_destroy(DRIVE_R);
    free(initialData);
    free(writeData);
    free(returnedData);
}

void updateCloseOpen(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";

    char* initialData = randomString(fileSize);
    fs_create_file(path,initialData, fileSize);

    uint16_t chunkSize = fileSize / 2;
    char* writeData = randomString(chunkSize);

    FD fp = dfopen(path, WRITE);
    dfrawwrite(fp, writeData, chunkSize, chunkSize / 2);
    // Read the chunk
    char* returnedDataChunk = (char*) malloc(chunkSize);
    dfrawread(fp,returnedDataChunk,chunkSize,chunkSize / 2);
    assert_mem_equals(writeData, returnedDataChunk, chunkSize);
    // Read the whole file
    memcpy(initialData + chunkSize / 2, writeData, chunkSize);
    char* returnedData = (char*) malloc(fileSize);
    dfrawread(fp,returnedData,fileSize,0);
    assert_mem_equals(initialData, returnedData, fileSize);

    dfclose(fp);
    fs_destroy(DRIVE_R);
    free(initialData);
    free(writeData);
    free(returnedData);
}

void writeFlushClose(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* initialData = randomString(fileSize);
    char* writeData = randomString(fileSize);
    fs_create_file(path,initialData, fileSize);


    FD fp = dfopen(path, WRITE);
    dfrawwrite(fp, writeData, fileSize, 0);
    char* returnedData = (char*) malloc(fileSize);
    dfrawread(fp,returnedData,fileSize,0);
    assert_mem_equals(writeData, returnedData, fileSize);

    char* returnedNonFlushedFile = fs_read_file(path);
    assert_mem_equals(initialData,returnedNonFlushedFile, fileSize); // Assert volume data didnt change yet
    dfclose(fp); // Data gets flushed to volume
    char* returnedFlushedFile = fs_read_file(path); // Assert volume data changed
    assert_mem_equals(writeData,returnedFlushedFile, fileSize);
    fs_destroy(DRIVE_R);
    free(initialData);
    free(writeData);
    free(returnedData);
    free(returnedNonFlushedFile);
    free(returnedFlushedFile);
}


void writeFlushCloseOpenLoop(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    uint16_t loopCount = MAX_BUFFERED_WRITES; // Loop until buffer gets automatically flushed
    char* path = "#R|file";
    char* initialData = randomString(fileSize);
    fs_create_file(path,initialData, fileSize);
    for (int i = 0; i < loopCount; ++i) {
        FD fp = dfopen(path, WRITE);
        char* writeData = randomString(fileSize);
        dfrawwrite(fp, writeData, fileSize, 0);
        char* returnedData = (char*) malloc(fileSize);
        dfrawread(fp,returnedData,fileSize,0);
        assert_mem_equals(writeData, returnedData, fileSize);
        char* returnedNonFlushedFile = fs_read_file(path);
        assert_mem_equals(initialData,returnedNonFlushedFile, fileSize); // Assert volume data didnt change yet
        dfclose(fp); // Data gets flushed to volume
        char* returnedFlushedFile = fs_read_file(path); // Assert volume data changed
        assert_mem_equals(writeData,returnedFlushedFile, fileSize);
        memcpy(initialData, writeData, fileSize);
        free(writeData);
        free(returnedData);
        free(returnedNonFlushedFile);
        free(returnedFlushedFile);
    }
    free(initialData);

    fs_destroy(DRIVE_R);
}


void openCatcClose(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* initialData = randomString(fileSize);

    fs_create_file(path,initialData, 0);
    FD fp = dfopen(path, WRITE);

    uint32_t i = 0;
    while (i < fileSize){
        dfcatc(fp, initialData[i++]);
    }
    dfflush(fp);
    char* returnedData = (char*) malloc(fileSize);
    dfrawread(fp,returnedData,fileSize,0);
    dfclose(fp);
    assert_mem_equals(initialData, returnedData, fileSize);
    free(initialData);
    free(returnedData);
    fs_destroy(DRIVE_R);

}void openCatstrClose(){
    setupFormattedVolume();
    uint16_t fileSize = 1024;
    char* path = "#R|file";
    char* initialData = randomString(fileSize);

    fs_create_file(path,initialData, fileSize);
    FD fp = dfopen(path, WRITE);

    uint32_t chunkSize = fileSize / 8;
    uint32_t i = 0;
    char* chunk = (char*)malloc(chunkSize);
    while (i < chunkSize){
        memcpy(chunk, initialData + i * chunkSize, chunkSize);
        dfcatstr(fp, chunk, chunkSize);
        i++;
    }
    dfflush(fp);
    free(chunk);
    char* returnedData = (char*) malloc(fileSize);
    dfrawread(fp,returnedData,fileSize,0);
    dfclose(fp);
    memCompare(initialData, returnedData, fileSize);
    fs_destroy(DRIVE_R);
}


void register_f_tests(){
    register_test(openCatstrClose);
    register_test(openCatcClose);
    register_test(writeFlushCloseOpenLoop);
    register_test(writeFlushClose);
    register_test(updateCloseOpen);
    register_test(writeCloseOpen);
    register_test(getStr);
    register_test(getC);
    register_test(openRawReadClose);
    register_test(openCloseTest);
}
#include "volume_test.h"

bool test_volume(RawVolume* volume){
    dataBlock data_1 = {
            "Hello world",
            12
    };
    int location_1 = 0;
    printf("Testing ramdisk\n");

    volume->write(volume, (uint32_t *) data_1.data, location_1, data_1.size);
    char* string = (char*) volume->read(volume, location_1, data_1.size);

    if(strcmp(data_1.data, string) != 0){
        printf("Returned data is not the same\n"
               "Expected:\n\t%s\nFound:\t\t %s\n", data_1.data, string);
        return 1;
    }

    printf("System check passed\n");

    return 0;
}
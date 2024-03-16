#include "volume_test.h"


typedef struct dataBlock {
    char* data;
    uint32_t size;
} dataBlock;



void test_volume(){
    //assert_int_equals(1,1);
}

void register_volume_tests(){
    register_test(test_volume);
}


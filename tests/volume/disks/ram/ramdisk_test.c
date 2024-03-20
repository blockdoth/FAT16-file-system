#include "ramdisk_test.h"

void init_test(){

    assert_int_equals(1,2);
}



void register_ramdisk_tests(){
     register_test(init_test);
}
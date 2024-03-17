#include <stdio.h>

#include "file_system_api.h"

#define GiB 1073741823 // Bytes


//#define DEBUG_VOLUME // Enable debug mode


void* rickRoll;
void* helloWorld = "Hello World";

int main() {
    //printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB);

    fs_format(raw_volume, FAT16);
    uint32_t rickLen = strlen((char*) rickRoll) + 1;

    fs_create_dir( "#rootDirA");
    fs_create_dir( "#rootDirA|subDirA");
    fs_create_dir( "#rootDirA|subDirA|subDirB");
    fs_create_dir( "#rootDirA|subDirC");
    fs_create_dir( "#rootDirD");

    fs_create_file("#rootDirA|subDirA|fileA.txt", rickRoll, rickLen);
    fs_create_file("#rootDirA|subDirA|subDirB|fileB.txt", rickRoll, rickLen);
    fs_create_file("#rootDirA|subDirC|fileC.txt", rickRoll, rickLen);
    fs_create_file("#rootDirD|fileC.txt", rickRoll, rickLen);
    fs_create_file("#hw.txt", helloWorld, 11);

    char* dirPath = "#rootDirA|subDirA|subDirB";
    if(fs_dir_exists(dirPath)){
        printf("Found dir at %s\n", dirPath);
    }
    char* filePath = "#rootDirA|subDirA|subDirB|fileB.txt";
    if(fs_file_exists(filePath)){
        printf("Found rickRoll at %s\n", filePath);
    }

    fs_update_file("#hw.txt", rickRoll, rickLen);
    char* string = (char *) fs_read_file("#hw.txt");
    printf("File content:\n%s",string);
    free(string);

    fs_destroy();

    return 0;
}



void* rickRoll = "We're no strangers to love, you know the rules and so do I\n"
                         "A full commitments what I'm thinking of\n"
                         "You wouldn't get this from any other guy.\n"
                         "I just wanna tell you how I'm feeling, gotta make you understand.\n"
                         "\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.\n"
                         "\n"
                         "We've known each other for so long\n"
                         "Your heart's been aching, but you're too shy to say it\n"
                         "Inside we both know what's been going on\n"
                         "We know the game and we're gonna play it.\n"
                         "And if you ask me how I'm feeling, don't tell me you're too blind to see.\n"
                         "\n"
                         "\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.\n"
                         "\n"
                         "(Ooh, give you up, ooh, give you up)\n"
                         "Never gonna give, never gonna give (Give you up)\n"
                         "Never gonna give, never gonna give (Give you up)\n"
                         "\n"
                         "We've known each other for so long\n"
                         "Your heart's been aching, but you're too shy to say it\n"
                         "Inside we both know what's been going on\n"
                         "We know the game and we're gonna play it.\n"
                         "I just wanna tell you how I'm feeling, gotta make you understand.\n"
                         "\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.\n"
                         "Never gonna give you up, never gonna let you down\n"
                         "Never gonna run around and desert you\n"
                         "Never gonna make you cry, never gonna say goodbye\n"
                         "Never gonna tell a lie and hurt you.";
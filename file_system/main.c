#include <stdio.h>
#include "file_system_api.h"

#define GiB 1073741823 // Bytes

// Only used for debugging and testing purposes

void* rickRoll;
void* helloWorld = "Hello World ";

int main() {
    //printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB);

    fs_format(raw_volume, (FormatSpecifier){FAT16,{512,64}}, DRIVE_R);
    uint32_t rickLen = strlen((char*) rickRoll) + 1;

    fs_create_dir( "#R|rootDirA");
    fs_create_dir( "#R|rootDirA|subDirA");
    fs_create_dir( "#R|rootDirA|subDirA|subDirB");
    fs_create_dir( "#R|rootDirA|subDirC");
    fs_create_dir( "#R|rootDirD");

    fs_create_file("#R|rootDirA|subDirA|fileA.txt", rickRoll, rickLen);
    fs_create_file("#R|rootDirA|subDirA|subDirB|fileB.txt", rickRoll, rickLen);
    fs_create_file("#R|rootDirA|subDirC|fileC.txt", rickRoll, rickLen);
    fs_create_file("#R|rootDirD|fileC.txt", rickRoll, rickLen);
    fs_create_file("#R|fileD", rickRoll, rickLen);
    fs_create_file("#R|fileE", helloWorld, 12);
    fs_create_file("#R|fileF", helloWorld, 12);

    char* dirPath = "#R|rootDirA|subDirA|subDirB";
    if(fs_dir_exists(dirPath)){
        printf("Found dir at %s\n", dirPath);
    }
    char* filePath = "#R|fileE";
    if(fs_file_exists(filePath)){
        printf("Found rickRoll at %s\n", filePath);
    }
    fs_delete_file(filePath);
    if(!fs_file_exists(filePath)){
        printf("File %s has been deleted\n", filePath);
    }else{
        printf("File %s stil exists\n", filePath);
    }
    char* dir = "#R|rootDirA|subDirA";
    fs_delete_dir(dir);
    if(!fs_dir_exists(dir)){
        printf("Dir %s has been deleted\n", dir);
    }else{
        printf("File %s still exists\n", dir);
    }
    char* tree = fs_tree("#R|");
    printf("%s",tree);

    char* string = (char *) fs_read_file("#R|fileF");
    printf("File content before expand:\n%.*s\n",12, string);
    free(string);

    uint32_t newSize = fs_expand_file("#R|fileF", helloWorld, 12);
    string = (char *) fs_read_file("#R|fileF");
    printf("File content after expand:\n%.*s\n",newSize, string);
    free(string);

    uint32_t reducedSize = fs_update_file("#R|fileF", helloWorld, 12, 6);
    string = (char *) fs_read_file("#R|fileF");
    printf("File content after update:\n%.*s\n", reducedSize, string);
    free(string);

    fs_destroy(DRIVE_R);

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
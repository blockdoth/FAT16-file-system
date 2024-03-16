#include <stdio.h>

#include "file_system.h"


#define GiB 1073741823 // Bytes


system_file_data file;


int main() {
    printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB);

    fs_format(raw_volume, FAT16);


    system_file_metadata rootdir = {
            "RootdirA",
            "",
            0,
            0,
            0,
            0
    };
    system_file_metadata subDirA = {
            "SubdirA",
            "RootdirA|",
            0,
            0,
            0,
            0
    };
    system_file_metadata subDirB = {
            "SubdirB",
            "RootdirA|SubdirA|",
            0,
            0,
            0,
            0
    };
    system_file_metadata subDirC = {
            "RootdirB",
            "",
            0,
            0,
            0,
            0
    };

    system_file_metadata fileA = {
            "FileA.txt",
            "",
            strlen((char*) file) + 1,
            0,
            0,
            0
    };
    system_file_metadata fileB = {
            "FileB.txt",
            "RootdirA|",
            strlen((char*) file) + 1,
            0,
            0,
            0
    };

    system_file_metadata fileC = {
            "FileC.txt",
            "RootdirA|SubdirA|SubdirB|",
            strlen((char*) file) + 1,
            0,
            0,
            0
    };

    system_file_metadata fileD = {
            "FileD.txt",
            "",
            strlen((char*) file) + 1,
            0,
            0,
            0
    };
    fs_create_dir( &rootdir);
    fs_create_dir( &subDirA);
    fs_create_dir( &subDirB);
    fs_create_dir( &subDirC);

    fs_create_file(&fileA, file);
    fs_create_file(&fileB, file);
    fs_create_file(&fileC, file);
    fs_create_file(&fileD, file);
    char* string = (char *) fs_read_file(&fileC);
    printf("File content:\n%s",string);
    free(string);

    fs_destroy();

    return 0;
}



system_file_data file =  "We're no strangers to love, you know the rules and so do I\n"
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
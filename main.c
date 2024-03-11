#include <stdio.h>
#include "file_system/volume_management/volume.h"
#include "file_system/volume_management/volume_test.h"
#include "file_system/file_system.h"
#include "file_system/file_system_implementations/FAT16.h"

#define GiB 1073741824 // Bytes





int main() {
    printf("Mounting ramdisk\n");
    RawVolume* raw_volume = mount_volume(RAM_DISK,  GiB  - 1);
    if(test_volume(raw_volume)){
        return 1;
    }

    format_volume(raw_volume, FAT16);

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
    uint32_t file_length = strlen((char*) file) + 1;

    system_file_metadata file_metadata = {
            "file.txt",
            0,
            1,
            0,
            0,
            0,
            0,
            0,
            file_length,
    };


    fs_create_file(&file_metadata, file);
    char* string = (char *) fs_read_file(&file_metadata);
    printf("File content:\n%s",string);

    return 0;
}

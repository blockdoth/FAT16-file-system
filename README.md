
A filesystem developed for the DelftOS project

My first large c program, be aware of the skill issues

## **Overview**

* The FAT16 file format is (almost) entirely supported
* The filesystem lives in memory on a "ramdisk"
* Caching is supported on a sector level
* Object oriented ish volume and formatted volume management makes the code easily extendable to support different kind of drives or other filesystem formats
* Supports multiple drives and separate volumes each with their own filesystem or volume type
* File manipulation API supported with buffered IO using the open/flush/close pattern and a global file table
* All outside facing API is thoroughly tested
* Features a gay file tree print function
* The filesystem is NOT yet integrated into DelftOS

## **How to use**

All outwards facing API's lives in `file_system_api.h` and `file_api.h`. Include these to get working. Be aware that `file_api.h` is not very fleshed out yet

The filesystem API contains all functions you will need to manage files and directories in a file system, but provides little support for the manipulation of the data of individual files. That is why the file API builds on the filesystem API in order to make working with files a little more convenient. It is entirely built upon the file system API and does nothing you can not do with the file system API alone.

The most important functionality of the file API is that it handles buffering file data for you to prevent excessive IO usage. This is done using the well know open/flush/close and file pointer pattern you should be accustomed to.

## **How to understand the code**

If you want to understand the code I advise checking out the files in the following order

* Start with `ramdisk.c` to understand how the most basic IO operations on the volume work, make sure to check out the function `prep_ramdisk()` to understand how the `RawVolume` object is composed. This type of object creation is used again for volumes formatted to a filesystem
* Take a look at `FAT16_IO.c` to understand how the FAT16 filesystem interacts with the `RawVolume` to read and write sectors, this file also contains the logic for caching.
* Browse trough `FAT16.c` to understand the implementation of the FAT16 format, the functions in this file are generally pretty high level and composed of helper functions from `FAT16_utility.c` I advise starting at the function `initFormattedVolume()` which composes the `FormattedVolume` object. Check out each function that gets added to this object one by one and make sure to also take a look at the helper functions used each time to properly understand what is happening.
* After understanding all the implementation level functionality go to `filesystem.c` to view the implementation agnostic functions to manipulate the file system. This file also contains some helper functions to deal with parsing and converting the user supplied data to internal formats.
* Built onto this raw file system API is the file API in `file_api.c` this contains a small subset of `stdio` inspired file manipulation features. `df_open/df_flush/df_close` are the most important because they control the lifecycle of a file and allow buffered IO usage which massively reduces actual IO operations

I generally tried to use extremely descriptive function and variable name to make the code more understandable, this should do a lot of self documenting I hope. Although it might be a bit verbose I believe its very helpful when doing convoluted calculations.

## **Design Challenges**

One of the biggest challenges when manipulating files on a volume is the addressing precession you are given. It is therefore important to understand how addressing on a volume works:

A volume consisting of a sequence of bits in an arbitrary length is divided in sectors of a constant size, a sector is the smallest addressable region on a volume. This means you will only be able to read and write data on a volume in blocks the size of a sector. Multiple sectors are grouped together in a cluster. Clusters are the smallest allocatable regions on a volume within the context of ownership management. In FAT16 the File Allocation Table allocates sectors in the data region based on the cluster number. This means that the smallest size a file can be is the size of a sector multiplied by the amount of sectors in a cluster. In our current implementation that is 512x64 = 32768 bytes.

Its important that the user of a the filesystem doesn't have to worry about this addressing system. Writing and reading of files with an arbitrary length should be supported. This means that we have to deal with the following scenarios:

* Reading/Writing a partial sector
* Reading/Writing a full sector
* Reading/Writing a partial cluster (multiple sectors)
* Reading/Writing a full cluster
* Reading/Writing a multiple clusters

The strategy often used it to create increasingly more alignment with sector/cluster boundaries.

When updating or expanding a file data can often span halfway though a sector, in the first step we write the sector until its full, this make further writes sector aligned. Then we write to the cluster sector by sector until its full, after that we ask the FAT for a new cluster location so we can continue writing the file. Writing is now aligned with cluster boundaries. When approaching the end of a file careful consideration needs to be used to make sure that clusters and sector writing loops are exited early in order to prevent touching data out of the bounds of the file on the volume.

## **FAT16**

FAT16 has been implemented according to the official design spec from Microsoft

The most important concept to understand is the FAT table, which encoded 2 pieces of information.

The index of an entry in the FAT marks the ownership of a cluster in the data region of the volume, the value of the entry is a pointer to the index of the next entry in the FAT that belongs to the same file, creating a single linked list of clusters in the FAT for each file. A special entry value is used to mark the end of a file. The metadata of a file only contains the index of the start of the file, the rest of the locations of the file can be found by traversing the linked list trough the FAT.

The second important system to understand is the metadata management. The concept of a file is completely abstract from the point of view of the volume, all information related to each file and directory has to be tracked separately, for this we need to manage metadata entries which hold information like size, access permissions and pointers to the start of the file in the FAT table. To handle metadata FAT16 uses linear nested file tables. Metadata entries are linearly packed into sectors. Directories are created by using the sector that an entry points to as another file tables. This creates a rapidly expanding hierarchal tree structure as expected. The root folder has a special region on the disk and is fixed in size under the FAT16 standard, all child file tables can be as large as their is space for.

I made de file system API with the idea in mind that other filesystem might have different information in their metadata, that is why the file system API uses the `file_metadata` struct to pass around the metadata used by the OS itself, which can then be converted into a format specific metadata struct, for FAT16 this is called `FAT16File`

**Public facing API**

```c
=== file_system_api.h ===

// === Filesystem lifecycle ===
FS_STATUS_CODE fs_format(RawVolume* raw_volume, FormatSpecifier formatSpecifier, DriveID drive_id);
void fs_destroy(DriveID drive_id);

// === Files ===
FS_STATUS_CODE fs_create_file(char* path, void* data, uint32_t size);
void* fs_read_file(char* path);
void* fs_read_file_section(char* path, uint32_t offset, uint32_t size);
uint32_t fs_expand_file(char* path, void* new_data, uint32_t new_size);
uint32_t fs_update_file(char* path, void* new_data, uint32_t new_data_size, uint32_t offset);
FS_STATUS_CODE fs_delete_file(char* path);
FS_STATUS_CODE fs_file_exists(char* path);

// === Dirs ===
FS_STATUS_CODE fs_create_dir(char* path);
FS_STATUS_CODE fs_delete_dir(char* path);
FS_STATUS_CODE fs_dir_exists(char* path);
FS_STATUS_CODE fs_is_dir(char* path);

// === Metadata ===
file_metadata* fs_get_metadata(char* path);
FS_STATUS_CODE fs_rename(char* path, char* new_name);
```

## **Files**

The pure filesystem API can be very wasteful when making repeated small modifications to file, therefore a different usage pattern is implemented in the file API

This implementation uses a global file table to hold the data of open files

```c
 === file_api.h ===

// File lifecycle management
FD f_open(char* path, PERMS perms);
void f_close(uint16_t fd);
void f_flush(FD fd);

// Reading data from a file
void f_raw_read(FD fd, void* buffer, uint32_t size, uint32_t offset);
char f_get_c(FD fd);
char* f_get_str(FD fd, uint32_t length);

// Writing data to a file
void f_raw_write(FD fd, void* src, uint32_t size, uint32_t offset);
void f_cat_c(FD fd, char c);
void f_cat_str(FD fd, char* str, uint32_t length);
```

This API is mainly based on the `stdio.h` The primary purpose of this implementation was to allow for buffered IO, actual file manipulation functions need to be expanded a lot. File pointers are used to refer to a specific file and must be passed to the function you want to use on the file. The file pointer is an index into the global filetable which allows for fast lookup. Writing and reading is done with a cursor like system like in C. Moving the cursor is not supported yet (no `fseek()` )

#### How to add new filesystems / volumes

---

Volumes and filesystem have been setup to allow for easy extension to different formats/mediums. Object Oriented Design patterns makes a lot of sense in this context. Since a volume/filesystem ultimately boils down to encapsulated state with accessor methods that modify this internal state. That is why I decided to use this design pattern to handle volumes and formats in a modular and extendable way.

C of course does not natively support objects/classes, but we can emulate something very close to it by using function pointers and by passing the struct as the first argument. The struct is essentially an interface you need to implement in order to use the object. After initialization the usage of the object should be implementation agnostic

Example:

```c
typedef struct Object {
    uint32_t counter;
    uint32_t (*add)(struct Object, uint32_t amount);
    uint32_t (*subtract)(struct Object, uint32_t amount);
} Object;

void addImpl(Object* self, uint32_t amount){
    self->counter += amount;
}
void substractImpl(Object* self, uint32_t amount){
    self->counter -= amount;
}

Object* initObject(uint32_t startCount){
    Object* newObject = (Object*) malloc(sizeof(Object));
    newObject->counter = startCount;
    newObject->add = addImpl;
    newObject->substract = substractImpl;
}

void main(){
    Object* object = initObject(5)
    object->add(object, 5);       // => Counter is 10 now 
    object->substract(object, 9); // => Counter is 1 now 
    object->substract(object, 1); // => Counter is 0 now 
    ... 
    // Destroying the object is left out for the sake of brevity
}
```

This pattern is used to make 2 pseudo objects, `RawVolume` and `FormattedVolume`. Adding a new volume is as simple as implementing all the required functions and adding them to the `RawVolume` struct, the same holds for adding new filesystem formats using the `FormattedVolume` struct.

One caveat is that when creating multiple `RawVolume` object is that we need some way of keeping track of them. How we do this differs for each implementation. Therefore a union is used to hold this reference. In the case of the a ramdisk this unions holds a pointer to the malloc'ed array which serves as the storage medium. But in the case of for example flashdisk it might be something like a drive ID.

The `FormattedVolume` object contains a union as well, which is used to hold onto internal implementation specific information of the file system. For example the sector offsets for the different regions in the FAT16 format. This is not relevant for other filesystem which therefore need to add there own version of `VolumeInfo` to the object

The above mentioned objects:

```c
typedef struct {
    RefInfo        refInfo;  // Union, specific format depends on what is needed to keep track of distinct volumes for each volume type
    uint32_t       volumeSize;
    FS_STATUS_CODE (*init)   (struct RawVolume* self, uint32_t volumeSize);
    void           (*destroy)(struct RawVolume* self);
    FS_STATUS_CODE (*write)  (struct RawVolume* self, const void* sourceAddress, uint32_t destinationAddress, uint32_t dataSize);
    void*          (*read)   (const struct RawVolume* self, uint32_t src_addr, uint32_t dataSize);
} RawVolume;
```

```c
typedef struct {
    RawVolume*     rawVolume;
    VolumeInfo*    info;             // Union, specific format depends on filesystem format
    Cache          cache;
    FS_STATUS_CODE (*createFile)     (struct FormattedVolume* self, Path* path, file_metadata* fileMetadata, void* fileData);
    FS_STATUS_CODE (*createDir)      (struct FormattedVolume* self, Path* path, file_metadata* fileMetadata);
    void*          (*readFile)       (struct FormattedVolume* self, Path* path);
    void*          (*readFileSection)(struct FormattedVolume* self, Path* path, uint32_t offset, uint32_t size);
    FS_STATUS_CODE (*checkFile)      (struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*checkDir)       (struct FormattedVolume* self, Path* path);
    uint32_t       (*updateFile)     (struct FormattedVolume* self, Path* path, void* fileData, uint32_t dataSize, uint32_t offset);
    uint32_t       (*expandFile)     (struct FormattedVolume* self, Path* path, void* fileData, uint32_t dataSize);
    FS_STATUS_CODE (*deleteDir)      (struct FormattedVolume *self, Path* path);
    FS_STATUS_CODE (*deleteFile)     (struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*isDir)          (struct FormattedVolume* self, Path* path);
    file_metadata* (*getMetadata)    (struct FormattedVolume* self, Path* path);
    FS_STATUS_CODE (*renameFile)     (struct FormattedVolume* self, Path* path, char* newName);
    FS_STATUS_CODE (*destroy)        (struct FormattedVolume* self);
    char*          (*toString)       (struct FormattedVolume* self);
} FormattedVolume;
```

I suggest that the next volume type to be implemented are flash drives, this would allow us to use the SD card of the raspberry PI. Make sure to check out the spec sheets of the specific SD card reader used in the RPI.

## **Caching**

All actual IO calls are routed trough the following 2 functions, they are also the only place where the read and write functions of `RawVolume` are used.

```
void readSector(FormattedVolume* self, sector_ptr sector, void* buffer, uint32_t readSize);
FS_STATUS_CODE writeSector(FormattedVolume* self, sector_ptr sector, const void* data, uint32_t size);
```

Both functions can read and write only on the sector level, even when less data is requested to be read/written they will stil read/write a whole sector and discard the rest. Because of this the size of sectors is always the same which means we can easily cache them in memory. Caching sectors can massively reduce actual IO usage especially when traversing the FAT or metadata tables. It will also improve repeated read performance a lot, but this depends on the amount of sectors a file spans and the size of the cache.

Right now the sector cache uses the LRU (Least Recently Used) replacements strategy to when full. To find entries a linear traversal and sector number match is used. The amount of sectors in the cache is chosen randomly at the moment, but should be tuned to the amount of memory we want to use on our actual device.

Caching could be improved by precaching the FAT tables, the rootsectors and maybe a selective traversal of the file metadata tree up until a certain depth. Ideally these sectors would live in a separate cache which doesn't expire, since they will have a high usage ratio because all IO operations start with them. Keep in mind that its important to make sure these sectors stay in sync with the real sectors.

## Error handling

Error handling at this moment is a bit rudimentary and mostly handled by returning status code enums defined by `FS_STATUS_CODE`. Sometimes more hacky ways of communicating errors is used because the function already returns data in which we do not want to blend error messages. Not all possible error scenarios are covered yet, a lot could be improved. Sometimes deeper errors are not communicate well up the call stack which could lead to hard to debug bugs in future. Be aware of this when encountering weird bugs.

My experience with error handling in C is not very deep. I only learned about a different style of error handling at the end of the project, where immediate returns are used on errors together with a globally defined `err` variable which should be checked after each error throwing function call. Personally I think this pattern is more elegant, so if somebody feels like improving error handling I suggest looking into it.

## Testing

All high level API in `file_api.c` and `file_system_api.c` is tested using integration tests. I used a simple testing framework lifted from the CG project and adopted it a bit to my use case. Testing has been proven invaluable for debugging this project, mainly because there are a lot of edge cases which need to be thoroughly checked to make sure the code behaves correctly under the extremely broad use cases of a filesystem. Tweaking logic related to sector/cluster aligned file manipulation and nesting of files/directories often leaded to regression in the tests, which would not have been caught without relatively granular tests.

All testing lives inside its own directory, the main function lives in `test_framework.c`. The file and and filesystem API are tested in separate files which can be toggled using the `TEST_FS` and `TEST_F` defines in their corresponding headers, these these headers also contain some additional test config defines.

## Debugging

During development I wrote a lot of debugging prints to give me more insight into what the code was doing, most major functions of the FAT16 file system should have their own debug printouts which give granular information about what is happening. Be aware that enabling debug prints when doing large operations can quickly flood the output. I advise against running all tests with debugging enabled

I provided the following debug flags in the headers the related headers `DEBUG_FAT16` and `DEBUG_VOLUME`

## Integration

The filesystem is at this point in time NOT integrated with the main code of Delft OS, this is because it depends heavily on functionality that was not yet implemented during the time of its creation. Mainly `malloc` ,which did not work yet during development. Even now it is not yet able to allocate large enough segments to initialize a filesystem. Because of this reason the filesystem was developed in a completely separate project.

During development I tried to minimize external dependencies. The filesystem only needs 4 external headers to function. Two of these only provide types, the other two should be replaced by our own inhouse implementations. The debug prints will probably require the most amount of work to integrate using our macro based string manipulation implementation

The external dependencies used in the filesystem are

```c
#include "stdint.h"  
#include "stdbool.h"
#include "string.h"  // Should be replaced with our home grown macro based implementation, although I need a working memcpy aswell
#include <malloc.h>  // Our own implementation should be a drop in replacement of this. Realloc is used once for printing the gay file tree
```

The test framework has these additional dependencies

```c
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
```

I am not sure how the test framework should be included in the main project, but I do believe having a working test suite is very valuable when debugging during development. external dependencies of the test framework could probably be reduced a lot by refactoring. I have not actively worked on that.

Apart from these dependencies the project is written in pure C.

## TODO's

####  Support mounting existing formatted volumes

During development I only used a ramdisk in order to get everything working. Ram is of course by its nature non persistent. The filesystem is rebuild and reformatted on each use at the moment, with very little regard to pre-existing structures. What needs to happens is to add functionality to parse in an already formatted volume in order to actually store data persistently. Very little infrastructure for this has been made, so good luck.

#### **Support trimming/reducing files**

I did not come around to implement reducing file sizes as a separate function or as part of updating functionality. This however should not be that hard to support since all the infrastructure to manipulate the FATs is already in place, consider it an exercise to the reader

#### Support longer file names

Right now the length of file names is limited to 11 bytes in FAT16, in order to allow for longer file names long entries must be used according to the spec. Longer file names are handled by sticking additional metadata entries after the first one which contain the extra chars.

#### Expand file API file manipulation functions

Right now the amount of options to read or write files using the API is very limited, expanding these would make working with files more convenient. Use `stdio.h` as inspiration.

#### Add Copy On Write (COW) functionality to the filletable

With an active file table implementation Copy On Write functionality can be supported. Copy On Write means that when opening a file multiple times only after the a program tries to write to the file it opened a separate copy is made in the buffer. When a file is opened multiple time and only reads are performed all reads read from the same memory instead of each file pointer having its own copy. This reducing memory usage and should improve speed aswell.

#### Communicate invalidation of pages by the file system API to the file API

Right now the file API assumes that the data in the volume doesnt change after reading it in the buffer. Yet the file system API can still interact with the volume just fine creating a desync between the in memory buffered file and the actual file. There needs to be some form of communication system set in place which lets the file API know when to invalidate buffered files and reread them again from memory. We also need to decide what happens when a file is opened multiple times by different programs and one of them closes before the other.

## **References:**

Microsoft official FAT16 spec:

https://github.com/rweichler/FAT16/blob/master/spec/fatgen103.pdf

https://pages.cs.wisc.edu/\~remzi/OSTEP/file-implementation.pdf

https://www.win.tue.nl/%7eaeb/linux/fs/fat/fat-1.html

https://drakeor.com/2022/10/12/koizos-writing-a-simple-fat16-filesystem/
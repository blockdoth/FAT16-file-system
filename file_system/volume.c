#include "../include/volume.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>


raw_volume* mount_volume(VOLUME_TYPE volumeType, uint64_t size){
    raw_volume* volume = prep_volume(volumeType);
    if(volume->init(volume, size)){
        printf("Failed to mount volume\n");
    }
    printf("Successfully mounted volume\n");
    return volume;
}

bool bounds_check(raw_volume* self, uint32_t start_addr, uint32_t data_size){
    if((start_addr + data_size) > self->size){
        if (start_addr > self->size){
            printf("ERROR:\t Address %u out of bounds of volume\n", start_addr);
        }else{
            printf("ERROR:\t Allocation of %u to large from start address %u\n", data_size, start_addr);
        }
        return 1;
    }
    return 0;
}

raw_volume* prep_volume(VOLUME_TYPE volumeType){
    switch (volumeType) {
        case RAM_DISK:
            return prep_ram_disk();
        case FLASH_DRIVE:
            return prep_flash_drive();
        default:
    }
}

// === RAM setup ===

raw_volume* prep_ram_disk() {
    raw_volume* volume = (raw_volume*)malloc(sizeof(raw_volume));
    if(volume == NULL){
        return NULL;
    }
    volume->volume_data = 0;
    volume->size = 0;
    volume->init = ramdisk_init;
    volume->destroy = ramdisk_destroy;
    volume->write= ramdisk_write;
    volume->read = ramdisk_read;
    return volume;
}

bool ramdisk_init(raw_volume* self, uint32_t volume_size){
    self->volume_data = malloc(volume_size);
    if(self->volume_data == NULL){
        printf("ERROR:\t Failed to allocate ram disk");
        return 1;
    }
    self->size = volume_size;
    INIT_DEBUG_LOG(volume_size);
    return 0;
}
void ramdisk_destroy(raw_volume* self){
    DESTROY_DEBUG_LOG(self->size);
    free(self->volume_data);
    free(self);
}

bool ramdisk_write(raw_volume* self, uint32_t* src_addr, volume_ptr dest_addr, uint32_t data_size){
    if(bounds_check(self, dest_addr, data_size)){
        return 1;
    }
    WRITE_DEBUG_LOG((char*) src_addr, dest_addr, data_size);
    uint32_t* destination = (uint32_t*) (self->volume_data + dest_addr);
    memcpy(destination,src_addr,data_size);
    return 0;
}
void* ramdisk_read(raw_volume* self, volume_ptr src_addr, uint32_t data_size){
    if(bounds_check(self, src_addr, data_size)){
        return NULL;
    }
    uint32_t* chunk = malloc(data_size);
    if(chunk == NULL){
        printf("ERROR:\t Failed to allocate memory for read\n");
    }

    uint32_t* source = (uint32_t*) (self->volume_data + src_addr);
    memcpy(chunk,source,data_size);

    READ_DEBUG_LOG(src_addr, data_size, (char*) chunk);
    return chunk;
}

// === FLASH drive setup ===
raw_volume* prep_flash_drive(){
    assert(0 && "TODO");
}

#undef DEBUG
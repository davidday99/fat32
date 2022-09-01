#include <stdint.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32.h"

void format_fat32(FAT32_FS *fs) {
    return;
}

void init_fat32_fs(FAT32_FS *fs) {
    return;
}    

void deinit_fat32_fs(FAT32_FS *fs) {
    return;
}

FAT32_FILE *fat32_open(char *path, FAT32_FILE *fptr) {
    return 0;
} 

void fat32_close(FAT32_FILE *fptr) {
    return;
}

uint32_t fat32_read(FAT32_FILE *fptr) {
    return 0;
}

uint32_t fat32_write(FAT32_FILE *fptr) {
    return 0;
}


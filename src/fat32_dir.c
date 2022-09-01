#include <stdint.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_dir.h"

FAT32_DIR *fat32_opendir(FAT32_FS *fs, char *path, FAT32_DIR *dptr) {
    return 0;
}

FAT32_DIR *fat32_fopendir(FAT32_FS *fs, FAT32_FILE *fptr, FAT32_DIR *dptr) {
    //check that fptr is a directory
    return 0;
}


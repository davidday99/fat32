#ifndef _FAT32_FILE_
#define _FAT32_FILE_

#include "fat32_internal.h"

#define FAT32_MAX_FILENAME_LEN 11

typedef struct _fat32_file {
    char name[FAT32_MAX_FILENAME_LEN + 1];
    uint32_t offset;
    uint16_t clus_offset;
    uint32_t size;
    uint32_t entry_addr;
    uint32_t clus_base;
    uint32_t clus_curr;
    uint32_t is_dir;
} FAT32_FILE;

FAT32_FILE *_fat32_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr);

#endif /* _FAT32_FILE */


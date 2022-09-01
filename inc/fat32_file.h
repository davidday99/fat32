#ifndef _FAT32_FILE_
#define _FAT32_FILE_

#include "fat32_internal.h"

#define FAT32_MAX_FILENAME_LEN 11

typedef struct _fat32_file {
    char name[FAT32_MAX_FILENAME_LEN + 1];
    uint32_t offset;
    uint32_t size;
    uint32_t entry_clus;
    uint32_t entry_clus_offset;
    uint32_t clus_base;
    uint32_t clus_curr;
    uint16_t clus_offset;
    uint32_t is_dir;
    FAT32_FS *_fs;
} FAT32_FILE;

FAT32_FILE *file_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr);
uint32_t file_read(FAT32_FILE *fptr, void *buf, uint32_t count);
FAT32_FILE *file_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr);
DIR_ENTRY *read_dir_entry(FAT32_FILE* fptr, DIR_ENTRY *dir_ent);
DIR_ENTRY *file_contains_dir_entry(FAT32_FILE *fptr, char *name, DIR_ENTRY *dir_ent);
FAT32_FILE *file_creat(FAT32_FS *fs, char *path, FAT32_FILE *fptr);

#endif /* _FAT32_FILE */


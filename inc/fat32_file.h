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
    uint32_t is_dir;
    FAT32_FS *_fs;
} FAT32_FILE;

enum FILE_OPTIONS {
    O_CREAT = 1,
    O_DIRECTORY = 2,
    O_RONLY = 4,
    O_WONLY = 8,
    O_RW = 16,
};

enum DIR_ENTRY_ATTRS {
    ATTR_READ_ONLY = 1,
    ATTR_HIDDEN = 2,
    ATTR_SYSTEM = 4,
    ATTR_VOLUME_ID = 8,
    ATTR_DIRECTORY = 16,
    ATTR_ARCHIVE = 32,
    ATTR_LONG_NAME = 64
};

FAT32_FILE *file_open(FAT32_FS *fs, char *path, enum FILE_OPTIONS flags, FAT32_FILE *fptr);
FAT32_FILE *file_creat(FAT32_FS *fs, char *path, FAT32_FILE *fptr);
uint32_t file_read(FAT32_FILE *fptr, void *buf, uint32_t count);
uint32_t file_write(FAT32_FILE *fptr, const void *buf, uint32_t count);
DIR_ENTRY *read_dir_entry(FAT32_FILE* fptr, DIR_ENTRY *dir_ent);
DIR_ENTRY *file_contains_dir_entry(FAT32_FILE *fptr, char *name, DIR_ENTRY *dir_ent);

#endif /* _FAT32_FILE */


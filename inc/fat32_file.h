#ifndef _FAT32_FILE_
#define _FAT32_FILE_

#define FAT32_MAX_FILENAME_LEN 12

typedef struct _fat32_file {
    char name[FAT32_MAX_FILENAME_LEN];
    uint32_t offset;
    uint32_t size;
} FAT32_FILE;

FAT32_FILE *_fat32_open(char *path, FAT32_FILE *fptr);

#endif /* _FAT32_FILE */


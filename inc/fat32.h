#ifndef _FAT32_H_
#define _FAT32_H_

#include <stdint.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_dir.h"

void format_fat32(FAT32_FS *fs);
void init_fat32_fs(FAT32_FS *fs);
void deinit_fat32_fs(FAT32_FS *fs);
uint8_t fat32_fs_valid(void);
FAT32_FILE *fat32_open(char *path, FAT32_FILE *fptr);
void fat32_close(FAT32_FILE *fptr);
uint32_t fat32_read(FAT32_FILE *fptr);
uint32_t fat32_write(FAT32_FILE *fptr);

#endif /* _FAT32_H_ */

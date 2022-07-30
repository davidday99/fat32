#ifndef _FAT32_H_
#define _FAT32_H_

#include "eeprom.h"
#include "fat32_file.h"

void format_fat32(struct EEPROM *mem);
void init_fat32_fs(struct EEPROM *mem);
uint8_t fat32_fs_valid(void);
FAT32_FILE *fat32_open(char *path, FAT32_FILE *fptr);

#endif /* _FAT32_H_ */

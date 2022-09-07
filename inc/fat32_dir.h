#ifndef _FAT32_DIR_H_
#define _FAT32_DIR_H_

#include "fat32_file.h"

typedef struct _fat32_dir {
    char name[FAT32_MAX_FILENAME_LEN];
    FAT32_FILE _fprops; 
} FAT32_DIR;

FAT32_DIR *dir_open(FAT32_FS *fs, char *path, enum FILE_OPTIONS flags, FAT32_DIR *dptr);

#endif /* _FAT32_DIR_H_ */


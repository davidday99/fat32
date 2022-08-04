#ifndef _FAT32_DIR_H_
#define _FAT32_DIR_H_

#include "fat32_file.h"

typedef struct _fat32_dir {
    char name[FAT32_MAX_FILENAME_LEN];
    FAT32_FILE _fprops; 
} FAT32_DIR;

#endif /* _FAT32_DIR_H_ */


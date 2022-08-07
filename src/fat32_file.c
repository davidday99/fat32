#include <stdio.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"

#define ATTR_READ_ONLY_MASK 0x01
#define ATTR_READ_ONLY_SHIFT 0
#define ATTR_HIDDEN_MASK 0x02
#define ATTR_HIDDEN_SHIFT 1
#define ATTR_SYSTEM_MASK 0x04
#define ATTR_SYSTEM_SHIFT 2
#define ATTR_VOLUME_ID_MASK 0x08
#define ATTR_VOLUME_ID_SHIFT 3
#define ATTR_DIRECTORY_MASK 0x10
#define ATTR_DIRECTORY_SHIFT 4
#define ATTR_ARCHIVE_MASK 0x20
#define ATTR_ARCHIVE_SHIFT 5
#define ATTR_LONG_NAME ATTR_READ_ONLY_MASK | \
                         ATTR_HIDDEN_MASK | \
                         ATTR_SYSTEM_MASK | \
                         ATTR_VOLUME_ID_MASK

#define IS_DIRECTORY(attr) ((attr & ATTR_DIRECTORY_MASK) >> ATTR_DIRECTORY_SHIFT)

FAT32_FILE *_fat32_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr) {
   return NULL;
}


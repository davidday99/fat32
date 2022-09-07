#include <stdint.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_dir.h"

FAT32_DIR *dir_open(FAT32_FS *fs, char *path, enum FILE_OPTIONS flags, FAT32_DIR *dptr) {
    FAT32_FILE f;
    FAT32_FILE *fptr = &f;
    fptr = file_open(fs, path, O_DIRECTORY | flags, fptr);
    if (fptr == NULL || !fptr->is_dir)
        return NULL;
    dptr->_fprops = f;
    strcpy(dptr->name, fptr->name);
    return dptr;
}


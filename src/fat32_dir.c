#include <stdint.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_dir.h"

FAT32_DIR *fat32_opendir(FAT32_FS *fs, char *path, FAT32_DIR *dptr) {
    return 0;
}

FAT32_DIR *fat32_fopendir(FAT32_FS *fs, FAT32_FILE *fptr, FAT32_DIR *dptr) {
    //check that fptr is a directory
    if (!fptr->is_dir) {
        return 0;
    }
    strcpy(dptr->_fprops.name, fptr->name);
    dptr->_fprops.offset = 0;
    dptr->_fprops.clus_offset = 0;
    dptr->_fprops.entry_addr = fptr->entry_addr;
    dptr->_fprops.clus_base = fptr->clus_base;
    dptr->_fprops.clus_curr = fptr->clus_curr;
    return dptr;
}


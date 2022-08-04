#include <stdio.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_data.h"

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
#define PAST_CLUSTER_BOUND(offset, clus_sz) (clus_sz >= offset)

static FAT32_FILE *get_root(FAT32_FS *fs, FAT32_FILE *fptr);

FAT32_FILE *_fat32_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr) {
    char buf[255];
    strcpy(buf, path);
    char *part = strtok(buf, "/");
    FAT32_FILE file;
    FAT32_FILE *fp = get_root(fs, &file);  
    while (part != NULL) {
        printf("%s\n", part);
        part = strtok(NULL, "/");
    } 
    return NULL;
}

static FAT32_FILE *get_root(FAT32_FS *fs, FAT32_FILE *fptr) {
    uint32_t root_clus = fs->bootsec.params.bpb.root_clus;
    strcpy(fptr->name, "/");
    fptr->offset = 0;
    fptr->clus_offset = 0;
    fptr->size = 0;
    fptr->entry_addr = 0;
    fptr->clus_base = root_clus;
    fptr->clus_curr = root_clus;
    fptr->is_dir = 1;
    return fptr;
} 

static FAT32_FILE *get_file_from_entry(FAT32_FS *fs, FAT32_FILE *fptr, DIR_ENTRY *dirent) {
    if (dirent == NULL) {
        return NULL;
    } 
    memcpy(fptr->name, dirent->fields.name, FAT32_MAX_FILENAME_LEN);
    fptr->name[FAT32_MAX_FILENAME_LEN] = '\0';
    fptr->offset = 0;
    fptr->clus_offset = 0;
    fptr->size = dirent->fields.file_size;
    fptr->entry_addr = 0;
    fptr->clus_base = ((dirent->fields.fst_clus_hi << 16) & 0xFFFF0000) | (dirent->fields.fst_clus_lo & 0xFFFF);
    fptr->clus_curr = fptr->clus_base;
    fptr->is_dir = IS_DIRECTORY(dirent->fields.attr); 
}

static DIR_ENTRY *get_next_dir_entry(FAT32_FS *fs, FAT32_FILE *fptr, DIR_ENTRY *dirent) { 
    uint32_t clus_sz = fs->bootsec.params.bpb.bytes_per_sec*fs->bootsec.params.bpb.sec_per_clus;
    uint32_t addr = get_data_region_base_addr(&fs->bootsec) + 
                    (clus_sz*fptr->clus_curr) + fptr->clus_offset;
    for (uint8_t i = 0; i < sizeof(DIR_ENTRY); i++) {
        dirent->bytes[i] = fs->mem->read8(addr);
    }
    fptr->offset += sizeof(DIR_ENTRY);
    fptr->clus_offset += sizeof(DIR_ENTRY);

    //if (PAST_CLUSTER_BOUND(fptr->clus_offset, clus_sz)) {
    //    move_current_cluster_forward(fs, fptr);
    //}
}






#include <stdio.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_fs.h"

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

#define MIN(x, y) (x <= y ? x : y)
#define MAX(x, y) (x >= y ? x : y)

static uint8_t get_next_cluster(FAT32_FILE *fptr);
static FAT32_FILE *get_root_fptr(FAT32_FS *fs, FAT32_FILE *fptr);
static FAT32_FILE *seek_dir_open_entry(FAT32_FILE *dptr);
static DIR_ENTRY *write_dir_entry(FAT32_FILE *dptr, DIR_ENTRY *dir_ent);
static FAT32_FILE *get_file_inside_dir(FAT32_FILE *dptr, char *name, FAT32_FILE *fptr);
static FAT32_FILE *get_fptr_from_dir_entry(DIR_ENTRY *e, uint32_t clus, uint32_t clus_offset, FAT32_FILE *fptr);

uint32_t file_write(FAT32_FILE *fptr, const void *buf, uint32_t count) {
    uint32_t i = 0;
    while (i < count) { 
        if (fptr->clus_offset >= CLUSTER_SIZE(fptr->_fs)) 
            if (get_next_cluster(fptr) == 0)
                break;
        uint32_t write_sz = MIN(CLUSTER_SIZE(fptr->_fs) - fptr->clus_offset, count - i); 
        fs_write_cluster(fptr->_fs,
                            fptr->clus_curr,
                            fptr->clus_offset,
                            buf + i,
                            write_sz);      
        fptr->clus_base += write_sz;
        fptr->offset += write_sz;
        fptr->size += write_sz;
        i += write_sz;
    }
    return i;
}

uint32_t file_read(FAT32_FILE *fptr, void *buf, uint32_t count) {
    uint32_t i = 0;
    uint32_t read_max = MIN(count, fptr->size - fptr->offset);
    while (i < read_max) { 
        if (fptr->clus_offset >= CLUSTER_SIZE(fptr->_fs)) 
             if (get_next_cluster(fptr) == 0)
                break; 
        uint32_t read_sz = MIN(CLUSTER_SIZE(fptr->_fs) - fptr->clus_offset, read_max - i); 
        fs_read_cluster(fptr->_fs, 
                        fptr->clus_curr,
                        fptr->clus_offset,
                        buf + i,
                        read_sz);      
        fptr->clus_base += read_sz;
        fptr->offset += read_sz;
        fptr->size += read_sz;
        i += read_sz;
    }
    return i;
}

FAT32_FILE *file_open(FAT32_FS *fs, char *path, FAT32_FILE *fptr) {
    char buf[256];
    strcpy(buf, path);
    FAT32_FILE curr;
    FAT32_FILE *curr_ptr = get_root_fptr(fs, &curr);
    char *part;
    for (part = strtok(buf, "/"); part !=  NULL; part = strtok(NULL, "/")) {
        printf("%s\n", part);
        if ((curr_ptr = get_file_inside_dir(curr_ptr, curr_ptr->name, curr_ptr)) == NULL) {
               break; 
        }
    }
    if (curr_ptr != NULL) {
        fptr->clus_base = curr_ptr->clus_base;
        fptr->clus_curr = curr_ptr->clus_curr;
        fptr->clus_offset = curr_ptr->clus_offset;
        fptr->entry_clus = curr_ptr->entry_clus;
        fptr->entry_clus_offset = curr_ptr->entry_clus_offset;
        fptr->is_dir = curr_ptr->is_dir;
        memcpy(fptr->name, curr_ptr->name, FAT32_MAX_FILENAME_LEN);
        fptr->offset = curr_ptr->offset;
        fptr->size = curr_ptr->size;
        fptr->_fs = fs;
        curr_ptr = fptr;
    } else if ((part = strtok(NULL, "/")) == NULL) {
        curr_ptr = file_creat(fs, path, fptr); 
    }
    return curr_ptr;
}

FAT32_FILE *file_creat(FAT32_FS *fs, char *path, FAT32_FILE *fptr) {
    char buf[256];
    strcpy(buf, path);
    uint8_t len = strlen(path);
    char *fname = NULL;
    char *parent_path = buf;
    for (int8_t i = len; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i] = '\0';
            fname = buf + i + 1;
        }
    } 
    FAT32_FILE parent;
    FAT32_FILE *parent_ptr = file_open(fs, parent_path, &parent);
    if (parent_ptr == NULL || !parent_ptr->is_dir)  // parent dir doesn't exist or isn't a dir
        return NULL;

    DIR_ENTRY e;
    uint32_t base_clus = fs_get_free_fat_entry(fs);
    if (base_clus == 0)
        return NULL;

    e.fields.fst_clus_hi = (base_clus >> 16) & 0xFFFF;
    e.fields.fst_clus_lo = base_clus & 0xFFFF;
    memcpy(e.fields.name, fname, FAT32_MAX_FILENAME_LEN);
    seek_dir_open_entry(parent_ptr);
    write_dir_entry(parent_ptr, &e);

    fptr = get_fptr_from_dir_entry(&e, parent_ptr->clus_curr, parent_ptr->clus_offset, fptr); 
    fptr->_fs = fs;
    return fptr;
}

static uint8_t get_next_cluster(FAT32_FILE *fptr) {
    uint32_t next_clus = fs_read_fat_entry(fptr->_fs, 0, fptr->clus_curr);
    if (next_clus == END_OF_CLUSTERCHAIN) {
        next_clus = fs_get_free_fat_entry(fptr->_fs);
        if (next_clus != 0) {
            fs_write_fat_entry(fptr->_fs, 0, fptr->clus_curr, next_clus);
            fs_write_fat_entry(fptr->_fs, 0, next_clus, END_OF_CLUSTERCHAIN);
        } else {
            return 0;  // no free clusters to allocate 
        }
    }
    fptr->clus_curr = next_clus;
    fptr->clus_offset = 0;
    return 1;
}

static FAT32_FILE *get_root_fptr(FAT32_FS *fs, FAT32_FILE *fptr) {
    fptr->clus_base = fs->bootsec.params.bpb.root_clus;
    fptr->clus_curr = fptr->clus_base;
    fptr->clus_offset = 0;
    fptr->entry_clus = 0;
    fptr->entry_clus_offset = 0;
    fptr->is_dir = 1;
    memcpy(fptr->name, "/          ",  FAT32_MAX_FILENAME_LEN);
    fptr->offset = 0;
    fptr->_fs = fs; 
    return fptr; 
} 

DIR_ENTRY *read_dir_entry(FAT32_FILE* fptr, DIR_ENTRY *dir_ent) {
    file_read(fptr, dir_ent->bytes, sizeof(DIR_ENTRY)); 
    return dir_ent;
}

static FAT32_FILE *get_fptr_from_dir_entry(DIR_ENTRY *e, uint32_t clus, uint32_t clus_offset, FAT32_FILE *fptr) {
    fptr->clus_base = ((e->fields.fst_clus_hi << 8) & 0xFF00) | (e->fields.fst_clus_lo & 0xFF);
    fptr->clus_curr = fptr->clus_base;
    fptr->clus_offset = 0;
    fptr->entry_clus = clus;
    fptr->entry_clus_offset = clus_offset;
    fptr->is_dir = IS_DIRECTORY(e->fields.attr);
    memcpy(fptr->name, e->fields.name, FAT32_MAX_FILENAME_LEN);
    fptr->offset = 0;
    fptr->size = e->fields.file_size; 
    fptr->_fs = NULL;
    return fptr; 
}

static FAT32_FILE *seek_dir_open_entry(FAT32_FILE *dptr) {
    DIR_ENTRY e;
    for (read_dir_entry(dptr, &e); e.bytes[0] != 0; read_dir_entry(dptr, &e))
        ; 
    return dptr;
}

static DIR_ENTRY *write_dir_entry(FAT32_FILE *dptr, DIR_ENTRY *dir_ent) {
    file_write(dptr, dir_ent->bytes, sizeof(DIR_ENTRY));
    return dir_ent;
}


static FAT32_FILE *get_file_inside_dir(FAT32_FILE *dptr, char *name, FAT32_FILE *fptr) {
    DIR_ENTRY e;
    FAT32_FILE *success = NULL;
    // save dptr offsets so they can be restored
    uint32_t offset = dptr->offset;
    uint32_t clus_curr = dptr->clus_curr;
    uint16_t clus_offset = dptr->clus_offset;

    uint32_t entry_clus = dptr->clus_curr;
    uint32_t entry_clus_offset = dptr->clus_offset;
    for (read_dir_entry(dptr, &e); e.bytes[0] != 0; read_dir_entry(dptr, &e)) {
        if (memcmp(e.fields.name, name, FAT32_MAX_FILENAME_LEN) == 0) {
            fptr = get_fptr_from_dir_entry(&e, entry_clus, entry_clus_offset, fptr);
            success = fptr; 
            break;
        } 
        entry_clus = dptr->clus_curr;
        entry_clus_offset = dptr->clus_offset;
    } 
    dptr->offset = offset;
    dptr->clus_curr = clus_curr;
    dptr->clus_offset = clus_offset;

    return success;
}


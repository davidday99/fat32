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

// Doesn't seem to be declared in the header file, so declaring here.
char *strtok_r(char *str, const char *delim, char **saveptr);

static uint8_t get_next_cluster(FAT32_FILE *fptr);
static FAT32_FILE *get_root_fptr(FAT32_FS *fs, FAT32_FILE *fptr);
static FAT32_FILE *seek_dir_open_entry(FAT32_FILE *dptr);
static enum DIR_ENTRY_ATTRS flags_to_dir_entry_attrs(enum FILE_OPTIONS flags);
static DIR_ENTRY *write_dir_entry(FAT32_FILE *dptr, DIR_ENTRY *dir_ent);
static FAT32_FILE *get_fptr_inside_dir(FAT32_FILE *dptr, char *name, FAT32_FILE *fptr);
static FAT32_FILE *get_fptr_from_dir_entry(DIR_ENTRY *e, FAT32_FILE *dptr, FAT32_FILE *fptr);
static char *filename_to_dir_entry_name(const char *fname, char *buf);
static char *dir_entry_name_to_filename(const char *entry_name, char *buf);
static FAT32_FILE *insert_dir_entry_and_return_fptr(FAT32_FILE *dptr, 
                                                    char *name, 
                                                    enum FILE_OPTIONS flags, 
                                                    FAT32_FILE *fptr);

uint32_t file_read(FAT32_FILE *fptr, void *buf, uint32_t count) {
    uint32_t read_max;
    if (fptr->is_dir) 
       read_max = count; 
    else
       read_max =  MIN(count, fptr->size - fptr->offset);
    uint16_t clus_sz = fs_get_cluster_size(fptr->_fs);
    uint16_t clus_off = fptr->offset % clus_sz; 
    uint32_t i = 0;
    while (i < read_max) { 
        if ((clus_off >= clus_sz) || (clus_off == 0 && fptr->offset > 0)) {
            if (get_next_cluster(fptr) == 0)
                break;
            else
                clus_off = 0;
        }
        uint32_t read_sz = MIN(clus_sz - clus_off, read_max - i); 
        fs_read_cluster(fptr->_fs, 
                        fptr->clus_curr,
                        clus_off,
                        buf + i,
                        read_sz);      
        
        if (fptr->offset >= fptr->size)
            fptr->size += read_sz;
        fptr->offset += read_sz;
        clus_off += read_sz;
        i += read_sz;
    }
    return i;
}

uint32_t file_write(FAT32_FILE *fptr, const void *buf, uint32_t count) {
    uint16_t clus_sz = fs_get_cluster_size(fptr->_fs);
    uint16_t clus_off = fptr->offset % clus_sz; 
    uint32_t i = 0;
    while (i < count) { 
        if ((clus_off >= clus_sz) || (clus_off == 0 && fptr->offset > 0)) {
            if (get_next_cluster(fptr) == 0)
                break;
            else
                clus_off = 0;
        }
        uint32_t write_sz = MIN(clus_sz - clus_off, count - i); 
        fs_write_cluster(fptr->_fs,
                            fptr->clus_curr,
                            clus_off,
                            buf + i,
                            write_sz);      
        if (fptr->offset >= fptr->size)
            fptr->size += write_sz;
        fptr->offset += write_sz;
        clus_off += write_sz;
        i += write_sz;
    }
    return i;
}

FAT32_FILE *file_open(FAT32_FS *fs, char *path, enum FILE_OPTIONS flags, FAT32_FILE *fptr) {
    char buf[256];
    char *saveptr;
    strcpy(buf, path);
    FAT32_FILE parent, child;
    FAT32_FILE *cptr = get_root_fptr(fs, &child);
    FAT32_FILE *pptr = get_root_fptr(fs, &parent);
    char *seg;
    for (seg = strtok_r(buf, "/", &saveptr); seg !=  NULL; seg = strtok_r(NULL, "/", &saveptr)) {
        printf("%s\n", seg);
        parent = child;
        if ((cptr = get_fptr_inside_dir(pptr, seg, cptr)) == NULL || !pptr->is_dir) {
               break; 
        }
    }
    if (cptr != NULL) { 
        *fptr = *cptr;
        cptr = fptr;
    } else if (cptr == NULL &&
                strtok_r(NULL, "/", &saveptr) == NULL && 
                (FILE_CREATE_FLAG_SET(flags))) {
        fptr = insert_dir_entry_and_return_fptr(pptr, seg, flags, fptr);   
        cptr = fptr;
    } 
    return cptr;
}

FAT32_FILE *file_creat(FAT32_FS *fs, char *path, FAT32_FILE *fptr) {
    return file_open(fs, path, O_CREAT, fptr);
}
    
static uint8_t get_next_cluster(FAT32_FILE *fptr) {
    uint32_t next_clus = fs_read_fat_entry(fptr->_fs, 0, fptr->clus_curr);
    if (next_clus == END_OF_CLUSTERCHAIN) {
        next_clus = fs_get_free_fat_entry(fptr->_fs);
        if (next_clus != FREE_CLUSTER) {
            fs_write_fat_entry(fptr->_fs, 0, fptr->clus_curr, next_clus);
            fs_write_fat_entry(fptr->_fs, 0, next_clus, END_OF_CLUSTERCHAIN);
        } else {
            return 0;  // no free clusters to allocate 
        }
    }
    fptr->clus_curr = next_clus;
    return 1;
}

static FAT32_FILE *get_root_fptr(FAT32_FS *fs, FAT32_FILE *fptr) {
    fptr->clus_base = fs->bootsec.params.bpb.root_clus;
    fptr->clus_curr = fptr->clus_base;
    fptr->entry_clus = 0;
    fptr->entry_clus_offset = 0;
    fptr->is_dir = 1;
    fptr->offset = 0;
    fptr->_fs = fs; 
    strcpy(fptr->name, "/");
    return fptr; 
} 

static FAT32_FILE *insert_dir_entry_and_return_fptr(FAT32_FILE *dptr, char *name, enum FILE_OPTIONS flags, FAT32_FILE *fptr) {
    uint32_t cluster = fs_get_free_fat_entry(dptr->_fs);
    if (cluster == 0)
        return NULL;
    fs_write_fat_entry(dptr->_fs, 0, cluster, END_OF_CLUSTERCHAIN);
    uint32_t tmp_off = dptr->offset;
    uint32_t tmp_clus = dptr->clus_curr;
    seek_dir_open_entry(dptr); 
    DIR_ENTRY e;
    memset(e.bytes, 0, sizeof(DIR_ENTRY));
    e.fields.attr = flags_to_dir_entry_attrs(flags);
    e.fields.fst_clus_hi = (cluster >> 16) & 0xFFFF;
    e.fields.fst_clus_lo = cluster & 0xFFFF;
    filename_to_dir_entry_name(name, (char *) e.fields.name);
    fptr = get_fptr_from_dir_entry(&e, dptr, fptr); 
    write_dir_entry(dptr, &e);
    dptr->offset = tmp_off;
    dptr->clus_curr = tmp_clus;
    return fptr;
}

DIR_ENTRY *read_dir_entry(FAT32_FILE* dptr, DIR_ENTRY *dir_ent) {
    file_read(dptr, dir_ent->bytes, sizeof(DIR_ENTRY)); 
    return dir_ent;
}

static DIR_ENTRY *write_dir_entry(FAT32_FILE *dptr, DIR_ENTRY *dir_ent) {
    file_write(dptr, dir_ent->bytes, sizeof(DIR_ENTRY));
    return dir_ent;
}

static FAT32_FILE *seek_dir_open_entry(FAT32_FILE *dptr) {
    DIR_ENTRY e;
    memset(e.bytes, 0, sizeof(DIR_ENTRY));
    uint32_t prev_clus = dptr->clus_curr;
    uint32_t prev_off = dptr->offset;
    for (read_dir_entry(dptr, &e);
            e.bytes[0] != 0 && e.bytes[0] != 0xE5; 
            read_dir_entry(dptr, &e)) {
        prev_clus = dptr->clus_curr;
        prev_off = dptr->offset;
    }
    dptr->clus_curr = prev_clus;
    dptr->offset = prev_off; 
    return dptr;
}

static FAT32_FILE *get_fptr_from_dir_entry(DIR_ENTRY *e, FAT32_FILE *dptr, FAT32_FILE *fptr) {
    uint32_t clus_sz = fs_get_cluster_size(dptr->_fs);
    fptr->clus_base = ((e->fields.fst_clus_hi << 8) & 0xFF00) | (e->fields.fst_clus_lo & 0xFF);
    fptr->clus_curr = fptr->clus_base;
    fptr->entry_clus = dptr->clus_curr;
    fptr->entry_clus_offset = dptr->offset % clus_sz;
    fptr->is_dir = IS_DIRECTORY(e->fields.attr);
    dir_entry_name_to_filename((char *) e->fields.name, fptr->name);
    fptr->offset = 0;
    fptr->size = e->fields.file_size; 
    fptr->_fs = dptr->_fs;
    return fptr; 
}

static FAT32_FILE *get_fptr_inside_dir(FAT32_FILE *dptr, char *name, FAT32_FILE *fptr) {
    DIR_ENTRY e;
    memset(e.bytes, 0, sizeof(e));
    char dir_entry_name[FAT32_MAX_FILENAME_LEN + 1];
    filename_to_dir_entry_name(name, dir_entry_name);
    FAT32_FILE *success = NULL;
    uint16_t clus_sz = fs_get_cluster_size(dptr->_fs);
    // save dptr offsets so they can be restored
    uint32_t offset = dptr->offset;
    uint32_t clus_curr = dptr->clus_curr;

    uint32_t entry_clus = dptr->clus_curr;
    uint32_t entry_clus_offset = offset % clus_sz;
    for (read_dir_entry(dptr, &e); e.bytes[0] != 0; read_dir_entry(dptr, &e)) {
        if (memcmp(e.fields.name, dir_entry_name, FAT32_MAX_FILENAME_LEN) == 0) {
            fptr->_fs = dptr->_fs;
            fptr = get_fptr_from_dir_entry(&e, dptr, fptr);
            fptr->entry_clus = entry_clus;
            fptr->entry_clus_offset = entry_clus_offset;
            success = fptr; 
            break;
        } 
        entry_clus = dptr->clus_curr;
        entry_clus_offset = dptr->offset % clus_sz;
    } 
    dptr->offset = offset;
    dptr->clus_curr = clus_curr;
    return success;
}

static enum DIR_ENTRY_ATTRS flags_to_dir_entry_attrs(enum FILE_OPTIONS flags) {
    enum DIR_ENTRY_ATTRS attrs = 0;
    if (flags & O_DIRECTORY)
        attrs |= ATTR_DIRECTORY;
    if (flags & O_RONLY)
        attrs |= ATTR_READ_ONLY;
    return attrs;
}

static char *filename_to_dir_entry_name(const char *fname, char *buf) {
    char temp[FAT32_MAX_FILENAME_LEN + 1];
    char *saveptr;
    strcpy(temp, fname);
    char *name; 
    char *ext;
    name = strtok_r(temp, ".", &saveptr);
    ext = strtok_r(NULL, ".", &saveptr);
    uint8_t name_len = name != NULL ? strlen(name): 0;
    uint8_t ext_len = ext != NULL ? strlen(ext) : 0;
    uint8_t ws_len = FAT32_MAX_FILENAME_LEN - name_len - ext_len;
    memcpy(buf, name, name_len);
    
    for (uint8_t i = 0; i < ws_len; i++)
        buf[name_len + i] = ' ';

    memcpy(buf + name_len + ws_len, ext, ext_len);
    return buf;
}

static char *dir_entry_name_to_filename(const char *entry_name, char *buf) {
    char temp[FAT32_MAX_FILENAME_LEN];
    char *saveptr;
    memcpy(temp, entry_name, FAT32_MAX_FILENAME_LEN);
    char *name; 
    char *ext;
    name = strtok_r(temp, " ", &saveptr);
    ext = strtok_r(NULL, " ", &saveptr);
    uint8_t name_len = name != NULL ? strlen(name): 0;
    uint8_t ext_len = ext != NULL ? strlen(ext) : 0;
    memcpy(buf, name, name_len);
    if (ext_len != 0)
        buf[name_len] = '.';
    memcpy(buf + name_len + 1, ext, ext_len); 
    buf[name_len + ext_len] = '\0'; 
    return buf;
}


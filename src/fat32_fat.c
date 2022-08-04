#include "fat32_internal.h"
#include "fat32_fat.h"

uint32_t get_fat_structure_base_addr(BOOT_SECTOR *sec) {
    uint32_t sectors = sec->params.bpb.reserved_sec_cnt;
    uint32_t fat_base = sectors*sec->params.bpb.bytes_per_sec;
    return fat_base;
}


uint32_t get_fat_entry_n_addr(BOOT_SECTOR *sec, uint32_t n) {
    uint32_t fat_base = get_fat_structure_base_addr(sec);
    uint32_t offset = n*sizeof(uint32_t); 
    return fat_base + offset;
}

uint32_t get_fat_entry_n(FAT32_FS *fs, uint32_t n) {
    uint32_t entry_addr = get_fat_entry_n_addr(&fs->bootsec, n);
    uint32_t entry = fs->mem->read32(entry_addr);
    return entry;
}

uint32_t get_next_free_fat_entry(FAT32_FS *fs) {
    uint32_t entry = BAD_CLUSTER;
    uint32_t fat_size = fs->bootsec.params.bpb.fat_sz_32*fs->bootsec.params.bpb.bytes_per_sec;
    uint32_t index;
    for (index = 2; index < fat_size/sizeof(uint32_t); index++) {
        uint32_t e = get_fat_entry_n(fs, index);
        if (e == FREE_CLUSTER) {
            entry = e;
            break;
        } 
    }
    return entry;
}


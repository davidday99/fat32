#include <stdint.h>
#include "fat32_internal.h"

uint32_t get_data_region_base_addr(BOOT_SECTOR *sec) {
    uint32_t fat_base_addr = get_fat_structure_base_addr(sec);
    uint32_t data_base = fat_base_addr + (sec->params.bpb.num_fats*
                                             sec->params.bpb.fat_sz_32*
                                             sec->params.bpb.bytes_per_sec); 
    return data_base;
}



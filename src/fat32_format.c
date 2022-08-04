#include <stdint.h>
#include <string.h>
#include "eeprom.h"
#include "fat32_internal.h"
#include "fat32_fat.h"
#include "fat32_data.h"

static BOOT_SECTOR *init_bpb(struct EEPROM *mem, BOOT_SECTOR *sec);
static void init_fat_structure(struct EEPROM *mem, BOOT_SECTOR *sec);
static void clear_data_region(struct EEPROM *mem, BOOT_SECTOR *sec);

void _format_fat32(struct EEPROM *mem) {
    BOOT_SECTOR boot;
    mem->erase();
    init_bpb(mem, &boot);
    init_fat_structure(mem, &boot);
    clear_data_region(mem, &boot);
    return;
}

static BOOT_SECTOR *init_bpb(struct EEPROM *mem, BOOT_SECTOR *sec) {
    sec->params.bpb.bytes_per_sec = 512;
    sec->params.bpb.sec_per_clus = 1;
    sec->params.bpb.reserved_sec_cnt = 32;
    sec->params.bpb.num_fats = 1;
    sec->params.bpb.root_ent_cnt = 0;
    sec->params.bpb.tot_sec_16 = 0;
    sec->params.bpb.media = MEDIA_NONREMOVABLE;
    sec->params.bpb.fat_sz_16 = 0;
    sec->params.bpb.sec_per_trk = 0;
    sec->params.bpb.num_heads = 0;
    sec->params.bpb.hidd_sec_cnt = 0;
    sec->params.bpb.tot_sec_32 = mem->size() / 512;
    sec->params.bpb.fat_sz_32 = 2;
    sec->params.bpb.ext_flags = 0x0010;
    sec->params.bpb.fs_ver = 0;
    sec->params.bpb.root_clus = 2;
    sec->params.bpb.fs_info = 1;
    sec->params.bpb.bk_boot_sec = 0;
    sec->params.drv_num = 0x80;
    sec->params.reserved1 = 0;
    sec->params.boot_sig = BOOTSIG;
    sec->params.vol_id = 0;
    memcpy(sec->params.oem_name, "MSWIN4.1", 8);
    memcpy(sec->params.vol_lab, "NO NAME    ", 11);
    memcpy(sec->params.fil_sys_type, "FAT     ", 8);
    sec->bytes[SECTOR_SECOND_TO_LAST_BYTE] = SECTORSIG_LO;
    sec->bytes[SECTOR_LAST_BYTE] = SECTORSIG_HI;

    for (uint16_t addr = 0; addr < sizeof(BOOT_SECTOR); addr++) {
	mem->write8(sec->bytes[addr], addr);
    }

    return sec;
}

static void init_fat_structure(struct EEPROM *mem, BOOT_SECTOR *sec) {
    uint32_t fat_base = get_fat_structure_base_addr(sec);
    uint16_t fat_size = sec->params.bpb.fat_sz_32*sec->params.bpb.bytes_per_sec;

    for (uint32_t addr = fat_base; addr < fat_base + fat_size; addr += 4) {
	    mem->write32(0, addr);     
    }
}


static void clear_data_region(struct EEPROM *mem, BOOT_SECTOR *sec) {
    uint32_t totsec = sec->params.bpb.tot_sec_32;
    uint32_t datasec = totsec - (sec->params.bpb.reserved_sec_cnt + (
                        sec->params.bpb.num_fats*sec->params.bpb.fat_sz_32));
    uint32_t datasz= datasec*sec->params.bpb.bytes_per_sec;
    uint32_t base_addr = get_data_region_base_addr(sec);
    for (uint32_t addr = base_addr; addr < base_addr + datasz; addr += 4) {
        mem->write32(0, addr);
    }
    
}

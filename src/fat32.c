#include <stdint.h>
#include <string.h>
#include "eeprom.h"

#define MEDIA_REMOVABLE 0xF8
#define MEDIA_NONREMOVABLE 0xF0
#define BOOTSIG 0x29

typedef struct __attribute__(( packed )) ios_param_block {
    uint16_t bytes_per_sec;
    uint8_t sec_per_clus;
    uint8_t reserved_sec_cnt;
    uint8_t num_fats;
    uint16_t root_ent_cnt;
    uint16_t tot_sec_16;
    uint8_t media;
    uint16_t fat_sz_16;
    uint16_t sec_per_trk;
    uint16_t num_heads;
    uint32_t hidd_sec_cnt;
    uint32_t tot_sec_32;
    uint32_t fat_sz_32;
    uint16_t ext_flags;
    uint16_t fs_ver;
    uint32_t root_clus;
    uint16_t fs_info;
    uint16_t bk_boot_sec;
    uint8_t reserved[12];
} BPB;

typedef struct __attribute__(( packed )) boot_sector {
    uint8_t jmp_boot[3];
    uint8_t oem_name[8];
    BPB bpb;
    uint8_t drv_num;
    uint8_t reserved1;
    uint8_t boot_sig;
    uint32_t vol_id;
    uint8_t vol_lab[11];
    uint8_t fil_sys_type[8];
} BS;

static void init_bpb(struct EEPROM *mem);

void format_fat32(struct EEPROM *mem) {
    mem->erase();
    init_bpb(mem);
    return;
}

void fat32_open(struct EEPROM *mem, char *name) {
    
}

void fat32_mkdir(struct EEPROM *mem, char *dirname) {

}

void fat32_mkfile(struct EEPROM *mem, char *fname) {

}

void fat32_move(struct EEPROM *mem, char *name) {

}

void fat32_remove(struct EEPROM *mem, char *name) {

}

static void init_bpb(struct EEPROM *mem) {
    BS sector0 = {
        .bpb.bytes_per_sec = 512,
        .bpb.sec_per_clus = 1,
        .bpb.reserved_sec_cnt = 32,
        .bpb.num_fats = 1,
        .bpb.root_ent_cnt = 0,
        .bpb.tot_sec_16 = 0,
        .bpb.media = MEDIA_NONREMOVABLE,
        .bpb.fat_sz_16 = 0,
        .bpb.sec_per_trk = 0,
        .bpb.num_heads = 0,
        .bpb.hidd_sec_cnt = 0,
        .bpb.tot_sec_32 = mem->size() / 512,
        .bpb.fat_sz_32 = 2,
        .bpb.ext_flags = 0x0010,
        .bpb.fs_ver = 0,
        .bpb.root_clus = 2,
        .bpb.fs_info = 1,
        .bpb.bk_boot_sec = 0,
        .drv_num = 0x80,
        .reserved1 = 0,
        .boot_sig = BOOTSIG,
        .vol_id = 0
    };
    
    memcpy(sector0.oem_name, "MSWIN4.1", 8);
    memcpy(sector0.vol_lab, "NO NAME    ", 11);
    memcpy(sector0.fil_sys_type, "FAT     ", 8);

    uint32_t addr = 0;
    for (uint8_t *bufptr = (uint8_t *) &sector0; bufptr - (uint8_t *) &sector0 < sizeof(BS); bufptr++) {
        mem->write(*bufptr, addr++);
    }
}

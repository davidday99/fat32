#include <stdint.h>
#include <string.h>
#include "eeprom.h"

#define MEDIA_REMOVABLE 0xF8
#define MEDIA_NONREMOVABLE 0xF0
#define BOOTSIG 0x29
#define SECTOR_SZ 512
#define SECTORSIG 0xAA55
#define SECTORSIG_LO 0x55
#define SECTORSIG_HI 0xAA

#define VERIFY_SECTORSIG(lo, hi) ((((hi << 8) & 0xFF00) | (lo & 0x00FF)) == SECTORSIG)

typedef struct __attribute__(( packed )) bios_param_block {
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

struct __attribute__(( packed )) boot_sector_parameters {
    uint8_t jmp_boot[3];
    uint8_t oem_name[8];
    BPB bpb;
    uint8_t drv_num;
    uint8_t reserved1;
    uint8_t boot_sig;
    uint32_t vol_id;
    uint8_t vol_lab[11];
    uint8_t fil_sys_type[8];
};

typedef struct __attribute__(( packed )) _dir_entry {
    uint8_t name[11];
    uint8_t attr;
    uint8_t NTRes;
    uint8_t crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t last_access_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;
} DIR_ENTRY;

typedef union boot_sector {
    struct boot_sector_parameters params;
    uint8_t bytes[SECTOR_SZ];
} BOOT_SECTOR;

static struct {
    BOOT_SECTOR params;
    uint8_t valid;
    uint8_t write_cnt;
} FILESYS;

static void init_bpb(struct EEPROM *mem);

void format_fat32(struct EEPROM *mem) {
    mem->erase();
    init_bpb(mem);
    // init FAT
    // sign sector 0 bytes 511, 512 with 0xAA55
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

void init_fat32_fs(struct EEPROM *mem) {
    for (uint16_t i = 0; i < SECTOR_SZ; i++) {
        FILESYS.params.bytes[i] = mem->read(i);
    }
    FILESYS.valid = VERIFY_SECTORSIG(FILESYS.params.bytes[510], FILESYS.params.bytes[511]);
}

uint8_t fat32_fs_valid(void) {
    return FILESYS.valid == 1;
}

static void init_bpb(struct EEPROM *mem) {
    BOOT_SECTOR sector0 = {
        .params.bpb.bytes_per_sec = 512,
        .params.bpb.sec_per_clus = 1,
        .params.bpb.reserved_sec_cnt = 32,
        .params.bpb.num_fats = 1,
        .params.bpb.root_ent_cnt = 0,
        .params.bpb.tot_sec_16 = 0,
        .params.bpb.media = MEDIA_NONREMOVABLE,
        .params.bpb.fat_sz_16 = 0,
        .params.bpb.sec_per_trk = 0,
        .params.bpb.num_heads = 0,
        .params.bpb.hidd_sec_cnt = 0,
        .params.bpb.tot_sec_32 = mem->size() / 512,
        .params.bpb.fat_sz_32 = 2,
        .params.bpb.ext_flags = 0x0010,
        .params.bpb.fs_ver = 0,
        .params.bpb.root_clus = 2,
        .params.bpb.fs_info = 1,
        .params.bpb.bk_boot_sec = 0,
        .params.drv_num = 0x80,
        .params.reserved1 = 0,
        .params.boot_sig = BOOTSIG,
        .params.vol_id = 0
    };
    memcpy(sector0.params.oem_name, "MSWIN4.1", 8);
    memcpy(sector0.params.vol_lab, "NO NAME    ", 11);
    memcpy(sector0.params.fil_sys_type, "FAT     ", 8);
    sector0.bytes[510] = SECTORSIG_LO;
    sector0.bytes[511] = SECTORSIG_HI;

    uint32_t addr = 0;
    for (uint8_t *bufptr = (uint8_t *) &sector0; bufptr - (uint8_t *) &sector0 < sizeof(BOOT_SECTOR); bufptr++) {
        mem->write(*bufptr, addr++);
    }
}

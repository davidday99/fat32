#ifndef _FAT32_INTERNAL_H_
#define _FAT32_INTERNAL_H_

#include <stdint.h>
#include "drive.h"

#define MEDIA_REMOVABLE 0xF8
#define MEDIA_NONREMOVABLE 0xF0
#define BOOTSIG 0x29
#define SECTOR_SZ 512
#define SECTORSIG 0xAA55
#define SECTORSIG_LO 0x55
#define SECTORSIG_HI 0xAA
#define SECTOR_SECOND_TO_LAST_BYTE 510
#define SECTOR_LAST_BYTE 511
#define BAD_CLUSTER 0x0FFFFFF7
#define END_OF_CLUSTERCHAIN 0x0FFFFFF8
#define FREE_CLUSTER 0
#define FAT_ENTRY_ZERO_MEDIA_BITMASK 0x0000000F
#define CLEAN_SHUTDOWN_BITMASK 0x08000000
#define HARD_ERROR_BITMASK 0x04000000

#define VERIFY_SECTORSIG(lo, hi) ((((hi << 8) & 0xFF00) | (lo & 0x00FF)) == SECTORSIG)

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
} dir_ent;

typedef union _DIR_ENTRY {
    dir_ent fields;
    uint8_t bytes[sizeof(dir_ent)];
} DIR_ENTRY;


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

typedef struct __attribute__(( packed )) boot_sector_parameters {
    uint8_t jmp_boot[3];
    uint8_t oem_name[8];
    BPB bpb;
    uint8_t drv_num;
    uint8_t reserved1;
    uint8_t boot_sig;
    uint32_t vol_id;
    uint8_t vol_lab[11];
    uint8_t fil_sys_type[8];
} bootsec_params;

typedef union boot_sector {
    bootsec_params params;
    uint8_t bytes[SECTOR_SZ];
} BOOT_SECTOR;

typedef struct {
    DRIVE *drv;
    BOOT_SECTOR bootsec;
    uint8_t valid;
    uint16_t clus_sz;
    uint32_t max_valid_clus;
} FAT32_FS;

#endif /* _FAT32_INTERNAL_H_ */


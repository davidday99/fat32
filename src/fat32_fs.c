#include <stdint.h>
#include <string.h>
#include "fat32_internal.h" 
#include "fat32_fs.h"
#include "fat32_conf.h"
#include "drive.h"

#define RESERVED_CLUSTER_ENTRY_COUNT 2
#define CLUSTER_SIZE(fs) (fs->bootsec.params.bpb.sec_per_clus* \
                            fs->bootsec.params.bpb.bytes_per_sec)
#define VERIFY_SECTORSIG(lo, hi) ((((hi << 8) & 0xFF00) | (lo & 0x00FF)) == SECTORSIG)
#define INVALID_OFFSET(clus, offset) (clus_off >= CLUSTER_SIZE(fs) ||\
                                        clus >= fs->max_valid_clus ||\
                                        clus < fs->bootsec.params.bpb.root_clus)
#define FAT32_CLUSTER_ENTRY_MASK 0x0FFFFFFF         
#define FAT32_CLUSTER_ENTRY_RESERVED_MASK (~FAT32_CLUSTER_ENTRY_MASK)

static uint32_t get_fat_base_addr(FAT32_FS *fs, uint8_t fatnum);
static uint32_t get_data_base_addr(FAT32_FS *fs);
static uint32_t get_data_offset(FAT32_FS *fs, uint32_t clus); 
static void init_bpb(FAT32_FS *fs);
static void init_fat_structure(FAT32_FS *fs, uint8_t fatnum);
static void clear_data_region(FAT32_FS *fs);
static uint32_t get_data_sector_count(FAT32_FS *fs);
 
uint32_t fs_write_cluster(FAT32_FS *fs,
                            uint32_t clus,
                            uint16_t clus_off, 
                            const void *buf,
                            uint16_t count) {
    uint32_t i = 0;
    if (INVALID_OFFSET(clus, clus_off))
        return i;
    uint32_t d_base = get_data_base_addr(fs);
    uint32_t offset = get_data_offset(fs, clus); 
    uint32_t addr = d_base + offset + clus_off;
    uint32_t max_write_count = CLUSTER_SIZE(fs) - clus_off;
    while (i < count && i < max_write_count) {
        write_drive8(fs->drv, ((uint8_t *) buf)[i++], addr++);
    }
    return i;
}

uint32_t fs_read_cluster(FAT32_FS *fs, 
                            uint32_t clus,
                            uint16_t clus_off,
                            void *buf,
                            uint32_t count) {
    uint32_t i = 0;
    if (INVALID_OFFSET(clus, off))
        return i;
    uint32_t d_base = get_data_base_addr(fs);
    uint32_t offset = get_data_offset(fs, clus); 
    uint32_t addr = d_base + offset + clus_off;
    uint32_t max_write_count = CLUSTER_SIZE(fs) - clus_off;
    while (i < count && i < max_write_count) {
        ((uint8_t *) buf)[i++] = read_drive8(fs->drv, addr++);
    }
    return i;
}

void fs_write_fat_entry(FAT32_FS *fs,
                        uint8_t fatnum,
                        uint32_t n, 
                        uint32_t v) {
    uint32_t addr = get_fat_base_addr(fs, fatnum); 
    uint32_t offset = n*sizeof(uint32_t);
    uint32_t high_nibble = read_drive32(fs->drv, addr + offset) & 
                            FAT32_CLUSTER_ENTRY_RESERVED_MASK;
    uint32_t entry = (v & FAT32_CLUSTER_ENTRY_MASK) | high_nibble;
    write_drive32(fs->drv, entry, addr + offset);
}

uint32_t fs_read_fat_entry(FAT32_FS *fs,
                            uint8_t fatnum,
                            uint32_t n) {
    uint32_t addr = get_fat_base_addr(fs, fatnum);
    uint32_t offset = n*sizeof(uint32_t);
    uint32_t entry = read_drive32(fs->drv, addr + offset) & FAT32_CLUSTER_ENTRY_MASK;
    return entry;
}

uint32_t fs_get_free_fat_entry(FAT32_FS *fs) {
    uint32_t fat_entry_cnt = fs->max_valid_clus;
    uint32_t i = 0; 
    while (i < fat_entry_cnt) {
        uint32_t entry = fs_read_fat_entry(fs, 0, i);
        if (entry == 0)
            return i; 
        i++;
    }
    return 0;
}

void fs_format(FAT32_FS *fs) {
    erase_drive(fs->drv);
    init_bpb(fs);
    for (uint8_t i = 0; i < fs->bootsec.params.bpb.num_fats; i++) 
        init_fat_structure(fs, i);
    clear_data_region(fs);
}

uint32_t fs_init(FAT32_FS *fs) {
    for (uint16_t i = 0; i < SECTOR_SZ; i++) {
        fs->bootsec.bytes[i] = read_drive8(fs->drv, i);
    }
    fs->valid = VERIFY_SECTORSIG(fs->bootsec.bytes[510], fs->bootsec.bytes[511]);
    uint32_t status = fs_read_fat_entry(fs, 0, 1);
    fs_write_fat_entry(fs, 0, 1, status & ~CLEAN_SHUTDOWN_BITMASK);
    fs->max_valid_clus= (get_data_sector_count(fs) / fs->bootsec.params.bpb.sec_per_clus) + 
                                RESERVED_CLUSTER_ENTRY_COUNT;
    return status;
}

void fs_deinit(FAT32_FS *fs) {
    uint32_t status = fs_read_fat_entry(fs, 0, 1);
    uint32_t new_status = status | CLEAN_SHUTDOWN_BITMASK;
    fs_write_fat_entry(fs, 0, 1, new_status);
    fs->valid = 0;
}

uint16_t fs_get_cluster_size(FAT32_FS *fs) {
    return fs->bootsec.params.bpb.sec_per_clus*
            fs->bootsec.params.bpb.bytes_per_sec;
}

__attribute__(( always_inline )) static inline uint32_t get_fat_base_addr(FAT32_FS *fs, uint8_t fatnum) {
    uint32_t sectors = fs->bootsec.params.bpb.reserved_sec_cnt + 
                        fs->bootsec.params.bpb.fat_sz_32*fatnum;
    uint32_t f_base = sectors*fs->bootsec.params.bpb.bytes_per_sec;
    return f_base;
}


__attribute__(( always_inline )) static inline uint32_t get_data_base_addr(FAT32_FS *fs) {
    uint32_t f_base = get_fat_base_addr(fs, 0);
    uint32_t d_base = f_base + (fs->bootsec.params.bpb.num_fats*
                                        fs->bootsec.params.bpb.fat_sz_32*
                                        fs->bootsec.params.bpb.bytes_per_sec); 
    return d_base;
}

__attribute__(( always_inline )) static inline uint32_t get_data_offset(FAT32_FS *fs, uint32_t clus) {
    if (clus < fs->bootsec.params.bpb.root_clus)
        return 0;
    uint32_t offset = CLUSTER_SIZE(fs)*(clus - fs->bootsec.params.bpb.root_clus);    
    return offset;
}

static void init_bpb(FAT32_FS *fs) {
    fs->bootsec.params.bpb.bytes_per_sec = CONF_BYTES_PER_SECTOR;
    fs->bootsec.params.bpb.sec_per_clus = CONF_SECTORS_PER_CLUSTER;
    fs->bootsec.params.bpb.reserved_sec_cnt = CONF_RESERVED_SECTOR_COUNT;
    fs->bootsec.params.bpb.num_fats = CONF_NUM_FATS;
    fs->bootsec.params.bpb.root_ent_cnt = CONF_ROOT_ENTRY_COUNT;
    fs->bootsec.params.bpb.tot_sec_16 = CONF_TOTAL_SECTORS_16;
    fs->bootsec.params.bpb.media = CONF_MEDIA_TYPE;
    fs->bootsec.params.bpb.fat_sz_16 = CONF_FAT_SIZE_16;
    fs->bootsec.params.bpb.sec_per_trk = CONF_SECTORS_PER_TRACK;
    fs->bootsec.params.bpb.num_heads = CONF_NUM_HEADS;
    fs->bootsec.params.bpb.hidd_sec_cnt = CONF_HIDDEN_SECTOR_COUNT;
    fs->bootsec.params.bpb.tot_sec_32 = size_drive(fs->drv) / 512;
    fs->bootsec.params.bpb.fat_sz_32 = CONF_FAT_SIZE_32;
    fs->bootsec.params.bpb.ext_flags = CONF_EXT_FLAGS;
    fs->bootsec.params.bpb.fs_ver = CONF_FS_VERSION;
    fs->bootsec.params.bpb.root_clus = CONF_ROOT_CLUSTER;
    fs->bootsec.params.bpb.fs_info = CONF_FS_INFO;
    fs->bootsec.params.bpb.bk_boot_sec = CONF_BK_BOOT_SECTOR;
    fs->bootsec.params.drv_num = CONF_DRIVE_NUM;
    fs->bootsec.params.reserved1 = 0;
    fs->bootsec.params.boot_sig = BOOTSIG;
    fs->bootsec.params.vol_id = CONF_VOLUME_ID;
    memcpy(fs->bootsec.params.oem_name, CONF_OEM_NAME, 8);
    memcpy(fs->bootsec.params.vol_lab, CONF_VOLUME_LABEL, 11);
    memcpy(fs->bootsec.params.fil_sys_type, CONF_FS_TYPE, 8);
    fs->bootsec.bytes[SECTOR_SECOND_TO_LAST_BYTE] = SECTORSIG_LO;
    fs->bootsec.bytes[SECTOR_LAST_BYTE] = SECTORSIG_HI;

    for (uint16_t addr = 0; addr < sizeof(BOOT_SECTOR); addr++) {
	    write_drive8(fs->drv, fs->bootsec.bytes[addr], addr);
    }
}

static void init_fat_structure(FAT32_FS *fs, uint8_t fatnum) {
    uint32_t fat_base = get_fat_base_addr(fs, fatnum);
    uint16_t fat_size = fs->bootsec.params.bpb.fat_sz_32*fs->bootsec.params.bpb.bytes_per_sec;

    for (uint32_t addr = fat_base; addr < fat_base + fat_size; addr += 4) {
	    write_drive32(fs->drv, 0, addr);     
    }

    fs_write_fat_entry(fs, 0, 0, fs->bootsec.params.bpb.media | ~FAT_ENTRY_ZERO_MEDIA_BITMASK);
    fs_write_fat_entry(fs, 0, 1, CLEAN_SHUTDOWN_BITMASK | HARD_ERROR_BITMASK);
    fs_write_fat_entry(fs, 0, fs->bootsec.params.bpb.root_clus, END_OF_CLUSTERCHAIN);
}

static void clear_data_region(FAT32_FS *fs) {
    uint32_t datasec = get_data_sector_count(fs); 
    uint32_t datasz= datasec*fs->bootsec.params.bpb.bytes_per_sec;
    uint32_t base_addr = get_data_base_addr(fs);
    for (uint32_t addr = base_addr; addr < base_addr + datasz; addr += 4) {
        write_drive32(fs->drv, 0, addr);
    } 
}

static uint32_t get_data_sector_count(FAT32_FS *fs) {
    uint32_t totsec = fs->bootsec.params.bpb.tot_sec_32;
    uint32_t datasec = totsec - (fs->bootsec.params.bpb.reserved_sec_cnt + (
                        fs->bootsec.params.bpb.num_fats*fs->bootsec.params.bpb.fat_sz_32));
    return datasec;
}


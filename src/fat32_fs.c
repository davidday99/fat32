#include <stdint.h>
#include <string.h>
#include "fat32_internal.h" 
#include "fat32_fs.h"
#include "fat32_conf.h"

#define VERIFY_SECTORSIG(lo, hi) ((((hi << 8) & 0xFF00) | (lo & 0x00FF)) == SECTORSIG)

static uint32_t get_fat_base_addr(FAT32_FS *fs, uint8_t fatnum);
static uint32_t get_data_base_addr(FAT32_FS *fs);
static uint32_t get_data_offset(FAT32_FS *fs, uint32_t clus); 
static void init_bpb(FAT32_FS *fs);
static void init_fat_structure(FAT32_FS *fs, uint8_t fatnum);
static void clear_data_region(FAT32_FS *fs);
 
uint32_t fs_write_cluster(FAT32_FS *fs,
                            uint32_t clus,
                            uint16_t clus_off, 
                            const void *buf,
                            uint16_t count) {
    uint32_t i = 0;
    if (clus_off + count > CLUSTER_SIZE(fs) || 
        clus >= fs->clus_count ||
        clus < fs->bootsec.params.bpb.root_clus)
        return i;
    uint32_t d_base = get_data_base_addr(fs);
    uint32_t offset = get_data_offset(fs, clus); 
    uint32_t addr = d_base + offset + clus_off;
    while (i < count) {
        fs->mem->write8(((uint8_t *) buf)[i++], addr++); 
    }
    return i;
}

uint32_t fs_read_cluster(FAT32_FS *fs, 
                            uint32_t clus,
                            uint16_t clus_off,
                            void *buf,
                            uint32_t count) {
    uint32_t i = 0;
    if (clus_off + count > CLUSTER_SIZE(fs) || 
        clus >= fs->clus_count ||
        clus < fs->bootsec.params.bpb.root_clus)
        return i;
    uint32_t d_base = get_data_base_addr(fs);
    uint32_t offset = get_data_offset(fs, clus); 
    uint32_t addr = d_base + offset + clus_off;
    while (i < count) {
        ((uint8_t *) buf)[i++] = fs->mem->read8(addr++);
    }
    return i;
}

void fs_write_fat_entry(FAT32_FS *fs,
                        uint8_t fatnum,
                        uint32_t n, 
                        uint32_t v) {
    uint32_t addr = get_fat_base_addr(fs, fatnum); 
    uint32_t offset = n*sizeof(uint32_t);
    fs->mem->write32(v, addr + offset);
}

uint32_t fs_read_fat_entry(FAT32_FS *fs,
                            uint8_t fatnum,
                            uint32_t n) {
    uint32_t addr = get_fat_base_addr(fs, fatnum);
    uint32_t offset = n*sizeof(uint32_t);
    uint32_t entry = fs->mem->read32(addr + offset);
    return entry;
}

uint32_t fs_get_free_fat_entry(FAT32_FS *fs) {
    uint32_t fat_entry_cnt = fs->clus_count;
    uint32_t i = fs->bootsec.params.bpb.root_clus; 
    while (i < fat_entry_cnt) {
        uint32_t entry = fs_read_fat_entry(fs, 0, i);
        if (entry == 0)
            return i; 
    }
    return 0;
}

void fs_format(FAT32_FS *fs) {
    fs->mem->erase();
    init_bpb(fs);
    for (uint8_t i = 0; i < fs->bootsec.params.bpb.num_fats; i++) 
        init_fat_structure(fs, i);
    clear_data_region(fs);
}

uint32_t fs_init(FAT32_FS *fs) {
    for (uint16_t i = 0; i < SECTOR_SZ; i++) {
        fs->bootsec.bytes[i] = fs->mem->read8(i);
    }
    fs->valid = VERIFY_SECTORSIG(fs->bootsec.bytes[510], fs->bootsec.bytes[511]);
    uint32_t status = fs_read_fat_entry(fs, 0, 1);
    return status;
}

void fs_deinit(FAT32_FS *fs) {
    uint32_t status = fs_read_fat_entry(fs, 0, 1) & 0xF0000000;
    uint32_t new_status = status | 0x08000000;
    fs_write_fat_entry(fs, 0, 1, new_status);
    fs->valid = 0;
}

static uint32_t get_fat_base_addr(FAT32_FS *fs, uint8_t fatnum) {
    uint32_t sectors = fs->bootsec.params.bpb.reserved_sec_cnt + 
                        fs->bootsec.params.bpb.fat_sz_32*fatnum;
    uint32_t f_base = sectors*fs->bootsec.params.bpb.bytes_per_sec;
    return f_base;
}


static uint32_t get_data_base_addr(FAT32_FS *fs) {
    uint32_t fat_base_addr = get_fat_base_addr(fs, 0);
    uint32_t d_base = fat_base_addr + (fs->bootsec.params.bpb.num_fats*
                                        fs->bootsec.params.bpb.fat_sz_32*
                                        fs->bootsec.params.bpb.bytes_per_sec); 
    return d_base;
}

static uint32_t get_data_offset(FAT32_FS *fs, uint32_t clus) {
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
    fs->bootsec.params.bpb.tot_sec_32 = fs->mem->size() / 512;
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
	    fs->mem->write8(fs->bootsec.bytes[addr], addr);
    }
}

static void init_fat_structure(FAT32_FS *fs, uint8_t fatnum) {
    uint32_t fat_base = get_fat_base_addr(fs, fatnum);
    uint16_t fat_size = fs->bootsec.params.bpb.fat_sz_32*fs->bootsec.params.bpb.bytes_per_sec;

    for (uint32_t addr = fat_base; addr < fat_base + fat_size; addr += 4) {
	    fs->mem->write32(0, addr);     
    }
}

static void clear_data_region(FAT32_FS *fs) {
    uint32_t totsec = fs->bootsec.params.bpb.tot_sec_32;
    uint32_t datasec = totsec - (fs->bootsec.params.bpb.reserved_sec_cnt + (
                        fs->bootsec.params.bpb.num_fats*fs->bootsec.params.bpb.fat_sz_32));
    uint32_t datasz= datasec*fs->bootsec.params.bpb.bytes_per_sec;
    uint32_t base_addr = get_data_base_addr(fs);
    for (uint32_t addr = base_addr; addr < base_addr + datasz; addr += 4) {
        fs->mem->write32(0, addr);
    }
    
}


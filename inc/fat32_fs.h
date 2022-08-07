#ifndef _FAT32_FS_H_
#define _FAT32_FS_H_

#include "fat32_internal.h"

#define CLUSTER_SIZE(fs) (fs->bootsec.params.bpb.sec_per_clus* \
                            fs->bootsec.params.bpb.bytes_per_sec)

uint32_t fs_write_cluster(FAT32_FS *fs,
                            uint32_t clus,
                            uint16_t clus_off, 
                            const void *buf,
                            uint16_t count); 

uint32_t fs_read_cluster(FAT32_FS *fs,
                            uint32_t clus,
                            uint16_t clus_off,
                            void *buf,
                            uint32_t count);

void fs_write_fat_entry(FAT32_FS *fs,
                        uint8_t fatnum,
                        uint32_t n, 
                        uint32_t v);
 
uint32_t fs_read_fat_entry(FAT32_FS *fs,
                            uint8_t fatnum,
                            uint32_t n);


uint32_t fs_get_free_fat_entry(FAT32_FS *fs);

void fs_format(FAT32_FS *fs);

uint32_t fs_init(FAT32_FS *fs);

void fs_deinit(FAT32_FS *fs);  

#endif /* _FAT32_FS_H_ */


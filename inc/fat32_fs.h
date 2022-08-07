#ifndef _FAT32_FS_H_
#define _FAT32_FS_H_

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

void fs_format(FAT32_FS *fs);

uint32_t fs_init(FAT32_FS *fs);

void fs_deinit(FAT32_FS *fs);  

#endif /* _FAT32_FS_H_ */


#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "fat32_internal.h"
#include "fat32_fs.h"
#include "test_utils.h"

 static int test_format(FAT32_FS *fs);
 static int test_write_cluster(FAT32_FS *fs);
 static int test_read_cluster(FAT32_FS *fs);
 static int test_read_cluster_equals_write_cluster(FAT32_FS *fs);
 static int test_read_and_write_fat_entry(FAT32_FS *fs);
 
int test_fs(FAT32_FS *fs) {
    test_format(fs);
    test_write_cluster(fs);
    test_read_cluster(fs);
    test_read_cluster_equals_write_cluster(fs);
    test_read_and_write_fat_entry(fs);
    return 1;
}

int test_format(FAT32_FS *fs) {
    fs_format(fs);
    fs_init(fs);
    assert(fs->valid); 
    return 1;
}

int test_write_cluster(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    fs_format(fs);
    fs_init(fs);
    assert(fs_write_cluster(fs, 2, 0, wbuf, 513) == 512);  // write only up to size of cluster       
    assert(fs_write_cluster(fs, 2, 1, wbuf, 512) == 511);  // non-zero offset
    assert(fs_write_cluster(fs, 2, 511, wbuf, 512) == 1);  // near end of cluster boundary
    assert(fs_write_cluster(fs, 2, 512, wbuf, 512) == 0);  // past end of cluster 
    assert(fs_write_cluster(fs, 0, 0, wbuf, 512) == 0);  // disallow writes to cluster before root cluster       
    assert(fs_write_cluster(fs, 2, 1, wbuf, 511) == 511);  // write full amount with non-zero offset
    assert(fs_write_cluster(fs, 2, 0, wbuf, 512) == 512);  // write full amount with zero offset
    return 1;
} 

int test_read_cluster(FAT32_FS* fs) {
    uint8_t rbuf[1024];
    fs_format(fs);
    fs_init(fs);
    assert(fs_read_cluster(fs, 2, 0, rbuf, 512) == 512);
    assert(memcmp(rbuf, rbuf, 512) == 0);
    assert(fs_read_cluster(fs, 0, 0, rbuf, 512) == 0); 
    assert(fs_read_cluster(fs, 1, 0, rbuf, 512) == 0); 
    return 1;
}

int test_read_cluster_equals_write_cluster(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    uint8_t wbuf[1024];
    fs_format(fs);
    fs_init(fs);
    unzero_buf(wbuf, 1024);
    assert(fs_write_cluster(fs, 2, 0, wbuf, 512) == 512);  
    assert(fs_read_cluster(fs, 2, 0, rbuf, 512) == 512); 
    assert(memcmp(rbuf, wbuf, 512) == 0);
    return 1; 
}

int test_read_and_write_fat_entry(FAT32_FS *fs) {
    fs_format(fs);
    fs_init(fs);
    fs_write_fat_entry(fs, 0, 0, 0xABCDEF01);
    uint32_t entry = fs_read_fat_entry(fs, 0, 0);
    assert(entry == 0x0BCDEF01);  // top nibble should be masked out 
    return 1;
}


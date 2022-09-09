#include <stdio.h>  
#include <stdint.h>
#include <string.h>
#include <assert.h> 
#include "fat32_internal.h"
#include "fat32_fs.h"
#include "fat32_dir.h"
#include "test_utils.h"

int test_dir_open_nested(FAT32_FS *fs) {
    FAT32_DIR d;
    fs_format(fs);
    fs_init(fs); 
    assert(dir_open(fs, "/bin/", O_CREAT, &d) != NULL);
    assert(dir_open(fs, "/bin/dir1", O_CREAT, &d) != NULL);
    return 1;
}

int test_dir_open_nested_bad_path(FAT32_FS *fs) {
    FAT32_DIR d;
    fs_format(fs);
    fs_init(fs); 
    assert(dir_open(fs, "/bin/dir1/", O_CREAT, &d) == NULL);
    return 1;
}

/*
 *static int test_dir_open_file_open_and_write(FAT32_FS *fs) {
 *    uint8_t wbuf[1024];
 *    uint8_t rbuf[1024];
 *    unzero_buf(wbuf, 1024);
 *    FAT32_FILE f;
 *    fs_format(fs);
 *    fs_init(fs); 
 *    assert(file_open(fs, "/bin/", O_CREAT | O_DIRECTORY, &f) != NULL);
 *    assert(file_creat(fs, "/bin/file", &f) != NULL);
 *    assert(file_write(&f, wbuf, 1024) == 1024);
 *    f.offset = 0;
 *    f.clus_curr = f.clus_base;
 *    assert(file_read(&f, rbuf, 1024) == 1024);
 *    assert(memcmp(rbuf, wbuf, 1024) == 0);
 *    return 1;
 *}
 *
 */

int test_dir_api(FAT32_FS *fs) {
    printf("******************** test_dir_open_nested ********************\n");
    test_dir_open_nested(fs);
    printf("******************** test_dir_open_nested_bad_path ********************\n");
    test_dir_open_nested_bad_path(fs);
    return 1;
}


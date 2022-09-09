#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h> 
#include "fat32_internal.h"
#include "fat32_fs.h"
#include "fat32_file.h"
#include "test_utils.h"

int test_file_write(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs);
    assert(file_write(&f, wbuf, 20) == 20);
    assert(f.offset == 20);
    assert(f.size == 20);
    return 1;
}

int test_file_read(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        .size = 512,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    assert(file_read(&f, rbuf, 20) == 20);
    assert(f.offset == 20);
    return 1;
}

int test_file_read_equals_write(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    uint8_t wbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    unzero_buf(wbuf, 1024);
    assert(file_write(&f, wbuf, 512) == 512);
    f.clus_curr = f.clus_base;
    f.offset = 0;
    assert(file_read(&f, rbuf, 512) == 512);
    assert(memcmp(rbuf, wbuf, 512) == 0);
    return 1;
}

int test_file_write_multi_cluster(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    assert(file_write(&f, wbuf, 513) == 513);
    assert(f.clus_curr == 3);
    assert(f.offset == 513); 
    return 1;
}

int test_file_write_skip_clusters(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    fs_write_fat_entry(fs, 0, 2, END_OF_CLUSTERCHAIN);
    fs_write_fat_entry(fs, 0, 3, 1);
    fs_write_fat_entry(fs, 0, 5, 1);
    assert(file_write(&f, wbuf, 513) == 513);
    assert(f.clus_curr == 4);
    assert(f.offset == 513);
    assert(file_write(&f, wbuf, 513) == 513);
    assert(f.clus_curr == 6);
    assert(f.offset == 1026);
    return 1;
}

int test_file_read_multi_cluster(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        .size = 512*3,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    fs_write_fat_entry(fs, 0, 2, 3);
    fs_write_fat_entry(fs, 0, 3, 4);
    fs_write_fat_entry(fs, 0, 4, END_OF_CLUSTERCHAIN);

    assert(file_read(&f, rbuf, 513) == 513);
    assert(f.clus_curr == 3);
    assert(file_read(&f, rbuf, 513) == 513);
    assert(f.clus_curr == 4);
    return 1;
}

int test_file_read_skip_clusters(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        .size = 512*3,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    fs_write_fat_entry(fs, 0, 2, 4);
    fs_write_fat_entry(fs, 0, 4, 6);
    fs_write_fat_entry(fs, 0, 6, END_OF_CLUSTERCHAIN);

    assert(file_read(&f, rbuf, 513) == 513);
    assert(f.clus_curr == 4);
    assert(file_read(&f, rbuf, 513) == 513);
    assert(f.clus_curr == 6);
    return 1;
}

int test_file_write_all_clusters(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    unsigned int fat_entry_count = fs->max_valid_clus; 
    for (unsigned int i = 0; i < fat_entry_count - 2; i++) {
        assert(file_write(&f, wbuf, 512) == 512);
        assert(f.clus_curr == i + 2);
    }
    return 1;
}

int test_file_read_all_clusters(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .size = fs->max_valid_clus*fs_get_cluster_size(fs) - 2,
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    unsigned int fat_entry_count = fs->max_valid_clus;
    for (unsigned int i = 0; i < fat_entry_count - 2; i++) {
        assert(file_read(&f, rbuf, 512) == 512);
        assert(f.clus_curr == i + 2);
    }
    return 1;
}

int test_file_read_past_size(FAT32_FS *fs) {
    uint8_t rbuf[1024];
    FAT32_FILE f = {
        .clus_base = 2,
        .clus_curr = 2,
        .entry_clus = 2,
        .entry_clus_offset = 0,
        .is_dir = 0,
        .size = 1023, 
        .offset = 0,
        ._fs = fs
    }; 
    fs_format(fs);
    fs_init(fs); 
    assert(file_read(&f, rbuf, 1024) == 1023); 
    return 1;
}

int test_file_open_root(FAT32_FS *fs) {
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    file_open(fs, "/", 0, &f);
    assert(strcmp(f.name, "/") == 0);
    assert(f.clus_curr == fs->bootsec.params.bpb.root_clus);
    return 1;
}

int test_file_open_file_in_root(FAT32_FS *fs) {
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    file_open(fs, "/file", O_CREAT, &f);
    assert(strcmp(f.name, "file") == 0);
    assert(f.clus_curr == 3);
    return 1;
}

int test_file_open_bad_path(FAT32_FS *fs) {
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    assert(file_open(fs, "/bin/file", O_CREAT, &f) == NULL);
    return 1;
}

int test_file_open_dir_nested(FAT32_FS *fs) {
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    assert(file_open(fs, "/bin/", O_CREAT | O_DIRECTORY, &f) != NULL);
    assert(file_open(fs, "/bin/dir1", O_CREAT | O_DIRECTORY, &f) != NULL);
    return 1;
}

int test_file_open_dir_nested_bad_path(FAT32_FS *fs) {
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    assert(file_open(fs, "/bin/dir1/", O_CREAT | O_DIRECTORY, &f) == NULL);
    return 1;
}

int test_file_creat_and_write(FAT32_FS *fs) {
    uint8_t wbuf[1024];
    uint8_t rbuf[1024];
    unzero_buf(wbuf, 1024);
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    assert(file_open(fs, "/bin/", O_CREAT | O_DIRECTORY, &f) != NULL);
    assert(file_creat(fs, "/bin/file", &f) != NULL);
    assert(file_write(&f, wbuf, 1024) == 1024);
    f.offset = 0;
    f.clus_curr = f.clus_base;
    assert(file_read(&f, rbuf, 1024) == 1024);
    assert(memcmp(rbuf, wbuf, 1024) == 0);
    return 1;
}

int test_file_creat_and_write_real_data(FAT32_FS *fs) {
    FILE *f_real = fopen("real_file.txt", "rb");
    fseek(f_real, 0, SEEK_END);
    int size = ftell(f_real);
    rewind(f_real);
    uint8_t wbuf[size + 1];
    uint8_t rbuf[1024];
    fread(wbuf, sizeof(char), size, f_real);
    FAT32_FILE f;
    fs_format(fs);
    fs_init(fs); 
    assert(file_creat(fs, "/file", &f) != NULL);
    assert(file_write(&f, wbuf, size) == size);
    f.offset = 0;
    f.clus_curr = f.clus_base;
    assert(file_read(&f, rbuf, size) == size);
    assert(memcmp(rbuf, wbuf, size) == 0);
    return 1;
}

int test_file_api(FAT32_FS *fs) {
    printf("******************** test_file_write ********************\n");
    test_file_write(fs);
    printf("******************** test_file_read ********************\n");
    test_file_read(fs);
    printf("******************** test_file_equals_write  ********************\n");
    test_file_read_equals_write(fs);    
    printf("******************** test_file_write_multi_cluster  ********************\n");
    test_file_write_multi_cluster(fs);
    printf("******************** test_file_write_skip_clusters  ********************\n");
    test_file_write_skip_clusters(fs);
    printf("******************** test_file_read_multi_cluster  ********************\n");
    test_file_read_multi_cluster(fs);
    printf("******************** test_file_read_skip_clusters ********************\n");
    test_file_read_skip_clusters(fs);
    printf("******************** test_file_write_all_clusters ********************\n");
    test_file_write_all_clusters(fs);
    printf("******************** test_file_read_all_clusters ********************\n");
    test_file_read_all_clusters(fs);
    printf("******************** test_file_read_past_size ********************\n");
    test_file_read_past_size(fs);
    printf("******************** test_file_open_root ********************\n");
    test_file_open_root(fs);
    printf("******************** test_file_open_file_in_root ********************\n");
    test_file_open_file_in_root(fs);
    printf("******************** test_file_open_bad_path ********************\n");
    test_file_open_bad_path(fs);
    printf("******************** test_file_open_dir_nested ********************\n");
    test_file_open_dir_nested(fs);
    printf("******************** test_file_open_dir_nested_bad_path ********************\n");
    test_file_open_dir_nested_bad_path(fs);
    printf("******************** test_file_creat_and_write ********************\n");
    test_file_creat_and_write(fs);
    printf("******************** test_file_creat_and_write_real_data ********************\n");
    test_file_creat_and_write_real_data(fs);
    return 1;
}


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "drive.h"
#include "fat32.h"
#include "fat32_internal.h"
#include "fat32_fs.h"
#include "fat32_file.h"

#define MEM_SZ 0x20000  // 128 MB

#pragma GCC push_options

#pragma GCC optimize("O0")

static volatile uint8_t MEM[MEM_SZ];

static uint8_t mock_read_mem8(uint32_t addr) {
    uint8_t v = MEM[addr];
    printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    return v;
}

static uint16_t mock_read_mem16(uint32_t addr) {
    uint16_t v = (MEM[addr] & 0xFF) | ((MEM[addr + 1] << 8) & 0xFF00);
    printf("read %#x\n", v);
    return v;
}

static uint32_t mock_read_mem32(uint32_t addr) {
    uint32_t v = (MEM[addr] & 0xFF) | 
                 ((MEM[addr + 1] << 8) & 0xFF00) |
                 ((MEM[addr + 2] << 16) & 0xFF0000) | 
                 ((MEM[addr + 3] << 24) & 0xFF000000);
    printf("read %#x\n", v);
    return v;
}

static void mock_write_mem8(uint8_t v, uint32_t addr) { 
    MEM[addr] = v;
    printf("write %#x to %#x\n", v, addr);
}

static void mock_write_mem16(uint16_t v, uint32_t addr) {
    MEM[addr] = v & 0xFF;
    MEM[addr + 1] = (v >> 8) & 0xFF;
    printf("write %#x to %#x\n", v, addr);
}

static void mock_write_mem32(uint32_t v, uint32_t addr) {
    MEM[addr] = v & 0xFF;
    MEM[addr + 1] = (v >> 8) & 0xFF;
    MEM[addr + 2] = (v >> 16) & 0xFF;
    MEM[addr + 3] = (v >> 24) & 0xFF;
    printf("write %#x to %#x\n", v, addr);
}

static void mock_erase_mem() {
    printf("erasing\n");
    memset((void *)MEM, 0xFF, MEM_SZ);
}

static uint32_t mock_size_mem() {
    return MEM_SZ;
}

#pragma GCC pop_options

DRIVE mem = {
    .read8 = mock_read_mem8,
    .read16 = mock_read_mem16,
    .read32 = mock_read_mem32,
    .write8 = mock_write_mem8,
    .write16 = mock_write_mem16,
    .write32 = mock_write_mem32,
    .erase = mock_erase_mem,
    .size = mock_size_mem
};
FAT32_FILE f;
FAT32_FS fs = {
    .mem = &mem
};


int test_fs(void) {
    uint8_t wbuf[1024];
    uint8_t rbuf[1024];
    memset(rbuf, 0, 1024);
    for (int i = 0; i < 1024; i++) wbuf[i] = i;

    fs_format(&fs);
    fs_init(&fs);
    printf("fs %s\n", fs.valid ? "valid" : "invalid");
    
    assert(fs_write_cluster(&fs, 2, 0, wbuf, 513) == 0);        
    assert(fs_write_cluster(&fs, 0, 0, wbuf, 512) == 0);        
    assert(fs_write_cluster(&fs, 2, 1, wbuf, 512) == 0);
    assert(fs_write_cluster(&fs, 2, 1, wbuf, 511) == 511);

    fs_read_cluster(&fs, 2, 1, rbuf, 511);
    assert(memcmp(rbuf, wbuf, 511) == 0);

    fs_write_fat_entry(&fs, 0, 0, 0xABCDEF01);
    uint32_t entry = fs_read_fat_entry(&fs, 0, 0);
    assert(entry == 0xABCDEF01); 
    return 1;
}

int test_file_api(void) {
    file_open(&fs, "/", &f);

    file_open(&fs, "/foo.c", &f);    

    file_open(&fs, "/foo.c/foo1.txt", &f);

    return 1;
}

int main() {
    test_fs();
    test_file_api();
    printf("tests complete\n");
    return 0;
}


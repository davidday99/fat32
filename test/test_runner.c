#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "drive.h"
#include "fat32.h"
#include "fat32_internal.h"
#include "fat32_fs.h"
#include "fat32_file.h"
#include "tests.h"

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

DRIVE drv = {
    ._read8 = mock_read_mem8,
    ._read16 = mock_read_mem16,
    ._read32 = mock_read_mem32,
    ._write8 = mock_write_mem8,
    ._write16 = mock_write_mem16,
    ._write32 = mock_write_mem32,
    ._erase = mock_erase_mem,
    ._size = mock_size_mem
};

FAT32_FS fs = {
    .drv = &drv
};

int main() {
    test_fs(&fs);
    test_file_api(&fs);
    test_dir_api(&fs);
    printf("tests complete\n");
    return 0;
}


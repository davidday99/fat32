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

static uint32_t  mock_read(uint32_t sector, uint32_t offset, void *buf, uint32_t count) {
    uint32_t addr = sector*512 + offset;
    uint32_t i = 0;;
    for (; i < count; i++) {
        uint8_t v = MEM[addr + i];
        ((uint8_t *) buf)[i] = v;
        printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    }
    return i;
}

static uint32_t mock_write(uint32_t sector, uint32_t offset, const void *buf, uint32_t count) {
    uint32_t addr = sector*512 + offset;
    uint32_t i = 0;
    for (; i < count; i++) {
        MEM[addr + i] = ((uint8_t *) buf)[i];
    }
    return i;
}

static void mock_erase_mem() {
    printf("erasing\n");
    memset((void *)MEM, 0, MEM_SZ);
}

static uint32_t mock_size_mem() {
    return MEM_SZ;
}

#pragma GCC pop_options

DRIVE drv = {
    ._read = mock_read,
    ._write = mock_write,
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


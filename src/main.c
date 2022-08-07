#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "eeprom.h"
#include "fat32.h"
#include "fat32_internal.h"
#include "fat32_fs.h"

#define MEM_SZ 0x20000  // 128 MB

static uint8_t MEM[MEM_SZ];

static uint8_t read_mem8(uint32_t addr) {
    uint8_t v = MEM[addr];
    printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    return v;
}

static uint16_t read_mem16(uint32_t addr) {
    uint16_t v = (MEM[addr] & 0xFF) | ((MEM[addr + 1] << 8) & 0xFF00);
    printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    return v;
}

static uint32_t read_mem32(uint32_t addr) {
    uint32_t v = (MEM[addr] & 0xFF) | 
                 ((MEM[addr + 1] << 8) & 0xFF00) |
                 ((MEM[addr + 2] << 16) & 0xFF0000) | 
                 ((MEM[addr + 3] << 24) & 0xFF000000);
    printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    return v;
}

static void write_mem8(uint8_t v, uint32_t addr) { 
    MEM[addr] = v;
    printf("write %#x to %#x\n", v, addr);
}

static void write_mem16(uint16_t v, uint32_t addr) {
    MEM[addr] = v & 0xFF;
    MEM[addr + 1] = (v >> 8) & 0xFF;
    printf("write %#x to %#x\n", v, addr);
}

static void write_mem32(uint32_t v, uint32_t addr) {
    MEM[addr] = v & 0xFF;
    MEM[addr + 1] = (v >> 8) & 0xFF;
    MEM[addr + 2] = (v >> 16) & 0xFF;
    MEM[addr + 3] = (v >> 24) & 0xFF;
    printf("write %#x to %#x\n", v, addr);
}

static void erase_mem() {
    printf("erasing\n");
    memset(MEM, 0xFF, MEM_SZ);
}

static uint32_t size_mem() {
    return MEM_SZ;
}

int main() {
    struct EEPROM mem = {
        .read8 = read_mem8,
        .read16 = read_mem16,
        .read32 = read_mem32,
        .write8 = write_mem8,
        .write16 = write_mem16,
        .write32 = write_mem32,
        .erase = erase_mem,
        .size = size_mem
    };
    FAT32_FILE f;
    FAT32_FS fs;
    fs.mem = &mem;

    fs_format(&fs);
    fs_init(&fs);
    printf("fs %s\n", fs.valid ? "valid" : "invalid");
    fat32_open("/bin/dir1/dir2/foo.c", &f);
    return 0;
}


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "eeprom.h"
#include "fat32.h"

#define MEM_SZ 0x20000  // 128 MB


static uint8_t MEM[MEM_SZ];

static uint8_t read_mem(uint32_t addr) {
    uint8_t v = MEM[addr];
    printf("read %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    return v;
}

static void write_mem(uint8_t v, uint32_t addr) {
    printf("write %#x %c\n", v, isalnum(v) || ispunct(v) ? v : ' ');
    MEM[addr] = v;
}

static void erase_mem() {
    printf("erasing\n");
    memset(MEM, 0xff, MEM_SZ);
}

static uint32_t size_mem() {
    return MEM_SZ;
}

int main() {
    struct EEPROM mem = {
        .read = read_mem,
        .write = write_mem,
        .erase = erase_mem,
        .size = size_mem
    };

    format_fat32(&mem);

    return 0;
}

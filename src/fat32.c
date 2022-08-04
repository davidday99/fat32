#include <stdint.h>
#include <string.h>
#include "eeprom.h"
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_format.h"
#include "fat32.h"

static FAT32_FS ACTIVE_FS;

void format_fat32(struct EEPROM *mem) {
    _format_fat32(mem);
}

void init_fat32_fs(struct EEPROM *mem) {
    for (uint16_t i = 0; i < SECTOR_SZ; i++) {
        ACTIVE_FS.bootsec.bytes[i] = mem->read8(i);
    }
    ACTIVE_FS.mem = mem;
    ACTIVE_FS.valid = VERIFY_SECTORSIG(ACTIVE_FS.bootsec.bytes[510], ACTIVE_FS.bootsec.bytes[511]);
}

void deinit_fat32_fs(void) {
    return;
}

uint8_t fat32_fs_valid(void) {
    return ACTIVE_FS.valid == 1;
}

FAT32_FILE *fat32_open(char *path, FAT32_FILE *fptr) {
    return fat32_fs_valid() ? _fat32_open(&ACTIVE_FS, path, fptr) : NULL;
}

void fat32_close(FAT32_FILE *fptr) {
    return;
}

uint32_t fat32_read(FAT32_FILE *fptr) {
    return 0;
}

uint32_t fat32_write(FAT32_FILE *fptr) {
    return 0;
}


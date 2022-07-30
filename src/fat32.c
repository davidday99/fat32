#include <stdint.h>
#include <string.h>
#include "eeprom.h"
#include "fat32_internal.h"
#include "fat32_file.h"
#include "fat32_format.h"
#include "fat32.h"

static FAT32_FS ACTIVE_FS;

FAT32_FILE *fat32_open(char *path, FAT32_FILE *fptr) {
    return fat32_fs_valid() ? _fat32_open(path, fptr) : NULL;
}

void fat32_mkdir(struct EEPROM *mem, char *dirname) {

}

void fat32_mkfile(struct EEPROM *mem, char *fname) {

}

void fat32_move(struct EEPROM *mem, char *name) {

}

void fat32_remove(struct EEPROM *mem, char *name) {

}

void format_fat32(struct EEPROM *mem) {
    _format_fat32(mem);
}

void init_fat32_fs(struct EEPROM *mem) {
    for (uint16_t i = 0; i < SECTOR_SZ; i++) {
        ACTIVE_FS.params.bytes[i] = mem->read8(i);
    }
    ACTIVE_FS.mem = mem;
    ACTIVE_FS.valid = VERIFY_SECTORSIG(ACTIVE_FS.params.bytes[510], ACTIVE_FS.params.bytes[511]);
}

void deinit_fat32_fs(void) {
    return;
}

uint8_t fat32_fs_valid(void) {
    return ACTIVE_FS.valid == 1;
}


#include <stdint.h>
#include "drive.h"

uint8_t read_mem8(DRIVE *drv, uint32_t addr) {
    return drv->read8(addr);
}

uint16_t read_mem16(DRIVE *drv, uint32_t addr) {
    return drv->read16(addr);
}

uint32_t read_mem32(DRIVE *drv, uint32_t addr) { 
    return drv->read32(addr);
}

void write_mem8(DRIVE *drv, uint8_t v, uint32_t addr) { 
    drv->write8(v, addr);
}

void write_mem16(DRIVE *drv, uint16_t v, uint32_t addr) {
    drv->write16(v, addr);
}

void write_mem32(DRIVE *drv, uint32_t v, uint32_t addr) {
    drv->write32(v, addr);
}

void erase_mem(DRIVE *drv) {
    drv->erase();
}

uint32_t size_mem(DRIVE *drv) {
    return drv->size();
}



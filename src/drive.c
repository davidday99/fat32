#include <stdint.h>
#include "drive.h"

uint8_t read_drive8(DRIVE *drv, uint32_t addr) {
    return drv->_read8(addr);
}

uint16_t read_drive16(DRIVE *drv, uint32_t addr) {
    return drv->_read16(addr);
}

uint32_t read_drive32(DRIVE *drv, uint32_t addr) { 
    return drv->_read32(addr);
}

void write_drive8(DRIVE *drv, uint8_t v, uint32_t addr) { 
    drv->_write8(v, addr);
}

void write_drive16(DRIVE *drv, uint16_t v, uint32_t addr) {
    drv->_write16(v, addr);
}

void write_drive32(DRIVE *drv, uint32_t v, uint32_t addr) {
    drv->_write32(v, addr);
}

void erase_drive(DRIVE *drv) {
    drv->_erase();
}

uint32_t size_drive(DRIVE *drv) {
    return drv->_size();
}



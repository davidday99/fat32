#include <stdint.h>
#include "drive.h"

uint32_t read_drive(DRIVE *drv, uint32_t sector, uint32_t offset, void *buf, uint32_t count) { 
    return drv->_read(sector, offset, buf, count);
}

uint32_t write_drive(DRIVE *drv, uint32_t sector, uint32_t offset, const void *buf, uint32_t count) {
    return drv->_write(sector, offset, buf, count);
}

void erase_drive(DRIVE *drv) {
    drv->_erase();
}

uint32_t size_drive(DRIVE *drv) {
    return drv->_size();
}



#ifndef _DRIVE_H_
#define _DRIVE_H_

#include <stdint.h>

typedef struct drive_t {
    uint32_t (*_read)(uint32_t sector, uint32_t offset, void *buf, uint32_t count);
    uint32_t (*_write)(uint32_t sector, uint32_t offset, const void *buf, uint32_t count);
    void (*_erase)(void);
    uint32_t (*_size)(void);
} DRIVE;

uint32_t read_drive(DRIVE *drv, uint32_t sector, uint32_t offset, void *buf, uint32_t count); 
uint32_t write_drive(DRIVE *drv, uint32_t sector, uint32_t offset, const void *buf, uint32_t count);
void erase_drive(DRIVE *drv);
uint32_t size_drive(DRIVE *drv);

#endif  /* _DRIVE_H_ */


#ifndef _DRIVE_H_
#define _DRIVE_H_

#include <stdint.h>

typedef struct drive_t {
    uint8_t (*_read8)(uint32_t addr);
    uint16_t (*_read16)(uint32_t addr);
    uint32_t (*_read32)(uint32_t addr);
    void (*_write8)(uint8_t v, uint32_t addr);
    void (*_write16)(uint16_t v, uint32_t addr);
    void (*_write32)(uint32_t v, uint32_t addr);
    void (*_erase)(void);
    uint32_t (*_size)(void);
} DRIVE;

uint8_t read_drive8(DRIVE *drv, uint32_t addr);
uint16_t read_drive16(DRIVE *drv, uint32_t addr);
uint32_t read_drive32(DRIVE *drv, uint32_t addr); 
void write_drive8(DRIVE *drv, uint8_t v, uint32_t addr); 
void write_drive16(DRIVE *drv, uint16_t v, uint32_t addr);
void write_drive32(DRIVE *drv, uint32_t v, uint32_t addr);
void erase_drive(DRIVE *drv);
uint32_t size_drive(DRIVE *drv);

#endif  /* _DRIVE_H_ */


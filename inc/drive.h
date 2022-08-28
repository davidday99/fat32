#ifndef _DRIVE_H_
#define _DRIVE_H_

#include <stdint.h>

typedef struct drive_t {
    uint8_t (*read8)(uint32_t addr);
    uint16_t (*read16)(uint32_t addr);
    uint32_t (*read32)(uint32_t addr);
    void (*write8)(uint8_t v, uint32_t addr);
    void (*write16)(uint16_t v, uint32_t addr);
    void (*write32)(uint32_t v, uint32_t addr);
    void (*erase)(void);
    uint32_t (*size)(void);
} DRIVE;

uint8_t read_mem8(DRIVE *drv, uint32_t addr);
uint16_t read_mem16(DRIVE *drv, uint32_t addr);
uint32_t read_mem32(DRIVE *drv, uint32_t addr); 
void write_mem8(DRIVE *drv, uint8_t v, uint32_t addr); 
void write_mem16(DRIVE *drv, uint16_t v, uint32_t addr);
void write_mem32(DRIVE *drv, uint32_t v, uint32_t addr);
void erase_mem(DRIVE *drv);
uint32_t size_mem(DRIVE *drv);

#endif  /* _DRIVE_H_ */


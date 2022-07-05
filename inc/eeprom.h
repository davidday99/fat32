#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>

struct EEPROM {
    uint8_t (*read)(uint32_t addr);
    void (*write)(uint8_t v, uint32_t addr);
    void (*erase)(void);
    uint32_t (*size)(void);
};

void eeprom_read(struct EEPROM *mem, uint32_t addr, uint8_t buf, uint32_t len);
void eeprom_write(struct EEPROM *mem, uint32_t addr, uint8_t buf, uint32_t len);

#endif /* _EEPROM_H_ */
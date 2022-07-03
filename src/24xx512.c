/* 24xx512 I2C EEPROM */

#include <stdint.h>

struct GPIO_PIN;
struct I2C_MODULE;

struct eeprom_24xx512 {
    struct GPIO_PIN *A0;
    struct GPIO_PIN *A1;
    struct GPIO_PIN *A2;
    struct I2C_MODULE *i2c;
};

uint8_t *eeprom_24xx512_read(struct eeprom_24xx512 *eeprom, uint32_t addr, uint8_t *buf, uint32_t len) {
    return 0;
}

void eeprom_24xx512_write(struct eeprom_24xx512 *eeprom, uint32_t addr, uint8_t *buf, uint32_t len) {
    return;
}

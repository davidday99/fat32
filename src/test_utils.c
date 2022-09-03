#include <stdint.h>
#include <string.h>
#include "test_utils.h"

void unzero_buf(uint8_t *buf, uint32_t count) {
    for (int i = 0; i < 1024; i++) buf[i] = i;
}

void zero_buf(uint8_t *buf, uint32_t count) {
    memset(buf, 0, count);
}



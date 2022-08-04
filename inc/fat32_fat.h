#ifndef _FAT32_FAT_H_
#define _FAT32_FAT_H_

#include <stdint.h>

uint32_t get_fat_structure_base_addr(BOOT_SECTOR *sec);

#endif /* _FAT32_FAT_H_ */


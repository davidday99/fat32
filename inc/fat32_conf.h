#ifndef _FAT32_CONF_H_
#define _FAT32_CONF_H_

#define CONF_OEM_NAME                        "MSWIN4.1"
#define CONF_BYTES_PER_SECTOR                512
#define CONF_SECTORS_PER_CLUSTER             1
#define CONF_RESERVED_SECTOR_COUNT           32
#define CONF_NUM_FATS                        1
#define CONF_ROOT_ENTRY_COUNT                0
#define CONF_TOTAL_SECTORS_16                      0
#define CONF_MEDIA_TYPE                      0xF8
#define CONF_FAT_SIZE_16                     0
#define CONF_SECTORS_PER_TRACK               0
#define CONF_NUM_HEADS                       0   
#define CONF_HIDDEN_SECTOR_COUNT             0
#define CONF_TOTAL_SECTORS_32                0
#define CONF_FAT_SIZE_32                     2
#define CONF_EXT_FLAGS                       0x0010
#define CONF_FS_VERSION                      0
#define CONF_ROOT_CLUSTER                    2
#define CONF_FS_INFO                         1
#define CONF_BK_BOOT_SECTOR                  0
#define CONF_DRIVE_NUM                       0x80
#define CONF_VOLUME_ID                       0xCCCCCCCC
#define CONF_VOLUME_LABEL                    "NO NAME    "
#define CONF_FS_TYPE                         "FAT32      "

#endif /* _FAT32_CONF_H_ */


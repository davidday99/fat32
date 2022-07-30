#include <stdio.h>
#include <string.h>
#include "fat32_internal.h"
#include "fat32_file.h"
#include "eeprom.h"

FAT32_FILE *_fat32_open(char *path, FAT32_FILE *fptr) {
    char buf[255];
    strcpy(buf, path);
    char *part = strtok(buf, "/");
    while (part != NULL) {
        printf("%s\n", part);
        part = strtok(NULL, "/");
    } 
    return NULL;
}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include "usb.h"
#include "fat32.h"
#include "fat32_fs.h"

#define READ 0
#define WRITE 1

static int p_read_disk[2];
static int p_write_disk[2];

void child_read(uint32_t addr, char count) {
    char addr_str[16];
    char count_str[8];
    snprintf(addr_str, sizeof(addr_str), "skip=%u", addr);
    snprintf(count_str, sizeof(count_str), "count=%c", count);
    char *argv[] = {"/bin/dd", "if=/dev/sdb1", "status=none", "iflag=count_bytes,skip_bytes", addr_str, count_str, NULL};
    execve(argv[0], argv, NULL);
}

void child_write(const uint32_t addr) {
    char addr_str[16];
    snprintf(addr_str, sizeof(addr_str), "seek=%u", addr);
    char *argv[] = {"/bin/dd", "of=/dev/sdb1", "count=1", "status=none", "oflag=seek_bytes", addr_str, NULL};
    execve(argv[0], argv, NULL);
}

uint8_t read8(uint32_t addr) {
    int status;
    uint8_t v;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_read(addr, '1');
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
    read(p_read_disk[READ], &v, 1);
    return v;
}

uint16_t read16(uint32_t addr) {
    int status;
    uint16_t v;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_read(addr, '2');
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
    read(p_read_disk[READ], &v, 2);
    return v;
}

uint32_t read32(uint32_t addr) {
    int status;
    uint32_t v;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_read(addr, '4');
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
    read(p_read_disk[READ], &v, 4);
    return v;
}

void write8(const uint8_t v, const uint32_t addr) {
    int status;
    write(p_write_disk[WRITE], &v, 1);
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_write(addr);
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
}

void write16(const uint16_t v, const uint32_t addr) {
    int status;
    write(p_write_disk[WRITE], &v, 2);
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_write(addr);
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
}

void write32(const uint32_t v, const uint32_t addr) {
    int status;
    write(p_write_disk[WRITE], &v, 4);
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_write(addr);
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
}

void erase() {
    return;
}

uint32_t size() {
    return 0x20000;
}

DRIVE drv = {
    ._read8 = read8,
    ._read16 = read16,
    ._read32 = read32,
    ._write8 = write8,
    ._write16 = write16,
    ._write32 = write32,
    ._erase = erase,
    ._size = size
};

FAT32_FS fs = {
    .drv = &drv
};

int main() {
    int status = pipe(p_read_disk);
    status = pipe(p_write_disk);
    if (status == -1)
        exit(-1);    
    fs_format(&fs);
    printf("Done!\n");
}

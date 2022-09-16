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

void child_read(uint32_t sector, uint32_t offset, uint32_t count) {
    uint32_t addr = sector*512 + offset;
    char addr_str[16];
    char count_str[10];
    snprintf(addr_str, sizeof(addr_str), "skip=%u", addr);
    snprintf(count_str, sizeof(count_str), "count=%u", count);
    char *argv[] = {"/bin/dd", "if=/dev/sda1", "iflag=count_bytes,skip_bytes", addr_str, count_str, NULL};
    execve(argv[0], argv, NULL);
}

void child_write(const uint32_t sector, uint32_t offset, const void *buf) {
    uint32_t addr = sector*512 + offset;
    char addr_str[16];
    snprintf(addr_str, sizeof(addr_str), "seek=%u", addr);
    char *argv[] = {"/bin/dd", "of=/dev/sda1", "count=1", "oflag=seek_bytes", addr_str, NULL};
    execve(argv[0], argv, NULL);
}

void child_erase() {
    char *argv[] = {"/bin/dd", "if=/dev/zero", "of=/dev/sda1", "bs=1G", NULL};
    execve(argv[0], argv, NULL);
}

uint32_t read_dd(uint32_t sector, uint32_t offset, void *buf, uint32_t count) {
    int status;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_read(sector, offset, count);
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
    read(p_read_disk[READ], buf, count);
    return count;
}

uint32_t write_dd(uint32_t sector, uint32_t offset, const void *buf, uint32_t count)  {
    int status;
    write(p_write_disk[WRITE], buf, count);
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_write(sector, offset, buf);
    } else {
        wait(&status);
        fsync(p_read_disk[WRITE]);
    }
    return count;
}

void erase_dd() {
    int status;
    int pid = fork();
    if (pid == 0) {
        child_erase();
    } else {
        wait(&status); 
    }
}

uint32_t size_dd() {
    return 1000000000UL;
}

DRIVE drv = {
    ._read = read_dd,
    ._write = write_dd,
    ._erase = erase_dd,
    ._size = size_dd
};

FAT32_FS fs = {
    .drv = &drv
};

int main() {
    int status = pipe(p_read_disk);
    status = pipe(p_write_disk);
    if (status == -1)
        exit(-1);    
    FAT32_FILE f;
    FAT32_FILE *fptr;
    char *wbuf = "hello world";
    fs_format(&fs);
    fs_init(&fs);
    fptr = file_open(&fs, "/test.c", O_CREAT | O_DIRECTORY, &f);
    file_write(fptr, wbuf, sizeof(wbuf));
    printf("Done!\n");
}


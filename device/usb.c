#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include "usb.h"

#define READ 0
#define WRITE 1

static int p_read_disk[2];
static int p_write_disk[2];

static char sector[4096];

USB_DRIVE *usb_init(USB_DRIVE *usb) {
    int result = libusb_init(&usb->ctx);
    if (result == LIBUSB_SUCCESS)
        return usb;
    else
        return NULL;
}

void child_read(void) {
    char *argv[] = {"/bin/dd", "if=/dev/sdb1", "bs=512", "count=1", "status=none", NULL};
    execve(argv[0], argv, NULL);
}

void child_write(void) {
    char *argv[] = {"/bin/dd", "of=/dev/sdb1", "bs=512", "count=1", "status=none", NULL};
    execve(argv[0], argv, NULL);
}

int read_sector() {
    int status;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_read();
    } else {
        wait(&status);
    }
    int count = read(p_read_disk[READ], sector, 512);
    return count;
}

int write_sector(const void *buf, int count) {
    int status;
    int written = 0;
    int pid = fork();
    if (pid == 0) {
        dup2(p_read_disk[WRITE], STDOUT_FILENO);
        dup2(p_write_disk[READ], STDIN_FILENO);
        child_write();
    } else {
        written = write(p_write_disk[WRITE], buf, count);
        wait(&status);
    }
    return count;
}

int main() {
    int status = pipe(p_read_disk);
    status = pipe(p_write_disk);
    if (status == -1)
        exit(-1);    
    printf("Done!\n");
    return 0;
}


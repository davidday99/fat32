#include <stdio.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>
#include "usb.h"

#define BULK_IN_ENDPOINT 0x81
#define BULK_OUT_ENDPOINT 0x02

USB_DRIVE *usb_init(USB_DRIVE *usb) {
    int result = libusb_init(&usb->ctx);
    if (result == LIBUSB_SUCCESS)
        return usb;
    else
        return NULL;
}

void usb_deinit(USB_DRIVE *usb) {
    libusb_close(usb->handle);
    libusb_exit(usb->ctx);
}

USB_DRIVE *usb_open_device(USB_DRIVE *usb, uint16_t vendor_id, uint16_t product_id) {
    usb->handle = libusb_open_device_with_vid_pid(usb->ctx, vendor_id, product_id);
    if (usb->handle == NULL)
        return NULL;
    else
        return usb;
}

int usb_bulk_write(USB_DRIVE *usb, const void *buf, int length, int *success) {
    int transferred;
    *success = libusb_bulk_transfer(usb->handle, BULK_IN_ENDPOINT, (unsigned char *) buf, length, &transferred, 0);
    return transferred;
}

int usb_bulk_read(USB_DRIVE *usb, void *buf, int length, int *success) {
    int transferred;
    *success = libusb_bulk_transfer(usb->handle, BULK_OUT_ENDPOINT, buf, length, &transferred, 0);
    return transferred;
}

int main() {
    USB_DRIVE usb;
    USB_DRIVE *usbptr = &usb;
    usbptr = usb_init(usbptr);
    usbptr = usb_open_device(usbptr, 0x0781, 0x5599); 
    char buf[1024];
    int size = libusb_get_string_descriptor_ascii(usbptr->handle, 1, buf, 1024);
    return 0;
}


#ifndef _USB_H_
#define _USB_H_

#include <stdint.h> 
#include <libusb-1.0/libusb.h>

typedef struct _usb {
    libusb_context *ctx;
    libusb_device_handle *handle;
} USB_DRIVE;

USB_DRIVE *usb_init(USB_DRIVE *usb);
void usb_deinit(USB_DRIVE *usb);
USB_DRIVE *usb_open_device(USB_DRIVE *usb, uint16_t vendor_id, uint16_t product_id);

#endif /* _USB_H_ */


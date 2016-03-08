#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>


// gcc list_usb.c -ludev

int main() {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev, check libudev\n");
        exit(1);
    }
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_add_match_property(enumerate, "ID_VENDOR_ID", "8087");
    udev_enumerate_add_match_property(enumerate,
        "ID_SERIAL", "Intel_ARDUINO_101_AE6642SQ55000RS");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        printf("Ardunio 101 Device Node Path: %s\n",
            udev_device_get_devnode(dev));
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return 0;
}

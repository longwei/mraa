#!/bin/bash

for usbpath in $(find /sys/bus/usb/devices/usb*/ -name ttyACM? ); do
    (
        devname="$(basename $usbpath)"
        usbdevpath="${usbpath%/dev}"
        eval "$(udevadm info -q property --export -p $usbdevpath)"
        [[ -z "$ID_MODEL" ]] && continue
        if [ $ID_MODEL == "ARDUINO_101" ]; then
          echo "$ID_MODEL - /dev/$devname"
        fi
    )
done

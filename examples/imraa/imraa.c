/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Author: Longwei Su <lsu@ics.com>
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <json-c/json.h>
#include <libudev.h>

#include <mraa/uart.h>
#include <mraa/gpio.h>

#define IMRAA_CONF_FILE "/etc/imraa.conf"

typedef struct mraa_io_objects_t
{
    const char* type;
    int index;
    bool raw;
    char* label;
} mraa_io_objects_t;

const char*
list_serialport() {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    const char* ret;
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
        ret = udev_device_get_devnode(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return ret;
}

int
reset_ping(const char* modem)
{
    mraa_uart_context uart;
    uart = mraa_uart_init_raw(modem);
    mraa_uart_set_baudrate(uart, 1200);

    if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return 1;
    }
    mraa_uart_stop(uart);
    mraa_deinit();
    return 0;
}

int
flash_subplat(const char* bin_path, const char* bin_file_name, const char* com_port) {
    printf("starting flash script\n");
    printf("bin path: %s\n", bin_path);
    printf("bin img file: %s\n", bin_file_name);
    printf("serial port: %s\n", com_port);
    size_t bin_path_len = strlen(bin_path);

    printf("Forcing reset using 1200bps open/close on port %s\n", com_port);
    reset_ping(com_port);

    char *ln = NULL;
    size_t len = 0;
    bool board_found = false;
    const char* dfu_list = "/dfu-util -d ,8087:0ABA -l";
    char* full_dfu_list = (char*) calloc ((bin_path_len
                                           + strlen(dfu_list) + 1), sizeof(char));
    strncat(full_dfu_list, bin_path, strlen(bin_path));
    strncat(full_dfu_list, dfu_list, strlen(dfu_list));

    int i;
    //dfu list is still needed, as the time for reset and recognized is varies from platform to platform.
    //one dfu able to query available device, then it is ready to flash
    for(i = 0; i < 10 && board_found == false; i++) {
        printf("Waiting for device...\n");
        //dfu-util -d,8087:0ABA -l
        FILE* dfu_result = popen(full_dfu_list, "r");
        if (dfu_result == NULL) {
            printf("Failed to run command\n");
            exit(1);
        }

        if (i == 4) {
            printf("Flashing is taking longer than expected\n");
            printf("Try pressing MASTER_RESET button\n");
        }

        while (getline(&ln, &len, dfu_result) != -1){
            if(strstr(ln, "sensor_core")) {
                board_found = true;
                printf("Device found!\n");
                break;
            }
        }
        sleep(1);
        if (pclose (dfu_result) != 0) {
            printf("Failed to close command\n");
            exit(1);
        }
    }
    free(ln);

    if (board_found == false) {
        printf("ERROR: Device is not responding.\n");
        exit(1);
    }

    const char* dfu_upload = "/dfu-util -d ,8087:0ABA -D ";
    const char* dfu_option = " -v -a 7 -R";
    int buffersize = bin_path_len + strlen(dfu_upload) + strlen(bin_file_name) + strlen(dfu_option) + 1;
    char* full_dfu_upload = calloc(buffersize, sizeof(char));
    strncat(full_dfu_upload, bin_path, strlen(bin_path));
    strncat(full_dfu_upload, dfu_upload, strlen(dfu_upload));
    strncat(full_dfu_upload, bin_file_name, strlen(bin_file_name));
    strncat(full_dfu_upload, dfu_option, strlen(dfu_option));
    printf("flash cmd: %s\n",full_dfu_upload);
    int status = system(full_dfu_upload);
    free(full_dfu_upload);
    if (status != 0) {
        printf("ERROR: Upload failed on %s\n", com_port);
        exit(1);
    }
    printf("SUCCESS: Sketch will execute in about 5 seconds.\n");
    sleep(5);
    return 0;
}

void
write_lockfile(const char* lock_file_location){
    FILE *fh;
    json_object * platform1 = json_object_new_object();
    json_object_object_add(platform1,"id", json_object_new_string("0"));
    json_object_object_add(platform1,"Platform", json_object_new_string("NULL_PLATFORM"));

    json_object * platform2 = json_object_new_object();
    json_object_object_add(platform2,"id", json_object_new_string("255"));
    json_object_object_add(platform2,"Platform", json_object_new_string("GENERIC_FIRMATA"));
    json_object_object_add(platform2,"uart", json_object_new_string("/dev/ttyACM0"));

    json_object *platfroms = json_object_new_array();
    json_object_array_add(platfroms,platform1);
    json_object_array_add(platfroms,platform2);
    json_object * lock_file = json_object_new_object();
    json_object_object_add(lock_file,"Platform", platfroms);
    // printf ("output JSON: \n %s",json_object_to_json_string_ext(lock_file, JSON_C_TO_STRING_PRETTY));
    fh = fopen(lock_file_location, "w");
    if (fh != NULL)
    {
        fputs(json_object_to_json_string_ext(lock_file, JSON_C_TO_STRING_PRETTY), fh);
        fclose(fh);
    }
}

void
handle_subplatform(struct json_object* jobj){
    struct json_object* platform;
    int i, ionum;
    const char* dfu_loc;
    const char* lockfile_loc;
    const char* flash_loc;

    struct json_object* dfu_location;
    if (json_object_object_get_ex(jobj, "dfu-utils-location", &dfu_location) == true) {
        if (json_object_is_type(dfu_location, json_type_string)) {
            printf("dfu location: %s\n", json_object_get_string(dfu_location));
            dfu_loc = json_object_get_string(dfu_location);
        } else {
            fprintf(stderr, "dfu location string incorrectly parsed\n");
        }
    }

    struct json_object* lockfile_location;
    if (json_object_object_get_ex(jobj, "lockfile-location", &lockfile_location) == true) {
        if (json_object_is_type(lockfile_location, json_type_string)) {
            printf("lock file location: %s\n", json_object_get_string(lockfile_location));
            lockfile_loc = json_object_get_string(lockfile_location);
        } else {
            fprintf(stderr, "lock file string incorrectly parsed\n");
        }
    }
    if (json_object_object_get_ex(jobj, "Platform", &platform) == true) {
        if (json_object_is_type(platform, json_type_array)) {
            ionum = json_object_array_length(platform);
            for (i = 0; i < ionum; i++ ){
                struct json_object* ioobj = json_object_array_get_idx(platform, i);
                json_object_object_foreach(ioobj, key, val) {
                    if (strcmp(key, "flash") == 0) {
                        flash_loc = json_object_get_string(val);
                    }
                }
            }
        } else {
            fprintf(stderr, "platform string incorrectly parsed\n");
        }
    }
    //got flash? do flash

    if (access(lockfile_loc, F_OK) != -1 ){
        printf("already exist lock file, skip flashing\n");
    } else {
        printf("***doing flashing here****\n");
        //TODO define dfu location in conf?
        int flash_result = flash_subplat(dfu_loc, flash_loc, list_serialport());
        write_lockfile(lockfile_loc);
    }
}

void
handle_IO(struct json_object* jobj) {
    struct mraa_io_objects_t* mraaobjs;
    struct json_object* ioarray;
    int ionum = 0;
    int i;
    if (json_object_object_get_ex(jobj, "IO", &ioarray) == true) {
        ionum = json_object_array_length(ioarray);
        mraaobjs = malloc(sizeof(ioarray) * sizeof(mraa_io_objects_t));
        printf("Length of IO array is %d\n", ionum);
        if (json_object_is_type(ioarray, json_type_array)) {
            for (i = 0; i < ionum; i++) {
                struct json_object* ioobj = json_object_array_get_idx(ioarray, i);
                struct json_object* x;
                if (json_object_object_get_ex(ioobj, "type", &x) == true) {
                    mraaobjs[i].type = json_object_get_string(x);
                }
                if (json_object_object_get_ex(ioobj, "index", &x) == true) {
                    mraaobjs[i].index = json_object_get_int(x);
                }
                if (json_object_object_get_ex(ioobj, "raw", &x) == true) {
                    mraaobjs[i].raw = json_object_get_boolean(x);
                }
                if (json_object_object_get_ex(ioobj, "label", &x) == true) {
                    mraaobjs[i].index = json_object_get_int(x);
                }
#if 1
                json_object_object_foreach(ioobj, key, val) {
//                    fprintf(stderr, "key: %s\n", key);
//                    fprintf(stderr, "val: %s\n", json_object_get_string(val));
                    if (strncmp(key, "type", 4) == 0) {
                        if (strncmp(json_object_get_string(val), "gpio", 4) == 0) {
                            // mraa_gpio_context gpio = mraa_gpio_init(13);
                            // mraa_result_t  r = mraa_gpio_owner(gpio, 0);
                            // if (r != MRAA_SUCCESS) {
                            //     mraa_result_print(r);
                            // }
                            printf("set up gpio here\n");

                        } else if (strncmp(json_object_get_string(val), "i2c", 3) == 0) {
                            printf("set up i2c here\n");
                        }
                    }
                }
#endif
            }
        } else {
            fprintf(stderr, "IO array incorrectly parsed\n");
        }
        //printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(ioarray, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
    }
}

int
check_version(struct json_object* jobj){
    struct json_object* imraa_version;
    if (json_object_object_get_ex(jobj, "version", &imraa_version) == true) {
        if (json_object_is_type(imraa_version, json_type_string)) {
            printf("imraa version is %s good\n", json_object_get_string(imraa_version));
            //TODO check version?
        } else {
            fprintf(stderr, "version string incorrectly parsed\n");
            return -1;
        }
    }
    return 0;
}

void
print_version()
{
    fprintf(stdout, "Version %s on %s\n", mraa_get_version(), mraa_get_platform_name());
}

void
print_help()
{
    fprintf(stdout, "version           Get mraa version and board name\n");
}

void
print_command_error()
{
    fprintf(stdout, "Invalid command, options are:\n");
    print_help();
    exit(EXIT_FAILURE);
}

int
main(int argc, char** argv)
{
    char* buffer = NULL;
    char* imraa_conf_file = IMRAA_CONF_FILE;
    long fsize;
    int i = 0;
    uint32_t ionum = 0;

    if (argc > 2) {
        print_command_error();
    }

    if (argc > 1) {
        if (strcmp(argv[1], "help") == 0) {
            print_help();
        } else if (strcmp(argv[1], "version") == 0) {
            print_version();
        } else {
            imraa_conf_file = argv[1];
        }
    }

    FILE *fh = fopen(imraa_conf_file, "r");
    if (fh == NULL) {
        fprintf(stderr, "Failed to open configuration file\n");
        return EXIT_FAILURE;
    }

    fseek (fh, 0, SEEK_END);
    fsize = ftell (fh) + 1;
    fseek (fh, 0, SEEK_SET);
    buffer = calloc (fsize, sizeof (char));
    if (buffer != NULL) {
        fread (buffer, sizeof (char), fsize, fh);
    }

    imraa_init();

    json_object* jobj = json_tokener_parse(buffer);
    if (check_version(jobj) != 0 ) {
        printf("version of configuaration file is not compatible, please check again\n");
    } else {
        handle_subplatform(jobj);
        handle_IO(jobj);
    }
    fclose(fh);
    return EXIT_SUCCESS;
}

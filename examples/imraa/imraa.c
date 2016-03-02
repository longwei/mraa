/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
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
#include <json-c/json.h>
 
#include <mraa/gpio.h>

#define IMRAA_CONF_FILE "/etc/imraa.conf"
// #define IMRAA_LOCK_FILE "/Users/longwei/imraa.lock"

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

typedef struct mraa_io_objects_t
{
    char* type;
    int index;
    bool raw;
    char* label;
} mraa_io_objects_t;

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

    struct json_object* imraa_version;
    if (json_object_object_get_ex(jobj, "version", &imraa_version) == true) {
        if (json_object_is_type(imraa_version, json_type_string)) {
            printf("imraa version is %s\n", json_object_get_string(imraa_version));
        } else {
            fprintf(stderr, "version string incorrectly parsed\n");
        }
    }

    struct mraa_io_objects_t* mraaobjs;

    struct json_object* ioarray;
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
                            printf("set up gpio and context owner for blink_onboard example\n");
                            mraa_gpio_context mux = mraa_gpio_init(13);
                            mraa_result_t  r = mraa_gpio_owner(mux, 0);
                            if (r != MRAA_SUCCESS) {
                                mraa_result_print(r);
                            }

                        } else if (strncmp(json_object_get_string(val), "i2c", 3) == 0) {
                            printf("i2c\n");
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

    fclose(fh);
    return EXIT_SUCCESS;
}

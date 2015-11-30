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

#include <yaml.h>

#include <mraa/gpio.h>

#define IMRAA_CONF_FILE "/etc/imraa.conf"

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
parse_yaml()
{
    return 0;
}

int
main(int argc, char** argv)
{
    if (argc > 1) {
        print_command_error();
    }

#if 0
    if (argc > 1) {
        if (strcmp(argv[1], "help") == 0) {
            print_help();
        } else if (strcmp(argv[1], "version") == 0) {
            print_version();
        }
    }
#endif

    FILE *fh = fopen(IMRAA_CONF_FILE, "r");
    if (fh == NULL) {
        fprintf(stderr, "Failed to open configuration file\n");
        return EXIT_FAILURE;
    }
    yaml_parser_t parser;

    /* Initialize parser */
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser!\n");
    }

    if (fh == NULL) {
        fprintf(stderr, "Failed to open file!\n");
    }

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    parse_yaml();

    yaml_parser_delete(&parser);
    fclose(fh);

    return EXIT_SUCCESS;
}

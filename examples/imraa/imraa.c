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
main(int argc, char** argv)
{
    if (argc > 2) {
        print_command_error();
    }

    char* imraa_conf_file = IMRAA_CONF_FILE;

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

    yaml_token_t token;
    yaml_parser_t parser;

    /* Initialize parser */
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser!\n");
        fclose(fh);
        return EXIT_FAILURE;
    }

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            /* Stream start/end */
            case YAML_STREAM_START_TOKEN: puts("STREAM START"); break;
            case YAML_STREAM_END_TOKEN:   puts("STREAM END");   break;
            /* Token types (read before actual token) */
            case YAML_KEY_TOKEN:   printf("(Key token)   "); break;
            case YAML_VALUE_TOKEN: printf("(Value token) "); break;
            /* Block delimeters */
            case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("<b>Start Block (Sequence)</b>"); break;
            case YAML_BLOCK_ENTRY_TOKEN:          puts("<b>Start Block (Entry)</b>");    break;
            case YAML_BLOCK_END_TOKEN:            puts("<b>End block</b>");              break;
            /* Data */
            case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
            case YAML_SCALAR_TOKEN:  printf("scalar %s \n", token.data.scalar.value); break;
            /* Others */
            default:
                printf("Got token of type %d\n", token.type);
        }
    if(token.type != YAML_STREAM_END_TOKEN)
        yaml_token_delete(&token);
    } while(token.type != YAML_STREAM_END_TOKEN);

    // cleanup
    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(fh);

    return EXIT_SUCCESS;
}

/**
 * Created by longwei on 03/03/16.
 * Derived from intel-arduino-tools arduino101load go script
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char *argv[]) {
    printf("starting download script\n");
    // ARG 1: Path to binaries of dfu, ".../intel-arduino-tools-linux64/x86/bin"
    // ARG 2: Sketch File to download, ".../test.ino.bin"
    // ARG 3: TTY port to use. "/dev/ttyACM0"
    // ARG 4: quiet/verbose
    const char* bin_path = argv[1];
    const char* bin_file_name = argv[2];
    const char* com_port = argv[3];
    bool verbosity = (strncmp(argv[4], "verbose", 5) == 0);

    if (verbosity) {
        printf("bin path: %s\n", bin_path);
        printf("bin img file: %s\n", bin_file_name);
        printf("serial port: %s\n", com_port);
    }


    bool board_found = false;
    size_t bin_path_len = strlen(bin_path);
    const char* dfu_list = "/dfu-util -d,8087:0ABA -l";
    char* full_dfu_list = (char*) malloc (bin_path_len
                                              + strlen(dfu_list) + 1);
    strncat(full_dfu_list, bin_path, strlen(bin_path));
    strncat(full_dfu_list, dfu_list, strlen(dfu_list));
    printf("%s\n",full_dfu_list);

    char *ln = NULL;
    size_t len = 0;

    for(int i = 0; i < 10 && board_found == false; i++) {
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

    const char* dfu_upload = "/dfu-util -d,8087:0ABA -D ";
    const char* dfu_option = " -v -a 7 -R";
    char* full_dfu_upload = (char*) malloc (bin_path_len
        + strlen(dfu_list)+ strlen(dfu_option) + strlen(bin_file_name) + 1);
    strncat(full_dfu_upload, bin_path, strlen(bin_path));
    strncat(full_dfu_upload, dfu_upload, strlen(dfu_upload));
    strncat(full_dfu_upload, bin_file_name, strlen(bin_file_name));
    strncat(full_dfu_upload, dfu_option, strlen(dfu_option));

    if (verbosity) {
        printf("uploading...%s\n",full_dfu_upload);
    }
    //dfu-util -d,8087:0ABA -D ~/blink_test.ino.bin -v -a 7 -R
    int status = system(full_dfu_upload);
    free(full_dfu_upload);
    free(full_dfu_list);
    if (status != 0) {
        printf("ERROR: Upload failed on %s", com_port);
        exit(1);
    }
    printf("SUCCESS: Sketch will execute in about 5 seconds.\n");
    return 0;
}

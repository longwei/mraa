#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mraa/uart.h>



int reset_ping(const char* modem)
{
    mraa_uart_context uart;
    uart = mraa_uart_init_raw("/dev/ttyACM0");
    mraa_uart_set_baudrate(uart, 1200);

    if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return 1;
    }
    mraa_uart_stop(uart);
    mraa_deinit();
    fprintf(stderr, "SUCCESS\n");
    return 0;
}

int
main(int argc, char** argv)
{
    reset_ping("/dev/ttyACM0");
    return 0;
}
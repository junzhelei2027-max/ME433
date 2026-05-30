#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

/*
 * Pico USB <-> UART bridge
 * USB serial on the computer side, UART0 on GP0/GP1 for the STM32 side.
 */
#define LINK_UART       uart0
#define LINK_BAUD       115200
#define LINK_TX_PIN     0       // GP0 -> STM32 PA1
#define LINK_RX_PIN     1       // GP1 <- STM32 PA0

static void setup_stm32_uart(void)
{
    uart_init(LINK_UART, LINK_BAUD);
    gpio_set_function(LINK_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LINK_RX_PIN, GPIO_FUNC_UART);

    // Keep characters exactly as received.
    uart_set_translate_crlf(LINK_UART, false);
}

static void forward_usb_to_uart(void)
{
    int usb_char = getchar_timeout_us(0);

    if (usb_char >= 0) {
        uart_putc_raw(LINK_UART, (char)usb_char);
    }
}

static void forward_uart_to_usb(void)
{
    while (uart_is_readable(LINK_UART)) {
        char uart_char = uart_getc(LINK_UART);
        putchar_raw(uart_char);
    }
}

int main(void)
{
    // Use only USB stdio; do not initialize default UART stdio.
    stdio_usb_init();

    setup_stm32_uart();

    // Give the computer time to detect the USB serial port.
    sleep_ms(2000);

    while (true) {
        forward_usb_to_uart();
        forward_uart_to_usb();
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// HW18 haptic paddle position sensing
// Reads AS5600 angle over I2C and accepts desired-current commands from Python.

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define AS5600_ADDR 0x36
#define AS5600_ANGLE_REG 0x0E

#define SERIAL_BUF_LEN 40

static float desired_current_A = 0.0f;

uint16_t get_as5600_raw_angle(void) {
    uint8_t reg = AS5600_ANGLE_REG;
    uint8_t data[2];

    int status = i2c_write_blocking(I2C_PORT, AS5600_ADDR, &reg, 1, true);
    if (status < 0) {
        return 0xFFFF;
    }

    status = i2c_read_blocking(I2C_PORT, AS5600_ADDR, data, 2, false);
    if (status < 0) {
        return 0xFFFF;
    }

    uint16_t raw_angle = ((uint16_t)data[0] << 8) | data[1];
    return raw_angle & 0x0FFF;   // AS5600 angle is 12 bits
}

void check_usb_command(void) {
    static char rx_buf[SERIAL_BUF_LEN];
    static int rx_index = 0;

    int ch = getchar_timeout_us(0);
    while (ch != PICO_ERROR_TIMEOUT) {
        if (ch == '\n' || ch == '\r') {
            rx_buf[rx_index] = '\0';

            // Python sends commands like: I,0.125
            if (rx_buf[0] == 'I' && rx_buf[1] == ',') {
                desired_current_A = strtof(&rx_buf[2], NULL);
            }

            rx_index = 0;
        } else if (rx_index < SERIAL_BUF_LEN - 1) {
            rx_buf[rx_index++] = (char)ch;
        } else {
            rx_index = 0;
        }

        ch = getchar_timeout_us(0);
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Keep this CSV format the same so the Python graphics script can read it.
    printf("time_ms,raw,angle_deg\n");

    while (true) {
        check_usb_command();

        uint16_t raw_angle = get_as5600_raw_angle();
        if (raw_angle != 0xFFFF) {
            float angle_deg = (raw_angle * 360.0f) / 4096.0f;
            printf("%lu,%u,%.2f\n", to_ms_since_boot(get_absolute_time()), raw_angle, angle_deg);
        }

        // 20 ms update period, about 50 Hz for position and computer graphics.
        sleep_ms(20);
    }

    return 0;
}

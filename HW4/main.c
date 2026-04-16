#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define HEARTBEAT_PIN 15
#define ADC_PIN 26
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

void drawChar(unsigned char x, unsigned char y, char c) {
    if (c < 0x20 || c > 0x7F) {
        c = '?';
    }

    int index = c - 0x20;

    for (int col = 0; col < 5; col++) {
        unsigned char bits = ASCII[index][col];
        for (int row = 0; row < 8; row++) {
            unsigned char color = (bits >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, color);
        }
    }

    for (int row = 0; row < 8; row++) {
        ssd1306_drawPixel(x + 5, y + row, 0);
    }
}

void drawMessage(unsigned char x, unsigned char y, char *message) {
    unsigned char cursor_x = x;
    while (*message != '\0') {
        drawChar(cursor_x, y, *message);
        cursor_x += 6;
        message++;
    }
}

int main() {
    stdio_init_all();

    gpio_init(HEARTBEAT_PIN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);
    gpio_put(HEARTBEAT_PIN, 0);

    i2c_init(i2c_default, 400000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    sleep_ms(100);
    ssd1306_setup();

    bool heartbeat = false;
    bool blink_pixel = false;
    unsigned int last_frame_us = to_us_since_boot(get_absolute_time());
    float fps = 0.0f;
    unsigned int last_toggle_us = 0;

    while (true) {
        unsigned int now_us = to_us_since_boot(get_absolute_time());
        unsigned int dt_us = now_us - last_frame_us;
        last_frame_us = now_us;
        if (dt_us > 0) {
            fps = 1000000.0f / (float)dt_us;
        }

        unsigned int adc_raw = adc_read();
        float volts = (3.3f * adc_raw) / 4095.0f;

        if ((now_us - last_toggle_us) >= 500000) {
            last_toggle_us = now_us;
            heartbeat = !heartbeat;
            blink_pixel = !blink_pixel;
            gpio_put(HEARTBEAT_PIN, heartbeat);
        }

        ssd1306_clear();
        ssd1306_drawPixel(0, 0, blink_pixel ? 1 : 0);

        char line1[30];
        char line2[30];
        char line3[30];
        char line4[30];

        sprintf(line1, "HW4 SSD1306 DEMO");
        sprintf(line2, "ADC0 = %.3f V", volts);
        sprintf(line3, "RAW  = %4u", adc_raw);
        sprintf(line4, "FPS  = %.1f", fps);

        drawMessage(0, 0, line1);
        drawMessage(0, 8, line2);
        drawMessage(0, 16, line3);
        drawMessage(0, 24, line4);

        ssd1306_update();
        sleep_ms(20);
    }
}

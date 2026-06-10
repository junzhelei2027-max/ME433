#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define HX711_SCK_PIN 14
#define HX711_DT_PIN  15

// IIR filter coefficients
#define IIR_A 0.90f
#define IIR_B 0.10f

static void hx711_init(void) {
    gpio_init(HX711_SCK_PIN);
    gpio_set_dir(HX711_SCK_PIN, GPIO_OUT);
    gpio_put(HX711_SCK_PIN, 0);

    gpio_init(HX711_DT_PIN);
    gpio_set_dir(HX711_DT_PIN, GPIO_IN);
}

// Wait until HX711 data ready (DT goes low)
static bool hx711_wait_ready_timeout_us(uint32_t timeout_us) {
    absolute_time_t start = get_absolute_time();
    while (gpio_get(HX711_DT_PIN)) {
        if (absolute_time_diff_us(start, get_absolute_time()) > timeout_us) {
            return false;
        }
    }
    return true;
}

// Read one 24-bit sample from HX711, keep channel A gain 128
static int32_t hx711_read_raw(void) {
    uint32_t raw = 0;

    // Wait until data is ready (DT goes low)
    while (gpio_get(HX711_DT_PIN)) {
        tight_loop_contents();
    }

    // Read 24 bits, MSB first
    // Data is stable AFTER the falling edge of SCK
    for (int i = 0; i < 24; i++) {
        gpio_put(HX711_SCK_PIN, 1);
        sleep_us(2);
        gpio_put(HX711_SCK_PIN, 0);
        sleep_us(2);  // wait for data to stabilize after falling edge

        raw <<= 1;
        if (gpio_get(HX711_DT_PIN)) {
            raw |= 1;
        }
    }

    // 25th pulse: set gain for next reading to 128
    gpio_put(HX711_SCK_PIN, 1);
    sleep_us(2);
    gpio_put(HX711_SCK_PIN, 0);
    sleep_us(2);

    // Sign extend 24-bit two's complement to 32-bit
    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

int main() {
    stdio_init_all();
    hx711_init();

    // Give USB serial time to enumerate
    sleep_ms(2000);

    printf("HW14 HX711 ready\r\n");
    printf("Send an integer sample count, e.g. 200\r\n");

    char line[64];
    float filtered = 0.0f;
    bool first_sample = true;

    while (1) {
        // Wait for a line from the computer
        if (fgets(line, sizeof(line), stdin) != NULL) {
            int n_samples = atoi(line);

            if (n_samples <= 0) {
                printf("Invalid sample count\r\n");
                continue;
            }

            // Reset filter state for each new collection
            first_sample = true;

            printf("START,%d\r\n", n_samples);

            absolute_time_t t0 = get_absolute_time();

            for (int i = 0; i < n_samples; i++) {
                // Wait for data ready with timeout
                if (!hx711_wait_ready_timeout_us(200000)) {
                    printf("TIMEOUT\r\n");
                    break;
                }

                int32_t raw = hx711_read_raw();

                if (first_sample) {
                    filtered = (float)raw;
                    first_sample = false;
                } else {
                    filtered = IIR_A * filtered + IIR_B * (float)raw;
                }

                uint32_t time_ms = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(t0);

                // CSV format: time_ms,raw,filtered
                printf("%lu,%ld,%.3f\r\n", time_ms, raw, filtered);
            }

            printf("END\r\n");
        }
    }

    return 0;
}
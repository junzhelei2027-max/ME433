/*
 * HW10 - Pico sensor streaming for Python visualization/game
 *
 * Hardware:
 *  - Potentiometer center pin -> GP26 / ADC0
 *  - Potentiometer side pins  -> 3V3 and GND
 *  - Optional pushbutton      -> GP15 and GND
 *  - Onboard LED             -> GP25
 *
 * Serial protocol:
 *  Pico prints one CSV line about 50 times per second:
 *      adc,button
 *  Example:
 *      2134,0
 *      2860,1
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 26
#define ADC_CHANNEL 0
#define BUTTON_PIN 15
#define LED_PIN 25

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_CHANNEL);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    sleep_ms(2000); // give USB serial time to connect

    while (true) {
        uint16_t adc_value = adc_read();       // 0 to 4095
        int button_pressed = !gpio_get(BUTTON_PIN); // active low

        gpio_put(LED_PIN, button_pressed);

        printf("%u,%d\n", adc_value, button_pressed);
        sleep_ms(20); // about 50 Hz
    }

    return 0;
}

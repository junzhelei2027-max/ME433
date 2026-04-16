#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 15
#define PERIOD_US 20000
#define MIN_PULSE_US 300
#define MAX_PULSE_US 2400

void servo_set_angle(float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_us = MIN_PULSE_US + (angle / 180.0f) * (MAX_PULSE_US - MIN_PULSE_US);
    pwm_set_gpio_level(SERVO_PIN, (uint16_t)pulse_us);
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);
    pwm_config_set_wrap(&config, PERIOD_US - 1);
    pwm_init(slice_num, &config, true);

    while (true) {
        for (float angle = 0; angle <= 180; angle += 2) {
            servo_set_angle(angle);
            sleep_ms(30);
        }
        for (float angle = 180; angle >= 0; angle -= 2) {
            servo_set_angle(angle);
            sleep_ms(30);
        }
    }
}

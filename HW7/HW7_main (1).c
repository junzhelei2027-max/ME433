#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#define DAC_SPI spi0

#define DAC_MISO_PIN 16
#define DAC_CS_PIN   17
#define DAC_SCK_PIN  18
#define DAC_MOSI_PIN 19
#define DAC_SHDN_PIN 21

#define DAC_VREF          3.3f
#define DAC_MAX_COUNT     1023.0f
#define SAMPLE_RATE_HZ    1000.0f
#define SAMPLE_DELAY_US   1000

#define LOW_VOLTAGE       0.15f
#define HIGH_VOLTAGE      2.90f
#define SINE_FREQ_HZ      2.0f
#define TRIANGLE_FREQ_HZ  1.0f
#define TWO_PI            6.28318530718f

static void set_cs(int level) {
    asm volatile("nop \n nop \n nop");
    gpio_put(DAC_CS_PIN, level);
    asm volatile("nop \n nop \n nop");
}

static uint16_t clamp_and_scale(float voltage) {
    if (voltage < 0.0f) {
        voltage = 0.0f;
    } else if (voltage > DAC_VREF) {
        voltage = DAC_VREF;
    }

    float scaled = voltage * DAC_MAX_COUNT / DAC_VREF;
    return (uint16_t)(scaled + 0.5f);
}

static uint16_t make_dac_word(uint8_t channel, uint16_t value) {
    uint16_t word = 0x3000;             // 1x gain and active output

    if (channel != 0) {
        word |= 0x8000;                 // select DAC B
    }

    word |= (value & 0x03ffu) << 2;     // MCP4912 uses bits 11:2 for data
    return word;
}

static void write_dac(uint8_t channel, uint16_t value) {
    uint16_t word = make_dac_word(channel, value);
    uint8_t bytes[2] = {
        (uint8_t)(word >> 8),
        (uint8_t)(word & 0xff)
    };

    set_cs(0);
    spi_write_blocking(DAC_SPI, bytes, 2);
    set_cs(1);
}

static float triangle_from_phase(float phase) {
    if (phase < 0.5f) {
        return 2.0f * phase;
    }
    return 2.0f * (1.0f - phase);
}

static void advance_phase(float *phase, float step) {
    *phase += step;
    if (*phase >= 1.0f) {
        *phase -= 1.0f;
    }
}

static void init_dac_spi(void) {
    spi_init(DAC_SPI, 1000 * 1000);
    spi_set_format(DAC_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(DAC_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(DAC_SCK_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(DAC_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(DAC_CS_PIN);
    gpio_set_dir(DAC_CS_PIN, GPIO_OUT);
    gpio_put(DAC_CS_PIN, 1);

    gpio_init(DAC_SHDN_PIN);
    gpio_set_dir(DAC_SHDN_PIN, GPIO_OUT);
    gpio_put(DAC_SHDN_PIN, 1);
}

int main(void) {
    stdio_init_all();
    init_dac_spi();

    const float center_v = 0.5f * (LOW_VOLTAGE + HIGH_VOLTAGE);
    const float amplitude_v = 0.5f * (HIGH_VOLTAGE - LOW_VOLTAGE);
    const float sine_step = SINE_FREQ_HZ / SAMPLE_RATE_HZ;
    const float triangle_step = TRIANGLE_FREQ_HZ / SAMPLE_RATE_HZ;

    float sine_phase = 0.5f * sine_step;
    float triangle_phase = 0.0f;
    absolute_time_t wake_time = get_absolute_time();

    while (true) {
        float sine_v = center_v + amplitude_v * sinf(TWO_PI * sine_phase);
        float triangle_v = LOW_VOLTAGE +
                           (HIGH_VOLTAGE - LOW_VOLTAGE) * triangle_from_phase(triangle_phase);

        write_dac(0, clamp_and_scale(sine_v));
        write_dac(1, clamp_and_scale(triangle_v));

        advance_phase(&sine_phase, sine_step);
        advance_phase(&triangle_phase, triangle_step);

        wake_time = delayed_by_us(wake_time, SAMPLE_DELAY_US);
        sleep_until(wake_time);
    }

    return 0;
}

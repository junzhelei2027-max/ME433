#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

// SPI0 connections shared by the SRAM and the DAC
#define SPI_CH          spi0
#define GP_MISO         16
#define GP_MEM_CS       17
#define GP_SCK          18
#define GP_MOSI         19
#define GP_DAC_CS       20
#define GP_DAC_SHDN     21

// 23K256 SRAM command bytes
#define SRAM_CMD_READ   0x03
#define SRAM_CMD_WRITE  0x02
#define SRAM_CMD_WRMR   0x01
#define SRAM_SEQ_MODE   0x40

// Sine wave settings
#define DAC_REF_VOLTAGE 3.3f
#define NUM_SAMPLES     1000
#define LOW_LIMIT_V     0.20f
#define HIGH_LIMIT_V    2.80f

static inline void pull_cs_low(uint pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void release_cs(uint pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin, 1);
    asm volatile("nop \n nop \n nop");
}

static uint16_t voltage_to_dac_count(float vout) {
    if (vout < 0.0f) {
        vout = 0.0f;
    } else if (vout > DAC_REF_VOLTAGE) {
        vout = DAC_REF_VOLTAGE;
    }

    int result = (int)lroundf((vout * 1023.0f) / DAC_REF_VOLTAGE);
    if (result < 0) {
        result = 0;
    } else if (result > 1023) {
        result = 1023;
    }

    return (uint16_t)result;
}

static void set_chip_select_outputs(void) {
    const uint cs_pins[] = {GP_MEM_CS, GP_DAC_CS};

    for (int i = 0; i < 2; i++) {
        gpio_init(cs_pins[i]);
        gpio_set_dir(cs_pins[i], GPIO_OUT);
        gpio_put(cs_pins[i], 1);
    }

    gpio_init(GP_DAC_SHDN);
    gpio_set_dir(GP_DAC_SHDN, GPIO_OUT);
    gpio_put(GP_DAC_SHDN, 1);
}

static void setup_spi0(void) {
    spi_init(SPI_CH, 1000 * 1000);

    gpio_set_function(GP_MISO, GPIO_FUNC_SPI);
    gpio_set_function(GP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(GP_MOSI, GPIO_FUNC_SPI);

    spi_set_format(SPI_CH, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

static void dac_send_16(uint16_t word) {
    uint8_t packet[2] = {
        (uint8_t)(word >> 8),
        (uint8_t)(word & 0xFF)
    };

    pull_cs_low(GP_DAC_CS);
    spi_write_blocking(SPI_CH, packet, 2);
    release_cs(GP_DAC_CS);
}

static void sram_set_sequential(void) {
    uint8_t command[2] = {SRAM_CMD_WRMR, SRAM_SEQ_MODE};

    pull_cs_low(GP_MEM_CS);
    spi_write_blocking(SPI_CH, command, 2);
    release_cs(GP_MEM_CS);
}

static void sram_write(uint16_t start_addr, const uint8_t *src, size_t count) {
    uint8_t command[3] = {
        SRAM_CMD_WRITE,
        (uint8_t)(start_addr >> 8),
        (uint8_t)(start_addr & 0xFF)
    };

    pull_cs_low(GP_MEM_CS);
    spi_write_blocking(SPI_CH, command, 3);
    spi_write_blocking(SPI_CH, src, count);
    release_cs(GP_MEM_CS);
}

static void sram_read(uint16_t start_addr, uint8_t *dest, size_t count) {
    uint8_t command[3] = {
        SRAM_CMD_READ,
        (uint8_t)(start_addr >> 8),
        (uint8_t)(start_addr & 0xFF)
    };

    pull_cs_low(GP_MEM_CS);
    spi_write_blocking(SPI_CH, command, 3);
    spi_read_blocking(SPI_CH, 0x00, dest, count);
    release_cs(GP_MEM_CS);
}

static void sram_store_word(uint16_t byte_addr, uint16_t word) {
    uint8_t bytes[2] = {
        (uint8_t)(word >> 8),
        (uint8_t)(word & 0xFF)
    };

    sram_write(byte_addr, bytes, 2);
}

static uint16_t sram_load_word(uint16_t byte_addr) {
    uint8_t bytes[2];
    sram_read(byte_addr, bytes, 2);

    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint16_t make_dac_a_word(uint16_t ten_bit_value) {
    uint16_t word = 0;

    word |= (1u << 13);                    // 1x gain
    word |= (1u << 12);                    // output enabled
    word |= (uint16_t)((ten_bit_value & 0x03FFu) << 2);

    return word;
}

static void fill_sine_table_in_sram(void) {
    const float center_v = (HIGH_LIMIT_V + LOW_LIMIT_V) / 2.0f;
    const float amplitude_v = (HIGH_LIMIT_V - LOW_LIMIT_V) / 2.0f;

    for (int n = 0; n < NUM_SAMPLES; n++) {
        float angle = 2.0f * (float)M_PI * (((float)n + 0.5f) / (float)NUM_SAMPLES);
        float sample_v = center_v + amplitude_v * sinf(angle);

        uint16_t dac_count = voltage_to_dac_count(sample_v);
        uint16_t output_word = make_dac_a_word(dac_count);

        sram_store_word((uint16_t)(n * 2), output_word);
    }
}

int main(void) {
    stdio_init_all();

    setup_spi0();
    set_chip_select_outputs();
    sram_set_sequential();
    fill_sine_table_in_sram();

    while (true) {
        for (int n = 0; n < NUM_SAMPLES; n++) {
            uint16_t next_word = sram_load_word((uint16_t)(n * 2));
            dac_send_16(next_word);
            sleep_ms(1);
        }
    }

    return 0;
}

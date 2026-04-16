#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BAUDRATE 100000

#define MCP23008_ADDR 0x20

// MCP23008 registers
#define MCP_IODIR 0x00
#define MCP_GPIO  0x09
#define MCP_OLAT  0x0A

#define HEARTBEAT_PIN 15

void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    unsigned char buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

unsigned char readPin(unsigned char address, unsigned char reg) {
    unsigned char value;
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, address, &value, 1, false);
    return value;
}

void mcp23008_init(unsigned char address) {
    // bit = 1 means input, bit = 0 means output
    // GP7 output, GP0-GP6 input
    unsigned char iodir = 0b01111111;
    setPin(address, MCP_IODIR, iodir);

    // Start outputs low
    setPin(address, MCP_OLAT, 0x00);
}

void mcp23008_set_gp7(unsigned char address, bool on) {
    unsigned char olat = readPin(address, MCP_OLAT);

    if (on) {
        olat |= (1 << 7);
    } else {
        olat &= ~(1 << 7);
    }

    setPin(address, MCP_OLAT, olat);
}

bool mcp23008_read_gp0(unsigned char address) {
    unsigned char gpio = readPin(address, MCP_GPIO);
    return (gpio & (1 << 0)) != 0;
}

int main() {
    stdio_init_all();

    gpio_init(HEARTBEAT_PIN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);
    gpio_put(HEARTBEAT_PIN, 0);

    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    sleep_ms(50);
    mcp23008_init(MCP23008_ADDR);

    bool heartbeat_state = false;

    while (true) {
        bool gp0_is_high = mcp23008_read_gp0(MCP23008_ADDR);
        bool button_pressed = !gp0_is_high;

        if (button_pressed) {
            mcp23008_set_gp7(MCP23008_ADDR, true);
        } else {
            mcp23008_set_gp7(MCP23008_ADDR, false);
        }

        heartbeat_state = !heartbeat_state;
        gpio_put(HEARTBEAT_PIN, heartbeat_state);

        sleep_ms(200);
    }
}


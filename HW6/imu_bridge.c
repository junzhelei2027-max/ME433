#include "mpu6050.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define ACTIVE_I2C_BUS i2c0
#define MPU_NOT_FOUND  0xFF
#define WHO_AM_I_OK_1  0x68
#define WHO_AM_I_OK_2  0x98

#define ACCEL_SCALE_LSB_PER_G   16384.0f
#define GYRO_SCALE_LSB_PER_DPS  16.384f
#define TEMP_SCALE              340.0f
#define TEMP_OFFSET             36.53f

uint8_t g_imu_addr = MPU6050_ADDR;

static void write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t value) {
    uint8_t frame[2] = { reg_addr, value };
    i2c_write_blocking(ACTIVE_I2C_BUS, dev_addr, frame, 2, false);
}

static uint8_t read_register(uint8_t dev_addr, uint8_t reg_addr) {
    uint8_t value = 0;
    i2c_write_blocking(ACTIVE_I2C_BUS, dev_addr, &reg_addr, 1, true);
    i2c_read_blocking(ACTIVE_I2C_BUS, dev_addr, &value, 1, false);
    return value;
}

static void read_block(uint8_t dev_addr, uint8_t first_reg, uint8_t *buffer, size_t count) {
    i2c_write_blocking(ACTIVE_I2C_BUS, dev_addr, &first_reg, 1, true);
    i2c_read_blocking(ACTIVE_I2C_BUS, dev_addr, buffer, count, false);
}

static int16_t join_u8_to_i16(uint8_t msb, uint8_t lsb) {
    return (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);
}

static bool id_matches_mpu(uint8_t value) {
    return (value == WHO_AM_I_OK_1) || (value == WHO_AM_I_OK_2);
}

static uint8_t probe_imu_address(void) {
    uint8_t who_am_i = read_register(MPU6050_ADDR, WHO_AM_I);
    if (id_matches_mpu(who_am_i)) {
        return MPU6050_ADDR;
    }

    who_am_i = read_register(MPU6050_ALT_ADDR, WHO_AM_I);
    if (id_matches_mpu(who_am_i)) {
        return MPU6050_ALT_ADDR;
    }

    return MPU_NOT_FOUND;
}

static void configure_device(uint8_t dev_addr) {
    write_register(dev_addr, PWR_MGMT_1, 0x00);
    write_register(dev_addr, ACCEL_CONFIG, 0x00);
    write_register(dev_addr, GYRO_CONFIG, 0x18);
}

static void fill_raw_sensor_fields(mpu6050_data_t *out, const uint8_t raw[14]) {
    out->ax_raw   = join_u8_to_i16(raw[0],  raw[1]);
    out->ay_raw   = join_u8_to_i16(raw[2],  raw[3]);
    out->az_raw   = join_u8_to_i16(raw[4],  raw[5]);
    out->temp_raw = join_u8_to_i16(raw[6],  raw[7]);
    out->gx_raw   = join_u8_to_i16(raw[8],  raw[9]);
    out->gy_raw   = join_u8_to_i16(raw[10], raw[11]);
    out->gz_raw   = join_u8_to_i16(raw[12], raw[13]);
}

static void fill_scaled_sensor_fields(mpu6050_data_t *out) {
    out->ax_g   = out->ax_raw / ACCEL_SCALE_LSB_PER_G;
    out->ay_g   = out->ay_raw / ACCEL_SCALE_LSB_PER_G;
    out->az_g   = out->az_raw / ACCEL_SCALE_LSB_PER_G;

    out->temp_c = out->temp_raw / TEMP_SCALE + TEMP_OFFSET;

    out->gx_dps = out->gx_raw / GYRO_SCALE_LSB_PER_DPS;
    out->gy_dps = out->gy_raw / GYRO_SCALE_LSB_PER_DPS;
    out->gz_dps = out->gz_raw / GYRO_SCALE_LSB_PER_DPS;
}

bool mpu6050_setup(void) {
    uint8_t detected_addr = probe_imu_address();
    if (detected_addr == MPU_NOT_FOUND) {
        return false;
    }

    g_imu_addr = detected_addr;
    configure_device(g_imu_addr);
    return true;
}

void mpu6050_read_all(uint8_t addr, mpu6050_data_t *out) {
    uint8_t raw_bytes[14];

    read_block(addr, ACCEL_XOUT_H, raw_bytes, sizeof(raw_bytes));
    fill_raw_sensor_fields(out, raw_bytes);
    fill_scaled_sensor_fields(out);
}

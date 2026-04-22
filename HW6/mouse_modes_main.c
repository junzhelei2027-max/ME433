#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "mpu6050.h"
#include "usb_descriptors.h"

#define IMU_I2C_BUS         i2c0
#define IMU_I2C_SDA_PIN     0
#define IMU_I2C_SCL_PIN     1
#define IMU_I2C_FREQUENCY   400000

#define TOGGLE_SWITCH_PIN   14
#define STATUS_LED_PIN      16

#define USB_TICK_MS         10
#define REMOTE_STEP_MS      100
#define BUTTON_GUARD_MS     200

enum {
    USB_BLINK_WAITING   = 250,
    USB_BLINK_ACTIVE    = 1000,
    USB_BLINK_SLEEPING  = 2500,
};

typedef enum {
    PROFILE_TILT_MOUSE = 0,
    PROFILE_IDLE_CIRCLE = 1,
} control_profile_t;

static control_profile_t current_profile = PROFILE_TILT_MOUSE;
static mpu6050_data_t sensor_snapshot;
static uint32_t usb_blink_period = USB_BLINK_WAITING;

static const int8_t orbit_x[16] = { 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, 0, 1, 2, 2, 2 };
static const int8_t orbit_y[16] = { 0, 1, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, 0, 1 };

void tud_mount_cb(void) {
    usb_blink_period = USB_BLINK_ACTIVE;
}

void tud_umount_cb(void) {
    usb_blink_period = USB_BLINK_WAITING;
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    usb_blink_period = USB_BLINK_SLEEPING;
}

void tud_resume_cb(void) {
    usb_blink_period = tud_mounted() ? USB_BLINK_ACTIVE : USB_BLINK_WAITING;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer,
                               uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer,
                           uint16_t bufsize) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

static void initialize_mode_io(void) {
    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_PIN, 0);

    gpio_init(TOGGLE_SWITCH_PIN);
    gpio_set_dir(TOGGLE_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(TOGGLE_SWITCH_PIN);
}

static void initialize_imu_bus(void) {
    i2c_init(IMU_I2C_BUS, IMU_I2C_FREQUENCY);
    gpio_set_function(IMU_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(IMU_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(IMU_I2C_SDA_PIN);
    gpio_pull_up(IMU_I2C_SCL_PIN);
    sleep_ms(50);
}

static void show_imu_missing_fault(void) {
    while (1) {
        gpio_put(STATUS_LED_PIN, 1);
        sleep_ms(100);
        gpio_put(STATUS_LED_PIN, 0);
        sleep_ms(100);
    }
}

static void refresh_board_led(void) {
    static uint32_t last_toggle_ms = 0;
    static bool led_on = false;

    uint32_t now = board_millis();
    if (now - last_toggle_ms < usb_blink_period) {
        return;
    }

    last_toggle_ms += usb_blink_period;
    board_led_write(led_on);
    led_on = !led_on;
}

static void apply_profile_indicator(void) {
    gpio_put(STATUS_LED_PIN, current_profile == PROFILE_IDLE_CIRCLE);
}

static void poll_mode_switch(void) {
    static bool previous_level = true;
    static uint32_t last_edge_ms = 0;

    bool current_level = gpio_get(TOGGLE_SWITCH_PIN);
    uint32_t now = board_millis();

    if (previous_level && !current_level && (now - last_edge_ms > BUTTON_GUARD_MS)) {
        last_edge_ms = now;
        current_profile = (current_profile == PROFILE_TILT_MOUSE)
                        ? PROFILE_IDLE_CIRCLE
                        : PROFILE_TILT_MOUSE;
        apply_profile_indicator();
    }

    previous_level = current_level;
}

static int8_t quantize_accel(float accel_g) {
    float abs_value = fabsf(accel_g);
    int8_t direction = (accel_g < 0.0f) ? -1 : 1;

    if (abs_value < 0.08f) {
        return 0;
    }
    if (abs_value < 0.25f) {
        return direction * 2;
    }
    if (abs_value < 0.50f) {
        return direction * 5;
    }
    if (abs_value < 0.75f) {
        return direction * 8;
    }
    return direction * 12;
}

static void get_motion_from_imu(int8_t *dx, int8_t *dy) {
    mpu6050_read_all(g_imu_addr, &sensor_snapshot);
    *dx = quantize_accel(sensor_snapshot.ax_g);
    *dy = -quantize_accel(sensor_snapshot.ay_g);
}

static void get_motion_from_remote_profile(int8_t *dx, int8_t *dy) {
    static uint32_t last_step_ms = 0;
    static uint8_t orbit_index = 0;

    uint32_t now = board_millis();
    if (now - last_step_ms < REMOTE_STEP_MS) {
        *dx = 0;
        *dy = 0;
        return;
    }

    last_step_ms += REMOTE_STEP_MS;
    *dx = orbit_x[orbit_index];
    *dy = orbit_y[orbit_index];
    orbit_index = (orbit_index + 1u) & 0x0Fu;
}

static void service_hid_mouse(void) {
    static uint32_t last_report_ms = 0;

    uint32_t now = board_millis();
    if (now - last_report_ms < USB_TICK_MS) {
        return;
    }
    last_report_ms += USB_TICK_MS;

    if (!tud_hid_ready()) {
        return;
    }

    int8_t delta_x = 0;
    int8_t delta_y = 0;

    if (current_profile == PROFILE_TILT_MOUSE) {
        get_motion_from_imu(&delta_x, &delta_y);
    } else {
        get_motion_from_remote_profile(&delta_x, &delta_y);
    }

    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta_x, delta_y, 0, 0);
}

int main(void) {
    board_init();
    initialize_mode_io();
    initialize_imu_bus();

    if (!mpu6050_setup()) {
        show_imu_missing_fault();
    }

    tusb_init();

    while (1) {
        tud_task();
        refresh_board_led();
        poll_mode_switch();
        service_hid_mouse();
    }

    return 0;
}

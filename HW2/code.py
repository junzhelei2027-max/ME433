import time
import board
import pwmio

SERVO_PIN = board.GP15
PWM_FREQ = 50
PERIOD_US = 20000
MIN_PULSE_US = 300
MAX_PULSE_US = 2400

servo = pwmio.PWMOut(SERVO_PIN, frequency=PWM_FREQ, duty_cycle=0)

def set_angle(angle):
    angle = max(0, min(180, angle))
    pulse_us = MIN_PULSE_US + (angle / 180) * (MAX_PULSE_US - MIN_PULSE_US)
    duty = int((pulse_us / PERIOD_US) * 65535)
    servo.duty_cycle = duty

while True:
    for angle in range(0, 181, 2):
        set_angle(angle)
        time.sleep(0.03)
    for angle in range(180, -1, -2):
        set_angle(angle)
        time.sleep(0.03)

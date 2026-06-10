# HW16 Files

## Files
- hw16_current_plot.py: Python script to send `a` to the STM32 and plot desired vs actual current.
- hw16_circuit_diagram.png: Circuit diagram for the STM32 + DRV8833 + INA219 + servo potentiometer setup.

## Wiring summary
| STM32 / Component | Connection |
|---|---|
| STM32 3.3V | Servo pot VCC, INA219 VCC, DRV8833 SLEEP/EEP |
| STM32 GND | Servo pot GND, INA219 GND, DRV8833 GND, battery pack GND |
| PA0 / ADC1_IN0 | Servo potentiometer signal |
| PA8 / TIM1_CH1 | DRV8833 IN1 and IN3 |
| PA1 / TIM1_CH2 | DRV8833 IN2 and IN4 |
| PA6 / I2C2_SDA | INA219 SDA |
| PA7 / I2C2_SCL | INA219 SCL |
| Battery +6V | DRV8833 VCC |
| DRV8833 OUT1 + OUT3 | One motor lead |
| Other motor lead | INA219 VIN+ |
| INA219 VIN- | DRV8833 OUT2 + OUT4 |

Note: Keep all grounds common.

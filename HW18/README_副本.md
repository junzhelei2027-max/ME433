# HW18 - Haptic Paddle Submission

## What this submission includes

This folder includes the minimum items requested for HW18:

1. `force_displacement_curves.png` - normalized force vs displacement curves for two haptic effects.
2. `mechanical_sketch.png` - drawing of the mechanical paddle parts.
3. `block_diagram.png` - system block diagram from computer graphics to microcontroller, sensors, motor driver, and paddle.
4. `circuit_diagram.png` - proposed circuit diagram with pins and voltages labelled.
5. `main.c` - Pico C code that reads AS5600 paddle angle and accepts desired-current commands from Python.
6. `haptic_graphics.py` - Python graphics code that reads the paddle angle, plots the haptic effect, calculates desired current, and sends it over serial.
7. `CMakeLists.txt` - Pico build file.

## Haptic effect equations

The normalized paddle position is

`x = angle_deg / 180 - 1`, clipped to the range `[-1, 1]`.

### Bump effect

`F_bump(x) = A (x - x0) exp(-((x - x0)^2)/(2 sigma^2))`

where `A = 4.0`, `x0 = 0.0`, and `sigma = 0.20`. The curve is normalized so the maximum absolute force is 1.

### Detent / toggle effect

`F_detent(x) = kx - bx^3`

where `k = 2.0` and `b = 4.0`. This curve is also normalized to +/- 1.

The desired current command is calculated as

`I_des = I_max * F_normalized`

where `I_max` is set in the Python script with `--imax` and defaults to `0.30 A`.

## How to run

Build and flash the Pico code as usual for ME433. The C code prints CSV angle data:

`time_ms,raw,angle_deg`

Then run the Python graphics script:

```bash
python haptic_graphics.py --port COM5
```

On Mac, the port may look like:

```bash
python haptic_graphics.py --port /dev/tty.usbmodemXXXX
```

Press `b` for the bump effect and `d` for the detent/toggle effect.

## Notes

The code implements paddle position sensing, real-time graphics, haptic effect equations, and desired-current command output. The current controller and physical H-bridge stage are shown in the block/circuit diagrams as the next controller stage. The HW18 page says the controller implementation is optional if there is time, so the drawings and plots are the key required submission items.

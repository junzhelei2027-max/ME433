# HW10 - Graphics in Python with Pico Sensor

## What this project does

This project uses a Raspberry Pi Pico as an external controller for a Pygame Zero visualization/game.

The Pico reads:
- a potentiometer on GP26 / ADC0
- a button on GP15

The Pico streams the sensor data over USB serial as CSV:

```text
adc,button
```

The Python/Pygame Zero program reads the serial data and uses it to control a small game:
- the potentiometer moves the player left and right
- the button makes the player jump
- the player catches falling stars

## Files

- `HW10.c` - Pico C code that reads ADC/button and streams data over serial.
- `HW10_game.py` - Pygame Zero visualization/game that reads Pico serial data.
- `CMakeLists.txt` - Pico SDK build file.
- `README.md` - this explanation.

## Wiring

| Part | Pico pin |
|---|---|
| Potentiometer middle pin | GP26 / ADC0 |
| Potentiometer side pins | 3V3 and GND |
| Pushbutton | GP15 and GND |
| Onboard LED | GP25, no external wiring |

The button uses the Pico internal pull-up resistor, so the button should connect GP15 to GND when pressed.

## How to run the Pico code

Build and copy the UF2 to the Pico using the normal Pico SDK process.

Example:

```bash
mkdir build
cd build
cmake ..
make
```

Then drag `HW10.uf2` onto the Pico when it is in BOOTSEL mode.

## How to run the Python game

Install packages:

```bash
pip install pgzero pyserial
```

Run:

```bash
pgzrun HW10_game.py
```

If the serial port is not detected automatically, edit this line in `HW10_game.py`:

```python
SERIAL_PORT = None
```

For example:

```python
SERIAL_PORT = "COM5"
```

or on Mac:

```python
SERIAL_PORT = "/dev/cu.usbmodem1101"
```

## Canvas video

The short demo video should show:
1. Pico connected to the computer.
2. Serial monitor printing values like `2048,0`.
3. Pygame Zero window open.
4. Turning the potentiometer moves the player.
5. Pressing the button makes the player jump.

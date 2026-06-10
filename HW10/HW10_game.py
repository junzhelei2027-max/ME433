"""
HW10 - Pico controlled Pygame Zero visualization/game

Run:
    pip install pgzero pyserial
    pgzrun HW10_game.py

What it does:
    - Reads potentiometer + button data from the Pico over USB serial.
    - Potentiometer controls the player's horizontal position.
    - Button makes the player jump.
    - If the Pico is not connected, the game still runs in demo mode using the keyboard.

Serial format expected from Pico:
    adc,button
Example:
    2048,0
    3200,1
"""

import math
import random
import time

import pgzrun

try:
    import serial
    import serial.tools.list_ports
except Exception:
    serial = None

WIDTH = 800
HEIGHT = 500

# Change this if automatic port detection does not work.
# Windows example: "COM5"
# Mac example: "/dev/cu.usbmodem1101"
SERIAL_PORT = None
BAUD_RATE = 115200

player_x = WIDTH // 2
player_y = HEIGHT - 80
player_vy = 0
ground_y = HEIGHT - 80

adc_value = 2048
button_value = 0
connected = False
ser = None

score = 0
last_spawn = time.time()
stars = []
message_timer = 0


def find_serial_port():
    """Try to find a likely Pico USB serial port."""
    if serial is None:
        return None

    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        text = f"{p.device} {p.description} {p.manufacturer}".lower()
        if "pico" in text or "rp2" in text or "usb serial" in text or "usbmodem" in text:
            return p.device

    if ports:
        return ports[0].device
    return None


def open_serial():
    global ser, connected

    if serial is None:
        connected = False
        return

    port = SERIAL_PORT or find_serial_port()
    if port is None:
        connected = False
        return

    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=0)
        connected = True
        print("Connected to Pico on", port)
    except Exception as e:
        print("Serial connection failed:", e)
        connected = False
        ser = None


def read_pico():
    """Read the newest CSV line from the Pico."""
    global adc_value, button_value, connected, ser

    if ser is None:
        return

    try:
        # Read all available lines and keep the last valid one.
        while ser.in_waiting:
            line = ser.readline().decode(errors="ignore").strip()
            if "," not in line:
                continue

            parts = line.split(",")
            if len(parts) < 2:
                continue

            adc = int(parts[0])
            btn = int(parts[1])

            adc_value = max(0, min(4095, adc))
            button_value = 1 if btn else 0

    except Exception as e:
        print("Serial read error:", e)
        connected = False
        try:
            ser.close()
        except Exception:
            pass
        ser = None


def spawn_star():
    x = random.randint(30, WIDTH - 30)
    y = -20
    speed = random.uniform(2.0, 4.0)
    stars.append({"x": x, "y": y, "speed": speed})


def update():
    global player_x, player_y, player_vy, score, last_spawn, message_timer
    global adc_value, button_value

    if connected:
        read_pico()
        # Map ADC 0-4095 to screen width.
        player_x = int(40 + (adc_value / 4095.0) * (WIDTH - 80))
    else:
        # Demo keyboard mode if Pico is not attached.
        if keyboard.left:
            player_x -= 6
        if keyboard.right:
            player_x += 6
        player_x = max(40, min(WIDTH - 40, player_x))
        button_value = 1 if keyboard.space else 0
        adc_value = int((player_x - 40) / (WIDTH - 80) * 4095)

    # Button controls jump.
    if button_value and player_y >= ground_y:
        player_vy = -12

    player_y += player_vy
    player_vy += 0.6

    if player_y > ground_y:
        player_y = ground_y
        player_vy = 0

    # Spawn falling stars.
    if time.time() - last_spawn > 0.65:
        spawn_star()
        last_spawn = time.time()

    # Move stars and check collision.
    for s in stars[:]:
        s["y"] += s["speed"]
        if s["y"] > HEIGHT + 20:
            stars.remove(s)
            continue

        dx = s["x"] - player_x
        dy = s["y"] - player_y
        if math.sqrt(dx * dx + dy * dy) < 32:
            stars.remove(s)
            score += 1
            message_timer = 20

    if message_timer > 0:
        message_timer -= 1


def draw_player():
    # simple robot/player shape
    screen.draw.filled_circle((player_x, player_y), 22, "deepskyblue")
    screen.draw.circle((player_x, player_y), 22, "white")
    screen.draw.filled_circle((player_x - 7, player_y - 5), 3, "black")
    screen.draw.filled_circle((player_x + 7, player_y - 5), 3, "black")
    screen.draw.line((player_x - 8, player_y + 8), (player_x + 8, player_y + 8), "black")


def draw():
    screen.clear()
    screen.fill((18, 22, 35))

    screen.draw.text("HW10 Pico Sensor Game", (20, 15), fontsize=36, color="white")
    screen.draw.text(f"ADC: {adc_value}", (20, 58), fontsize=26, color="lightgray")
    screen.draw.text(f"Button: {button_value}", (20, 88), fontsize=26, color="lightgray")
    screen.draw.text(f"Score: {score}", (650, 20), fontsize=34, color="gold")

    if connected:
        screen.draw.text("Pico serial connected", (20, 125), fontsize=24, color="lightgreen")
    else:
        screen.draw.text("Demo mode: Left/Right move, Space jumps", (20, 125), fontsize=24, color="orange")

    # ground
    screen.draw.line((0, ground_y + 25), (WIDTH, ground_y + 25), "white")

    # potentiometer bar
    bar_x = 170
    bar_y = HEIGHT - 35
    bar_w = 460
    screen.draw.rect(Rect((bar_x, bar_y), (bar_w, 12)), "white")
    knob_x = bar_x + int((adc_value / 4095.0) * bar_w)
    screen.draw.filled_circle((knob_x, bar_y + 6), 12, "yellow")
    screen.draw.text("potentiometer position", (bar_x, bar_y - 25), fontsize=20, color="white")

    for s in stars:
        screen.draw.filled_circle((s["x"], s["y"]), 9, "gold")
        screen.draw.circle((s["x"], s["y"]), 9, "white")

    draw_player()

    if message_timer > 0:
        screen.draw.text("+1", (player_x + 25, player_y - 45), fontsize=34, color="gold")


open_serial()
pgzrun.go()

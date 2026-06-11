import argparse
import math
import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# HW18 haptic paddle graphics and desired-current command
# b = bump effect, d = detent/toggle effect

parser = argparse.ArgumentParser()
parser.add_argument("--port", default="COM5", help="serial port for the Pico, for example COM5 or /dev/tty.usbmodemXXXX")
parser.add_argument("--baud", type=int, default=115200)
parser.add_argument("--imax", type=float, default=0.30, help="maximum desired current in amps")
args = parser.parse_args()

serial_link = serial.Serial(args.port, args.baud, timeout=1)

latest_time_ms = 0
latest_raw_angle = 0
latest_angle_deg = 0.0
latest_current_A = 0.0

# Force-displacement curves, normalized to +/- 1.
x_curve = np.linspace(-1.0, 1.0, 500)

# Bump: odd Gaussian derivative shape.
BUMP_GAIN = 4.0
BUMP_CENTER = 0.0
BUMP_WIDTH = 0.20
bump_curve = BUMP_GAIN * (x_curve - BUMP_CENTER) * np.exp(-((x_curve - BUMP_CENTER) ** 2) / (2 * BUMP_WIDTH ** 2))
bump_curve = bump_curve / np.max(np.abs(bump_curve))

# Detent/toggle: cubic spring shape.
DETENT_K = 2.0
DETENT_B = 4.0
detent_curve = DETENT_K * x_curve - DETENT_B * x_curve**3
detent_curve = detent_curve / np.max(np.abs(detent_curve))

selected_effect = "bump"

fig, (angle_ax, force_ax) = plt.subplots(1, 2, figsize=(11, 5))

# Left: paddle angle
angle_ax.set_xlim(-1.2, 1.2)
angle_ax.set_ylim(-1.2, 1.2)
angle_ax.set_aspect("equal")
angle_ax.grid(True)
angle_ax.set_title("Real-Time Paddle Angle")
angle_ax.add_patch(plt.Circle((0, 0), 1.0, fill=False))
angle_line, = angle_ax.plot([0, 1], [0, 0], linewidth=3)
angle_text = angle_ax.text(-1.1, 1.05, "", fontsize=11, va="top")

# Right: force-displacement curve
force_curve, = force_ax.plot(x_curve, bump_curve, linewidth=2, label="bump")
force_ax.axhline(0, linewidth=1)
force_ax.axvline(0, linewidth=1)
force_ax.set_xlim(-1.05, 1.05)
force_ax.set_ylim(-1.1, 1.1)
force_ax.grid(True)
force_ax.set_title("Haptic Effect Curve")
force_ax.set_xlabel("Normalized Displacement")
force_ax.set_ylabel("Normalized Desired Force / Current")
force_marker, = force_ax.plot([0], [0], "ro", markersize=8)
effect_text = force_ax.text(-0.98, 1.02, "Effect: bump", fontsize=11, va="top")

def angle_to_normalized_position(angle_deg):
    # Map 0 to 360 deg into about -1 to +1 over the main 0 to 180 deg paddle range.
    x = (angle_deg / 180.0) - 1.0
    return max(-1.0, min(1.0, x))

def bump_force(x):
    val = BUMP_GAIN * (x - BUMP_CENTER) * math.exp(-((x - BUMP_CENTER) ** 2) / (2 * BUMP_WIDTH ** 2))
    return val / np.max(np.abs(bump_curve))

def detent_force(x):
    val = DETENT_K * x - DETENT_B * x**3
    return val / np.max(np.abs(detent_curve))

def send_desired_current(current_A):
    # Current command that would be used by a downstream current controller.
    # Format: I,<amps>\n
    try:
        serial_link.write(f"I,{current_A:.4f}\n".encode())
    except serial.SerialException:
        pass

def on_key(event):
    global selected_effect
    if event.key == "b":
        selected_effect = "bump"
    elif event.key == "d":
        selected_effect = "detent"

fig.canvas.mpl_connect("key_press_event", on_key)

def update(frame):
    global latest_time_ms, latest_raw_angle, latest_angle_deg, latest_current_A

    while serial_link.in_waiting:
        line = serial_link.readline().decode(errors="ignore").strip()
        parts = line.split(",")
        if len(parts) == 3 and parts[0] != "time_ms":
            try:
                latest_time_ms = int(parts[0])
                latest_raw_angle = int(parts[1])
                latest_angle_deg = float(parts[2])
            except ValueError:
                pass

    theta = math.radians(latest_angle_deg)
    pointer_x = math.cos(theta)
    pointer_y = math.sin(theta)
    angle_line.set_data([0, pointer_x], [0, pointer_y])

    x_norm = angle_to_normalized_position(latest_angle_deg)

    if selected_effect == "bump":
        force_curve.set_data(x_curve, bump_curve)
        normalized_force = bump_force(x_norm)
        effect_text.set_text("Effect: bump   (press 'd' for detent)")
    else:
        force_curve.set_data(x_curve, detent_curve)
        normalized_force = detent_force(x_norm)
        effect_text.set_text("Effect: detent   (press 'b' for bump)")

    latest_current_A = args.imax * normalized_force
    send_desired_current(latest_current_A)

    angle_text.set_text(
        f"time_ms = {latest_time_ms}\n"
        f"raw = {latest_raw_angle}\n"
        f"angle = {latest_angle_deg:.2f} deg\n"
        f"x = {x_norm:.2f}\n"
        f"desired current = {latest_current_A:.3f} A"
    )
    force_marker.set_data([x_norm], [normalized_force])

    return angle_line, angle_text, force_curve, force_marker, effect_text

animation = FuncAnimation(fig, update, interval=50, blit=False)
plt.tight_layout()
plt.show()
serial_link.close()

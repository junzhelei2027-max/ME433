"""
HW16 current control data collection script.
Sends 'a' to the STM32/Nucleo, reads index, desired current, actual current,
and plots desired vs actual current.

Expected STM32 serial output format, one sample per line:
index desired_current actual_current

Example:
0 200 12
1 200 60
2 200 110
...
"""

import argparse
import time
import serial
import matplotlib.pyplot as plt


def parse_line(line):
    """Parse one line from STM32.

    Accepts either whitespace or comma separated data.
    Example valid lines:
        0 200 15
        0,200,15
        idx:0 desired:200 actual:15
    """
    line = line.strip()
    if not line:
        return None

    # Make it tolerant of commas/labels.
    for ch in [",", ":", ";"]:
        line = line.replace(ch, " ")

    parts = line.split()
    nums = []
    for p in parts:
        try:
            nums.append(float(p))
        except ValueError:
            pass

    if len(nums) < 3:
        return None

    return int(nums[0]), nums[1], nums[2]


def main():
    parser = argparse.ArgumentParser(description="HW16 STM32 current control plotter")
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM5 on Windows or /dev/cu.usbmodemXXXX on Mac")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate, default 115200")
    parser.add_argument("--samples", type=int, default=400, help="Number of samples to read, default 400")
    parser.add_argument("--timeout", type=float, default=8.0, help="Read timeout in seconds")
    parser.add_argument("--save", default="hw16_current_plot.png", help="Output plot filename")
    args = parser.parse_args()

    index_data = []
    desired_data = []
    actual_data = []

    with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
        time.sleep(2.0)          # allow board reset / serial connection
        ser.reset_input_buffer()

        print("Sending command 'a' to STM32...")
        ser.write(b"a")

        start_time = time.time()
        while len(index_data) < args.samples and (time.time() - start_time) < args.timeout:
            raw = ser.readline().decode(errors="ignore")
            parsed = parse_line(raw)

            if parsed is None:
                continue

            idx, desired, actual = parsed
            index_data.append(idx)
            desired_data.append(desired)
            actual_data.append(actual)

            print(f"{idx:3d} desired={desired:8.2f} actual={actual:8.2f}")

    if not index_data:
        print("No valid data received. Check serial port, baud rate, and STM32 printf format.")
        return

    plt.figure()
    plt.plot(index_data, desired_data, label="Desired current")
    plt.plot(index_data, actual_data, label="Actual current")
    plt.xlabel("Sample index")
    plt.ylabel("Current (mA)")
    plt.title("HW16 PI Current Control")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(args.save, dpi=200)
    plt.show()

    print(f"Saved plot to {args.save}")


if __name__ == "__main__":
    main()

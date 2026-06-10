import serial
import time
import numpy as np
import matplotlib.pyplot as plt

# =========================
# User settings
# =========================
SERIAL_PORT = "COM5"      # 改成你的 Pico 串口号
BAUD_RATE = 115200
# N_SAMPLES = 400
# TIMEOUT = 10
N_SAMPLES = 100
TIMEOUT = 20

# Output filenames
TIME_PLOT_FILE = "raw_filtered_time.png"
FFT_PLOT_FILE = "raw_filtered_fft.png"


def read_hw14_data(port, baud, n_samples, timeout=10):
    ser = serial.Serial(port, baud, timeout=1)
    time.sleep(2)  # give Pico time after serial open

    # clear any old buffered text
    ser.reset_input_buffer()

    # send sample count
    ser.write(f"{n_samples}\n".encode("utf-8"))

    time_ms = []
    raw_vals = []
    filt_vals = []

    started = False
    t_start = time.time()

    while True:
        if time.time() - t_start > timeout:
            raise TimeoutError("Timed out waiting for Pico data.")

        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if not line:
            continue

        print(line)

        if line.startswith("START"):
            started = True
            continue

        if line.startswith("END"):
            break

        if line.startswith("TIMEOUT"):
            print("Pico reported HX711 timeout.")
            break

        if started:
            parts = line.split(",")
            if len(parts) != 3:
                continue

            try:
                t = float(parts[0])
                raw = float(parts[1])
                filt = float(parts[2])

                time_ms.append(t)
                raw_vals.append(raw)
                filt_vals.append(filt)
            except ValueError:
                continue

    ser.close()

    return np.array(time_ms), np.array(raw_vals), np.array(filt_vals)


def compute_fft(signal, fs):
    n = len(signal)
    signal_zero_mean = signal - np.mean(signal)
    fft_vals = np.fft.rfft(signal_zero_mean)
    freqs = np.fft.rfftfreq(n, d=1.0/fs)
    magnitude = np.abs(fft_vals) / n
    return freqs, magnitude


def main():
    # -------------------------
    # Read data from Pico
    # -------------------------
    time_ms, raw_vals, filt_vals = read_hw14_data(
        SERIAL_PORT, BAUD_RATE, N_SAMPLES, timeout=TIMEOUT
    )

    if len(time_ms) < 2:
        print("Not enough data received.")
        return

    # -------------------------
    # Estimate sampling rate
    # -------------------------
    dt_ms = np.diff(time_ms)
    mean_dt_ms = np.mean(dt_ms)
    fs = 1000.0 / mean_dt_ms

    print(f"\nEstimated sample interval: {mean_dt_ms:.3f} ms")
    print(f"Estimated sampling rate: {fs:.3f} Hz")

    # convert to seconds for plotting
    time_s = time_ms / 1000.0

    # -------------------------
    # Time-domain plot
    # -------------------------
    plt.figure(figsize=(10, 5))
    plt.plot(time_s, raw_vals, label="Raw")
    plt.plot(time_s, filt_vals, label="Filtered")
    plt.xlabel("Time (s)")
    plt.ylabel("Sensor Reading")
    plt.title("HX711 Raw and Filtered Data vs Time")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(TIME_PLOT_FILE, dpi=200)
    plt.show()

    # -------------------------
    # FFT plot
    # -------------------------
    freqs_raw, mag_raw = compute_fft(raw_vals, fs)
    freqs_filt, mag_filt = compute_fft(filt_vals, fs)

    plt.figure(figsize=(10, 5))
    plt.plot(freqs_raw, mag_raw, label="Raw FFT")
    plt.plot(freqs_filt, mag_filt, label="Filtered FFT")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title("HX711 Raw and Filtered FFT")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(FFT_PLOT_FILE, dpi=200)
    plt.show()

    print(f"\nSaved plots:")
    print(f" - {TIME_PLOT_FILE}")
    print(f" - {FFT_PLOT_FILE}")


if __name__ == "__main__":
    main()
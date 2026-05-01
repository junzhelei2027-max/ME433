HW9 brief notes

Sample rates:
sigA: 10000.20 Hz
sigB: 3300.20 Hz
sigC: 2500.13 Hz
sigD: 400.08 Hz

Filter choices:
sigA:
- MAF: 500 points
- IIR: A = 0.995, B = 0.005
- FIR: 1201 taps, low-pass cutoff = 10 Hz, Hamming window

sigB:
- MAF: 300 points
- IIR: A = 0.990, B = 0.010
- FIR: 401 taps, low-pass cutoff = 5 Hz, Hamming window

sigC:
- MAF: 80 points
- IIR: A = 0.980, B = 0.020
- FIR: 401 taps, low-pass cutoff = 20 Hz, Hamming window

sigD:
- MAF: 50 points
- IIR: A = 0.950, B = 0.050
- FIR: 201 taps, low-pass cutoff = 2 Hz, Hamming window

Observation:
The filtered FFTs show the expected low-pass effect. High-frequency components are reduced while the lower-frequency parts of the signals are preserved. The moving average and IIR filters are simple and useful, but the FIR filter gives more intentional control over cutoff frequency.

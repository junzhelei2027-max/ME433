# HW8 - Saving a Sine Wave in SPI RAM and Sending It to a DAC

## Submitted files
- `wave_memory_dac.c`
- `CMakeLists.txt`
- `pico_sdk_import.cmake`

## Pin connections

### SPI0 bus
- GP16: MISO
- GP18: SCK
- GP19: MOSI

### Extra control pins
- GP17: 23K256 SRAM chip select
- GP20: MCP4912 DAC chip select
- GP21: MCP4912 shutdown pin

## 23K256 SRAM wiring
- VCC to 3.3 V
- VSS to GND
- CS to GP17
- SCK to GP18
- SI to GP19
- SO to GP16
- HOLD to 3.3 V
- WP to 3.3 V

## MCP4912 DAC wiring
- VDD to 3.3 V
- VSS to GND
- CS to GP20
- SCK to GP18
- SDI to GP19
- LDAC to GND
- SHDN to GP21
- VREFA to 3.3 V
- VREFB to 3.3 V
- VOUTA to oscilloscope channel 1

## Program description
The program first sets up SPI0 and enables both external devices. The 23K256 SRAM is placed in sequential mode. During startup, the Pico calculates 1000 sine-wave samples between about 0.20 V and 2.80 V. Each sample is converted into a 10-bit DAC value and then packed into the 16-bit command format used by the MCP4912.

The 16-bit DAC words are saved in the external SRAM. After the table is saved, the main loop reads each word back from SRAM and sends it directly to DAC channel A. The loop waits 1 ms between samples, so 1000 samples give a period of about 1 second, which makes the output about 1 Hz.

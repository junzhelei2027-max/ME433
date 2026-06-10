# HW13 - PCB Design

This folder contains the deliverables for HW13.

## Files
- `schematic.png` - schematic image of the MT3608L boost converter breakout board.
- `board_layout.png` - PCB board layout image.

## Design summary
The board is a boost converter breakout using the MT3608L. It is designed to take an input voltage above about 2.2 V, such as 3.3 V or a 3.7 V LiPo battery, and boost it to 5 V.

## Main components
- U1: MT3608L boost converter, SOT23-6
- C1: 22 uF X7R input capacitor
- C2: 22 uF X7R output capacitor
- L1: 4.7 uH inductor
- D1: Schottky diode
- R1: 110 kOhm 1% feedback resistor
- R2: 15 kOhm 1% feedback resistor
- R3: 20 kOhm EN pull-up resistor
- LED1: red power indicator LED
- R4: 1 kOhm LED resistor

## Submission
Submit the GitHub folder link for `HW13` on Canvas.

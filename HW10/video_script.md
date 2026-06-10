# Short demo video script for HW10

For the Canvas video, show this:

1. "This is my HW10 Pico + Pygame Zero project."
2. Show the wiring:
   - potentiometer signal to GP26
   - button to GP15 and GND
3. Show the serial monitor printing:
   - adc,button
   - for example 2020,0 and 3150,1
4. Run:
   ```bash
   pgzrun HW10_game.py
   ```
5. Turn the potentiometer and show the player moving left and right.
6. Press the button and show the player jumping.
7. Say: "The Pico sends sensor data to Python over USB serial, and the Python game uses the data to control the graphics."

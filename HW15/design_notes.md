# HW15 Design Notes

For HW15, I made a first-draft mechanical design for the haptic paddle. The design is based around an RC servo, a servo horn, a paddle arm, a small load cell, a thimble/finger contact point, and the AS5600 magnetic encoder.

## Mechanical layout

The servo is mounted to a rectangular base plate. The servo horn attaches to the paddle arm. The paddle arm has two small screw holes near the horn side and a load-cell mounting pattern near the end. The load cell is placed so the user's fingertip force is applied close to tangentially to the paddle motion. The thimble clamp is mounted at the outer end of the arm. The AS5600 is held near the servo spline, with the small magnet centered coaxially with the rotating shaft.

## Design choices

- The base plate uses oversized clearance holes so the servo can be adjusted slightly during assembly.
- The paddle arm is kept long and narrow to leave space for the load cell and reduce interference with the servo body.
- The load cell area uses two screw holes for clamping.
- The thimble clamp uses a simple single-bolt clamp as the first draft.
- The AS5600 bracket is separate so its air gap and alignment can be adjusted after printing.

## Expected first-print issues

The first version may need to be printed more than once. I expect small issues such as support material in holes, slightly tight screw clearance, servo horn screw length, backlash in the servo gear, and possible pinch points around the horn and paddle arm. After the first print, I would check the load-cell fit, AS5600 magnet alignment, and whether the fingertip force is applied tangentially.

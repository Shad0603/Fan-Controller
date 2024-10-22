# Fan_Controller
Project using a NUCLEO board to control the speed of a cpu fan using PID control.  
[![My Skills](https://skillicons.dev/icons?i=arduino,cpp)](https://skillicons.dev)

**CPU Fan Controller**

The program is used to control the speed of a 3-pin 12V CPU fan with the use of a Nucleo board and extension board.
The speed of the fan is changed by rotating the rotary encoder.

Minimum speed of the fan: 600 RPM
Maximum speed of the fan: 2000 RPM

The control mode in operation is demonstrated by the clour of the LED.
By default, the system is in open loop control mode, therefore the LED colour is GREEN.

The increment in fan speed per click of the encoder is 14 RPM.

LCD Display:
The LCD shows the desired RPM speed set by the user, and the current RPM read from the tachometer.

Example Interaction:
1. Increase fan speed by rotating rotary encoder clockwise
2. Observe tachometer reading on LCD display, shown by " True RPM"
3. Decrease fan speed by rotating rotary encoder anticlockwise
4. Observe tachometer reading on LCD display, shown by " True RPM"

Fan speed is set to 0 using reset button on Nucleo board.


# IR_Light_Gun

This is an arduino sketch for IR Light gun with the DFRobot camera.

The project started as a small update of SAMCO's great IR Cam code ( https://github.com/samuelballantyne ), 
but since I rewrote most of the code I decided to make my own Github.

Here is the list of notable differences compared to SAMCO's original code:
- A bit more precise mouse cursor; 
  I kept SAMCO's efficient position calculation code, but made it float instead of integer. 
  The difference is not huge (few pixels), but it removes a bit of cursor jittering
- Faster cursor and less latency; 
  since I rewrote everything without delay function in the main loop,
  the exection is faster and the mouse has less lag.
- Out of range detection; 
  Detects if there is less than 2 points visible (out of the screen aiming, obsctacle...),
  leave the cursor at the last known point (to avoid accidental cursor jump), and release the mouse control (to use a normal mouse).
- Calibration parameters;
  You can now edit where you want the calibration points to be on the screen.
- Calibration error check; 
  To avoid calibration problems, it will not let you save calibration points if it's out of range (for instance if you are too close from the screen).
- Joystick mode support;
  Now you can also use your Gun as a joystick for games that don't have mouse support.
  You can enable/disable it by holding the calibration button more than 2 seconds.
  There seems to be slightly more latency in this mode, so I would advice always using the mouse mode when possible.
- Save calibration points and joystick/mouse mode in the arduino EEPROM;
  Now your last calibration points, as well as the last mode used (joystick/mouse) are saved in the arduino EEPROM memory.
  You can disable the option easily if you don't want to use it.
- Added a button holding detection, with timer for each button;
  A hold function will be triggered on whatever button you hold, with the timing you want.
  Useful for things like fullauto recoil mode or simply having a secondary button function when holding.
- Sleep timer;
  This timer is used to pause any calculation when not aiming at the screen for a while.
  I wanted to also show down the camera, but it doesn't seem to want to be switch OFF and ON again. (limitation of the Wire library?)
- Position filter;
  You can activate this option to make motion become a lot smoother, and removes lot of jittering,
  but it also adds more latency. I recommend to leave it off unless you really have precision trouble.
- Rumble motors and solenoid support;
  If you connect a solenoid board or a rumble motor to pin 7 (or other pin if you change it), you will get rumble/recoil support.
  It will be triggered once when the left button (gun trigger) is pushed, and switch to fullauto mode if the button is hold.
  Be very careful if you use solenoid as you need the correct circuit for it, and using a bad timing settings might also damage it.
  As the other options, you can change the settings at the beginning of the sketch file.

Thing to add next:
- Add button combo detection (for instance calibration + trigger).
- A way to enable/disable cursor filter with a button combo.
- Make a Windows tool to manage those options, and help with the configuration.
- Investigating in new ways of improving even further the accuracy and viewing angle.
- Trying to support PS2/XBox or other old hardware. If somebody can help me with the PS2/XBox protocol, that would be great :)
- Add a RGB LED support for status/mode control and errors. 

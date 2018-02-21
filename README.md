![logo](doc/logo.png)
# Illuminate - Open-source LED array controller firmware
Illuminate was designed to standardize the interface and functionality of LED array illuminators for optical microscopy.

## Install
1. Install [Arduino](https://www.arduino.cc/) software
2. Install [Teensyduino](https://www.pjrc.com/teensy/td_download.html) software (If using a teensy micro-controller)
3. Check out repostory
```bash
  git clone https://github.com/zfphil/illuminate.git
```
4. open illuminate.ino in arduino, select micro-controller from Tools -> Board, select port from Tools -> Port
5. Open Serial Monitor (Tools -> Serial Monitor), set line ending to Newline and baud rate to 115200 (default)
6. Type command (ex: "bf") and press enter to send

## Commands
Command help can be accessed by typing "?" into the Arduino terminal.

## Interfaces
All commands are sent over a serial (COM) port. This allows interfacing from any program or program language on most systems, as well as through Micro-Manager or other microscopy platforms.

#### Interface Repositories
- (more to come)

## Devices
This project is designed for led arrays which are controlled by a Teensy 3.2 micro-controller. Additional micro-controllers should be easy to support if pins are configured correctly.

#### Currently supported
- Quasi-Dome Illuminator (Waller Lab, UC Berkeley)
- Direct LED connection array (Waller Lab, UC Berkeley)
- Target Array (SCI Microscopy)

#### Planned support
- 32x32 array (Adafruit)
- (Your LED array here!)

#### Adding New Devices
New devices are created by adding a new .cpp file to the root directory of the project and providing functions for the LedArrayInterface class. Static variables within this class must also be defined in the cpp file (including LED positions, trigger ports, etc.).

## License
Illuminate is licensed under the BSD 3-clause license

## Contributions
Pull requests will be reviewed as received!

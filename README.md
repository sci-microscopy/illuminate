<<<<<<< HEAD
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
4. Check out submodule (TLC5955)
```bash
  cd illuminate
  git submodule init
  git submodule update
```
5. open illuminate.ino in Arduino software
6. Ensure correct device is selected in the illuminate.h file by uncommenting the appropriate ``#define`` line
7. elect micro-controller from Tools -> Board, select port from Tools -> Port
8. Open Serial Monitor (Tools -> Serial Monitor), set line ending to Newline and baud rate to 115200 (default)
9. Press upload to load code onto Teensy
10. Type command (ex: "bf") and press enter to send

## Commands
Command help can be accessed by typing "?" into the Arduino terminal.

## Interfaces
All commands are sent over a serial (COM) port. This allows interfacing from any program or program language on most systems, as well as through Micro-Manager or other microscopy platforms. There is also a controller library located [here](http://www.github.com/zfphil/illuminate_controller)

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
BSD 3-clause

## Contributions
Pull requests will be reviewed as received!

## API Reference

```
=== Illuminate r1.16 | Serial Number: 65535 | Part Number: 65535 | Teensy MAC address: 04:E9:E5:04:17:29
=== For help, type ?
-----------------------------------
Command List:
-----------------------------------
COMMAND:
? / help
SYNTAX:?
DESCRIPTION:
Display help info
-----------------------------------
COMMAND:
info / about
SYNTAX:about
DESCRIPTION:
Displays information about this LED Array
-----------------------------------
COMMAND:
reboot / reset
SYNTAX:reboot
DESCRIPTION:
Runs setup routine again, for resetting LED array
-----------------------------------
COMMAND:
ver / version
SYNTAX:
DESCRIPTION:
Display controller version number
-----------------------------------
COMMAND:
ac / autoClear
SYNTAX:ac --or-- ac.[0/1]
DESCRIPTION:
Toggle clearing of array between led updates. Can call with or without options.
-----------------------------------
COMMAND:
na / setNa
SYNTAX:na.[na*100]
DESCRIPTION:
Set na used for bf/df/dpc/cdpc patterns
-----------------------------------
COMMAND:
sc / setColor
SYNTAX:sc,[rgbVal] --or-- sc.[rVal].[gVal].[bVal]
DESCRIPTION:
Set LED array color
-----------------------------------
COMMAND:
sb / setBrightness
SYNTAX:sb,[rgbVal] --or-- sb.[rVal].[gVal].[bVal]
DESCRIPTION:
Set LED array brightness
-----------------------------------
COMMAND:
sad / setArrayDistance
SYNTAX:sad,[dist (mm)]
DESCRIPTION:
Set LED array distance
-----------------------------------
COMMAND:
l / led
SYNTAX:ll.[led #].[led #], ...
DESCRIPTION:
Turn on a single LED (or multiple LEDs in a list)
-----------------------------------
COMMAND:
x / xx
SYNTAX:x
DESCRIPTION:
Clear the LED array.
-----------------------------------
COMMAND:
ff / fillArray
SYNTAX:ff
DESCRIPTION:
Fill the LED array with default color.
-----------------------------------
COMMAND:
bf / brightfield
SYNTAX:bf
DESCRIPTION:
Display brightfield pattern
-----------------------------------
COMMAND:
df / darkfield
SYNTAX:df
DESCRIPTION:
Display darkfield pattern
-----------------------------------
COMMAND:
dpc / halfCircle
SYNTAX:dpc.[t/b/l/r] --or-- dpc.[top/bottom/left/right] --or-- dpc (will raw first pattern)
DESCRIPTION:
Illuminate half-circle (DPC) pattern
-----------------------------------
COMMAND:
cdpc / colorDpc
SYNTAX:cdpc.[rVal],[gVal].[bVal]) --or-- cdpc.[rgbVal]) --or-- cdpc
DESCRIPTION:
Illuminate color DPC (cDPC) pattern
-----------------------------------
COMMAND:
an / annulus
SYNTAX:an.[minNA*100].[maxNA*100]
DESCRIPTION:
Display annulus pattern set by min/max na
-----------------------------------
COMMAND:
ha / halfAnnulus
SYNTAX:ha.[type].[minNA*100].[maxNA*100]
DESCRIPTION:
Illuminate half annulus
-----------------------------------
COMMAND:
dq / drawQuadrant
SYNTAX:dq --or-- dq.[rVal].[gVal].[bVal]
DESCRIPTION:
Draws single quadrant
-----------------------------------
COMMAND:
cdf / Color Darkfield
SYNTAX:cdf.[rVal].[gVal].[bVal]) --or-- cdf.[rgbVal]) --or-- cdf
DESCRIPTION:
Draws color darkfield pattern
-----------------------------------
COMMAND:
ndpc / navigator
SYNTAX:ndpc.[t/b/l/r] --or-- ndpc.[top/bottom/left/right]
DESCRIPTION:
Illuminate half-circle (DPC) pattern with navigator
-----------------------------------
COMMAND:
scf / scanFull
SYNTAX:scf,[delay_ms]
DESCRIPTION:
Scan all active LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.
-----------------------------------
COMMAND:
scb / scanBrightfield
SYNTAX:scb,[delay_ms]
DESCRIPTION:
Scan all brightfield LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.
-----------------------------------
COMMAND:
ssl / setSeqLength
SYNTAX:ssl,[Sequence length]
DESCRIPTION:
Set sequence length in terms of independent patterns
-----------------------------------
COMMAND:
ssv / setSeqValue
SYNTAX:ssl.[1st LED #]. [1st rVal]. [1st gVal]. [1st bVal]. [2nd LED #]. [2nd rVal]. [2nd gVal]. [2nd bVal] ...
DESCRIPTION:
Set sequence value
-----------------------------------
COMMAND:
rseq / runSequence
SYNTAX:rseq,[Delay between each pattern in ms].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2]
DESCRIPTION:
Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.
-----------------------------------
COMMAND:
rseqf / runSequenceFast
SYNTAX:rseqf,[Delay between each pattern in ms].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2]
DESCRIPTION:
Runs sequence with specified delay between each update. Uses parallel digital IO to acheive very fast speeds. Only available on certain LED arrays.
-----------------------------------
COMMAND:
pseq / printSeq
SYNTAX:pseq
DESCRIPTION:
Prints sequence values to the terminal
-----------------------------------
COMMAND:
pseql / printSeqLength
SYNTAX:pseql
DESCRIPTION:
Prints sequence length to the terminal
-----------------------------------
COMMAND:
sseq / stepSequence
SYNTAX:sseq.[trigger output mode for index 0].[trigger output mode for index 1],
DESCRIPTION:
Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.
-----------------------------------
COMMAND:
reseq / resetSeq
SYNTAX:reseq
DESCRIPTION:
Resets sequence index to start
-----------------------------------
COMMAND:
ssbd / setSeqBitDepth
SYNTAX:ssbd.1 --or-- ssbd.8 --or-- ssbd.16
DESCRIPTION:
Sets bit depth of sequence values (1, 8, or 16)
-----------------------------------
COMMAND:
ssz / setSeqZeros
SYNTAX:ssz.10
DESCRIPTION:
Sets a range of the sequence entries to zero, starting at the current sequence index
-----------------------------------
COMMAND:
tr / trig
SYNTAX:tr.[trigger index]
DESCRIPTION:
Output TTL trigger pulse to camera
-----------------------------------
COMMAND:
trs / trigSetup
SYNTAX:trs.[trigger index].[trigger pin index].['trigger delay between H and L pulses]
DESCRIPTION:
Set up hardware (TTL) triggering
-----------------------------------
COMMAND:
ptr / trigPrint
SYNTAX:ptr
DESCRIPTION:
Prints information about the current i/o trigger setting
-----------------------------------
COMMAND:
trt / trigTest
SYNTAX:trt.[trigger input index]
DESCRIPTION:
Waits for trigger pulses on the defined channel
-----------------------------------
COMMAND:
ch / drawChannel
SYNTAX:dc.[led#]
DESCRIPTION:
Draw LED by hardware channel (use for debugging)
-----------------------------------
COMMAND:
dbg / debug
SYNTAX:dbg.[command router debug].[LED array (generic) debug].[LED interface debug] --or-- dbg (toggles all between level 1 or 0)
DESCRIPTION:
Toggle debug flag. Can call with or without options.
-----------------------------------
COMMAND:
spo / setPinOrder
SYNTAX:spo.[rChan].[gChan].[bChan] --or-- spo.[led#].[rChan].[gChan].[bChan]
DESCRIPTION:
Sets pin order (R/G/B) for setup purposes. Also can flip individual leds by passing fourth argument.
-----------------------------------
COMMAND:
delay / wait
SYNTAX:delay.[length of time in ms]
DESCRIPTION:
Simply puts the device in a loop for the amount of time in ms
-----------------------------------
COMMAND:
smc / setMaxCurrent
SYNTAX:smc.[current limit in amps]
DESCRIPTION:
Sets max current in amps
-----------------------------------
COMMAND:
smce / setMaxCurrentEnforcement
SYNTAX:smce.[0, 1]
DESCRIPTION:
Sets whether or not max current limit is enforced (0 is no, all other values are yes)
-----------------------------------
COMMAND:
pvals / printVals
SYNTAX:pvals
DESCRIPTION:
Print led values for software interface
-----------------------------------
COMMAND:
pp / printParams
SYNTAX:pp
DESCRIPTION:
Prints system parameters such as NA, LED Array z-distance, etc. in the format of a json file
-----------------------------------
COMMAND:
pledpos / printLedPositions
SYNTAX:pledpos
DESCRIPTION:
Prints the positions of each LED in cartesian coordinates.
-----------------------------------
COMMAND:
pledposna / printLedPositionsNa
SYNTAX:pledposna
DESCRIPTION:
Prints the positions of each LED in NA coordinates (NA_x, NA_y, NA_distance
-----------------------------------
COMMAND:
disco / party
SYNTAX:disco,[Number of LEDs in pattern]
DESCRIPTION:
Illuminate a random color pattern of LEDs
-----------------------------------
COMMAND:
demo / runDemo
SYNTAX:demo
DESCRIPTION:
Runs a demo routine to show what the array can do.
-----------------------------------
COMMAND:
water / waterDrop
SYNTAX:water
DESCRIPTION:
Water drop demo
-----------------------------------
COMMAND:
setsn / setSerialNumber
SYNTAX:
DESCRIPTION:
Sets device serial number in EEPROM (DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING
-----------------------------------
COMMAND:
setpn / setPartNumber
SYNTAX:
DESCRIPTION:
Sets device part number in EEPROM (DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING
-----------------------------------
COMMAND:
rdpc / runDpc
SYNTAX:rdpc,[Delay between each pattern in ms].[Number of acquisitions].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2]
DESCRIPTION:
Runs a DPC sequence with specified delay between each update. If update speed is too fast, a warming message will print.
-----------------------------------
COMMAND:
rfpm / runFpm
SYNTAX:rfpm,[Delay between each pattern in ms].[Number of acquisitions].[Maximum NA * 100 (e.g. 0.25NA would be 25].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2]
DESCRIPTION:
Runs a FPM sequence with specified delay between each update. If update speed is too fast, a warming message will print.
-----------------------------------
COMMAND:
sbr / setBaudRate
SYNTAX:sbr.1000000
DESCRIPTION:
Sets SPI baud rate for TLC5955 Chips in Hz (baud)
-----------------------------------
COMMAND:
sgs / setGsclkFreq
SYNTAX:sgs.1000000
DESCRIPTION:
Sets GSCLK frequency in Hz
-----------------------------------
COMMAND:
human / setModeHuman
SYNTAX:human
DESCRIPTION:
Sets command mode to human-readable
-----------------------------------
COMMAND:
machine / setModeMachine
SYNTAX:machine
DESCRIPTION:
Sets command mode to machine-readable
-----------------------------------
```
=======
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
4. Check out submodule (TLC5955)
```bash
  cd illuminate
  git submodule init
  git submodule update
```
5. open illuminate.ino in Arduino software
6. Ensure correct device is selected in the illuminate.h file by uncommenting the appropriate ``#define`` line
7. elect micro-controller from Tools -> Board, select port from Tools -> Port
8. Open Serial Monitor (Tools -> Serial Monitor), set line ending to Newline and baud rate to 115200 (default)
9. Press upload to load code onto Teensy
10. Type command (ex: "bf") and press enter to send

## Commands
Command help can be accessed by typing "?" into the Arduino terminal.

## Interfaces
All commands are sent over a serial (COM) port. This allows interfacing from any program or program language on most systems, as well as through Micro-Manager or other microscopy platforms. There is also a controller library located [here](http://www.github.com/zfphil/illuminate_controller)

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
BSD 3-clause

## Contributions
Pull requests will be reviewed as received!

## API Reference

```
-----------------------------------
Command List: 
-----------------------------------
COMMAND: 
? / help
SYNTAX:?
DESCRIPTION:
Display help info
-----------------------------------
COMMAND: 
info / about
SYNTAX:about
DESCRIPTION:
Displays information about this LED Array
-----------------------------------
COMMAND: 
reboot / reset
SYNTAX:reboot
DESCRIPTION:
Runs setup routine again, for resetting LED array
-----------------------------------
COMMAND: 
ver / version
SYNTAX:
DESCRIPTION:
Display controller version number
-----------------------------------
COMMAND: 
ac / autoClear
SYNTAX:ac --or-- ac.[0/1]
DESCRIPTION:
Toggle clearing of array between led updates. Can call with or without options.
-----------------------------------
COMMAND: 
na / setNa
SYNTAX:na.[na*100]
DESCRIPTION:
Set na used for bf/df/dpc/cdpc patterns
-----------------------------------
COMMAND: 
sc / setColor
SYNTAX:sc,[rgbVal] --or-- sc.[rVal].[gVal].[bVal]
DESCRIPTION:
Set LED array color
-----------------------------------
COMMAND: 
sb / setBrightness
SYNTAX:sb,[rgbVal] --or-- sb.[rVal].[gVal].[bVal]
DESCRIPTION:
Set LED array brightness
-----------------------------------
COMMAND: 
sad / setArrayDistance
SYNTAX:sad,[dist (mm)]
DESCRIPTION:
Set LED array distance
-----------------------------------
COMMAND: 
l / led
SYNTAX:ll.[led #].[led #], ...
DESCRIPTION:
Turn on a single LED (or multiple LEDs in a list)
-----------------------------------
COMMAND: 
x / xx
SYNTAX:x
DESCRIPTION:
Clear the LED array.
-----------------------------------
COMMAND: 
ff / fillArray
SYNTAX:ff
DESCRIPTION:
Fill the LED array with default color.
-----------------------------------
COMMAND: 
bf / brightfield
SYNTAX:bf
DESCRIPTION:
Display brightfield pattern
-----------------------------------
COMMAND: 
df / darkfield
SYNTAX:df
DESCRIPTION:
Display darkfield pattern
-----------------------------------
COMMAND: 
dpc / halfCircle
SYNTAX:dpc.[t/b/l/r] --or-- dpc.[top/bottom/left/right] --or-- dpc (will raw first pattern)
DESCRIPTION:
Illuminate half-circle (DPC) pattern
-----------------------------------
COMMAND: 
cdpc / colorDpc
SYNTAX:cdpc.[rVal],[gVal].[bVal]) --or-- cdpc.[rgbVal]) --or-- cdpc
DESCRIPTION:
Illuminate color DPC (cDPC) pattern
-----------------------------------
COMMAND: 
an / annulus
SYNTAX:an.[minNA*100].[maxNA*100]
DESCRIPTION:
Display annulus pattern set by min/max na
-----------------------------------
COMMAND: 
ha / halfAnnulus
SYNTAX:ha.[type].[minNA*100].[maxNA*100]
DESCRIPTION:
Illuminate half annulus
-----------------------------------
COMMAND: 
dq / drawQuadrant
SYNTAX:dq --or-- dq.[rVal].[gVal].[bVal]
DESCRIPTION:
Draws single quadrant
-----------------------------------
COMMAND: 
cdf / Color Darkfield
SYNTAX:cdf.[rVal].[gVal].[bVal]) --or-- cdf.[rgbVal]) --or-- cdf
DESCRIPTION:
Draws color darkfield pattern
-----------------------------------
COMMAND: 
ndpc / navigator
SYNTAX:ndpc.[t/b/l/r] --or-- ndpc.[top/bottom/left/right]
DESCRIPTION:
Illuminate half-circle (DPC) pattern with navigator
-----------------------------------
COMMAND: 
scf / scanFull
SYNTAX:scf,[delay_ms]
DESCRIPTION:
Scan all active LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.
-----------------------------------
COMMAND: 
scb / scanBrightfield
SYNTAX:scb,[delay_ms]
DESCRIPTION:
Scan all brightfield LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.
-----------------------------------
COMMAND: 
ssl / setSeqLength
SYNTAX:ssl,[Sequence length]
DESCRIPTION:
Set sequence length in terms of independent patterns
-----------------------------------
COMMAND: 
ssv / setSeqValue
SYNTAX:ssl.[# Number of LEDs], [LED number 0], [LED number 1]], [LED number 2], ...
DESCRIPTION:
Set sequence value
-----------------------------------
COMMAND: 
rseq / runSequence
SYNTAX:rseq,[Delay between each pattern in ms].[number of times to repeat pattern].[trigger output 0 mode].[trigger output 1 mode].[trigger input 0 mode].[trigger input 1 mode]
DESCRIPTION:
Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.
-----------------------------------
COMMAND: 
rseqf / runSequenceFast
SYNTAX:rseqf,[Delay between each pattern in ms].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2] 
DESCRIPTION:
Runs sequence with specified delay between each update. Uses parallel digital IO to acheive very fast speeds (single us). Only available on certain LED arrays.
-----------------------------------
COMMAND: 
pseq / printSeq
SYNTAX:pseq
DESCRIPTION:
Prints sequence values to the terminal
-----------------------------------
COMMAND: 
pseql / printSeqLength
SYNTAX:pseql
DESCRIPTION:
Prints sequence length to the terminal
-----------------------------------
COMMAND: 
sseq / stepSequence
SYNTAX:sseq.[trigger output mode for index 0].[trigger output mode for index 1],
DESCRIPTION:
Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.
-----------------------------------
COMMAND: 
reseq / resetSeq
SYNTAX:reseq
DESCRIPTION:
Resets sequence index to start
-----------------------------------
COMMAND: 
ssbd / setSeqBitDepth
SYNTAX:ssbd.1 --or-- ssbd.8 --or-- ssbd.16.
DESCRIPTION:
Sets bit depth of sequence values (1, 8, or 16)
-----------------------------------
COMMAND: 
ssz / setSeqZeros
SYNTAX:ssz.10
DESCRIPTION:
Sets a range of the sequence entries to zero, starting at the current sequence index
-----------------------------------
COMMAND: 
tr / trig
SYNTAX:tr.[trigger index]
DESCRIPTION:
Output TTL trigger pulse to camera
-----------------------------------
COMMAND: 
trs / trigSetup
SYNTAX:trs.[trigger index].[trigger pin index].['trigger delay between H and L pulses]
DESCRIPTION:
Set up hardware (TTL) triggering
-----------------------------------
COMMAND: 
trt / trigTest
SYNTAX:trt.[trigger input index]
DESCRIPTION:
Waits for trigger pulses on the defined channel
-----------------------------------
COMMAND: 
ch / drawChannel
SYNTAX:dc.[led#]
DESCRIPTION:
Draw LED by hardware channel (use for debugging)
-----------------------------------
COMMAND: 
dbg / debug
SYNTAX:dbg.[command router debug].[LED array (generic) debug].[LED interface debug] --or-- dbg (toggles all between level 1 or 0)
DESCRIPTION:
Toggle debug flag. Can call with or without options.
-----------------------------------
COMMAND: 
spo / setPinOrder
SYNTAX:spo.[rChan].[gChan].[bChan] --or-- spo.[led#].[rChan].[gChan].[bChan]
DESCRIPTION:
Sets pin order (R/G/B) for setup purposes. Also can flip individual leds by passing fourth argument.
-----------------------------------
COMMAND: 
delay / wait
SYNTAX:delay.[length of time in ms]
DESCRIPTION:
Simply puts the device in a loop for the amount of time in ms
-----------------------------------
COMMAND: 
smc / setMaxCurrent
SYNTAX:smc.[current limit in amps]
DESCRIPTION:
Sets max current in amps
-----------------------------------
COMMAND: 
smce / setMaxCurrentEnforcement
SYNTAX:smce.[0, 1]
DESCRIPTION:
Sets whether or not max current limit is enforced (0 is no, all other values are yes)
-----------------------------------
COMMAND: 
pvals / printVals
SYNTAX:pvals
DESCRIPTION:
Print led values for software interface
-----------------------------------
COMMAND: 
pp / printParams
SYNTAX:pp
DESCRIPTION:
Prints system parameters such as NA, LED Array z-distance, etc. in the format of a json file
-----------------------------------
COMMAND: 
pledpos / printLedPositions
SYNTAX:pledpos
DESCRIPTION:
Prints the positions of each LED in cartesian coordinates.
-----------------------------------
COMMAND: 
pledposna / printLedPositionsNa
SYNTAX:pledposna
DESCRIPTION:
Prints the positions of each LED in NA coordinates (NA_x, NA_y, NA_distance
-----------------------------------
COMMAND: 
disco / party
SYNTAX:disco.[Number of LEDs in pattern]
DESCRIPTION:
Illuminate a random color pattern of LEDs
-----------------------------------
COMMAND: 
demo / runDemo
SYNTAX:demo
DESCRIPTION:
Runs a demo routine to show what the array can do.
-----------------------------------
COMMAND: 
water / waterDrop
SYNTAX:water
DESCRIPTION:
Water drop demo
-----------------------------------
COMMAND: 
setsn / setSerialNumber
SYNTAX:
DESCRIPTION:
Sets device serial number in EEPROM (DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING
-----------------------------------
COMMAND: 
setpn / setPartNumber
SYNTAX:
DESCRIPTION:
Sets device part number in EEPROM (DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING
-----------------------------------
COMMAND: 
rdpc / runDpc
SYNTAX:rdpc,[Delay between each pattern in ms (can be zero)].[Number of acquisitions].[trigger output mode for trigger output 0].[trigger input mode for trigger input 0].[trigger output mode for trigger output 1].[trigger input mode for trigger input 1]
DESCRIPTION:
Runs a DPC sequence with specified delay between each update. If update speed is too fast, a warning message will print.
-----------------------------------
COMMAND: 
rfpm / runFpm
SYNTAX:rfpm,[Delay between each pattern in ms (can be zero)].[Number of acquisitions].[Maximum NA * 100 (e.g. 0.25NA would be 25].[trigger output mode for trigger output 0].[trigger input mode for trigger input 0].[trigger output mode for trigger output 1].[trigger input mode for trigger input 1]
DESCRIPTION:
Runs a FPM sequence with specified delay between each update. If update speed is too fast, a warning message will print.
-----------------------------------
COMMAND: 
sbr / setBaudRate
SYNTAX:sbr.1000000
DESCRIPTION:
Sets SPI baud rate for TLC5955 Chips in Hz (baud)
-----------------------------------
COMMAND: 
sgs / setGsclkFreq
SYNTAX:sgs.1000000
DESCRIPTION:
Sets GSCLK frequency in Hz
-----------------------------------
COMMAND: 
human / setModeHuman
SYNTAX:human
DESCRIPTION:
Sets command mode to human-readable
-----------------------------------
COMMAND: 
machine / setModeMachine
SYNTAX:machine
DESCRIPTION:
Sets command mode to machine-readable
-----------------------------------
COMMAND: 
pwrc / isPowerSourceConnected
SYNTAX:pwrc
DESCRIPTION:
Gets the state of the power source, if this device has the hardware to do so.
-----------------------------------
COMMAND: 
pwrs / togglePowerSourceSensing
SYNTAX:pwrs
DESCRIPTION:
Toggle power source sensing on or off.
-----------------------------------
COMMAND: 
pwrv / printPowerSourceVoltage
SYNTAX:pwrv
DESCRIPTION:
Print power sourve voltage.
-----------------------------------
COMMAND: 
nai / setInnerNa
SYNTAX:nai.20
DESCRIPTION:
Sets the inner NA. (nai.20 sets an inner NA of 0.20)  Respected by bf, dpc, and rdpc commands. Default is 0
-----------------------------------
COMMAND: 
trinputtimeout / triggerInputTimeout
SYNTAX:trinputtimeout.10
DESCRIPTION:
Sets the trigger input timeout in seconds. Default is 3600
-----------------------------------
COMMAND: 
troutputpulsewidth / triggerOutputPulseWidth
SYNTAX:troutputpulsewidth.1000
DESCRIPTION:
Sets the trigger pulse width in microseconds, default is 1000.
-----------------------------------
COMMAND: 
trinputpolarity / triggerInputPolarity
SYNTAX:trinputpolarity.1
DESCRIPTION:
Sets the trigger input polarity. 1=active high, 0=active low. Default is 1.
-----------------------------------
COMMAND: 
troutputpolarity / triggerOutputPolarity
SYNTAX:troutputpolarity.1
DESCRIPTION:
Sets the trigger output polarity. 1=active high, 0=active low. Default is 1.
-----------------------------------
COMMAND: 
troutputdelay / triggerOutputDelay
SYNTAX:troutputdelay.0
DESCRIPTION:
Sets the trigger delay in microseconds. Default is zero.
-----------------------------------
COMMAND: 
trinputpin / triggerInputPin
SYNTAX:trinputpin
DESCRIPTION:
Returns the Teensy pin of the trigger inputsignal. Used only for debugging.
-----------------------------------
COMMAND: 
troutputpin / triggerOutputPin
SYNTAX:troutputpin
DESCRIPTION:
Returns the Teensy pin of the trigger outputsignal. Used only for debugging.
-----------------------------------
COMMAND: 
cos / cosineFactor
SYNTAX:cos.2
DESCRIPTION:
Returns or sets the cosine factor, used to scale LED intensity (so outer LEDs are brighter). Input is cos.[integer cosine factor]
-----------------------------------
```
>>>>>>> 40744e1e02bed4aeb18c2c7ac67519cb5af3f1f7

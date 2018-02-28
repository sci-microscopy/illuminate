/*
  Copyright (c) 2018, Zachary Phillips (UC Berkeley)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
      Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMAND_CONSTANTS_H
#define COMMAND_CONSTANTS_H

// List of command indicies in below array
#define COMMAND_COUNT 46

#define CMD_HELP_IDX 0
#define CMD_ABOUT_IDX 1
#define CMD_REBOOT_IDX 2
#define CMD_SHOW_VERSION 3

#define CMD_AUTOCLEAR_IDX 4
#define CMD_NA_IDX 5
#define CMD_SET_COLOR_IDX 6
#define CMD_SET_ARRAY_DIST 7

#define CMD_LED_IDX 8

#define CMD_CLEAR_IDX 9
#define CMD_FILL_IDX 10
#define CMD_BF_IDX 11
#define CMD_DF_IDX 12
#define CMD_DPC_IDX 13
#define CMD_CDPC_IDX 14
#define CMD_AN_IDX 15
#define CMD_HALF_ANNULUS 16
#define CMD_DQ_IDX 17
#define CMD_CDF_IDX 18
#define CMD_NAV_DPC_IDX 19

#define CMD_SCF_IDX 20
#define CMD_SCB_IDX 21

#define CMD_LEN_SEQ_IDX 22
#define CMD_SET_SEQ_IDX 23
#define CMD_RUN_SEQ_IDX 24
#define CMD_RUN_SEQ_FAST_IDX 25
#define CMD_PRINT_SEQ_IDX 26
#define CMD_PRINT_SEQ_LENGTH_IDX 27
#define CMD_STEP_SEQ_IDX 28
#define CMD_RESET_SEQ_IDX 29
#define CMD_SET_SEQ_BIT_DEPTH 30

#define CMD_TRIG_IDX 31
#define CMD_TRIG_SETUP_IDX 32
#define CMD_TRIG_PRINT_IDX 33
#define CMD_TRIG_TEST_IDX 34
#define CMD_CHANNEL_IDX 35
#define CMD_TOGGLE_DEBUG_IDX 36
#define CMD_PIN_ORDER_IDX 37
#define CMD_DELAY 38

#define CMD_PRINT_VALS_IDX 39
#define CMD_PRINT_PARAMS 40
#define CMD_PRINT_LED_POSITIONS 41
#define CMD_PRINT_LED_POSITIONS_NA 42

#define CMD_DISCO_IDX 43
#define CMD_DEMO_IDX 44
#define CMD_WATER_IDX 45

// Syntax is: {short command, long command, description, syntax}
const char* command_list[COMMAND_COUNT][4] = {

  // High-level Commands
  {"?", "help", "Display help info", "?"},
  {"ab", "about", "Displays information about this LED Array", "about"},
  {"reboot", "reset", "Runs setup routine again, for resetting LED array", "reboot"},
  {"ver", "version", "Display controller version number"},

  // System Parameters
  {"ac", "autoClear", "Toggle clearing of array between led updates. Can call with or without options.", "ac --or-- ac.[0,1]"},
  {"na", "setNa", "Set na used for bf/df/dpc/cdpc patterns", "na.[na*100]"},
  {"sc", "setColor", "Set LED array color", "sc,[rgbVal] --or-- sc.[rVal].[gVal].[bVal]"},
  {"sad", "setArrayDistance", "Set LED array distance", "sad,[100*dist(mm) --or-- 1000*dist(cm)]"},

  // Single (or multiple) LED Display
  {"l", "led", "Turn on a single LED (or multiple LEDs in a list)", "ll.[led #].[led #], ..."},

  // General Display
  {"x", "xx", "Clear the LED array.", "x"},
  {"ff", "fillArray", "Fill the LED array with default color.", "ff"},
  {"bf", "brightfield", "Display brightfield pattern", "bf"},
  {"df", "darkfield", "Display darkfield pattern", "df"},
  {"dpc", "halfCircle", "Illuminate half-circle (DPC) pattern", "dpc.[t/b/l/r] --or-- dpc.[top/bottom/left/right]"},
  {"cdpc", "colorDpc", "Illuminate color DPC (cDPC) pattern", "cdpc.[rVal],[gVal].[bVal]) --or-- cdpc.[rgbVal]) --or-- cdpc"},
  {"an", "annulus", "Display annulus pattern set by min/max na", "an.[minNA*100].[maxNA*100]"},
  {"ha", "halfAnnulus", "Illuminate half annulus", "ha.[type].[minNA*100].[maxNA*100]"},
  {"dq", "drawQuadrant", "Draws single quadrant", "dq --or-- dq.[rVal].[gVal].[bVal]"},
  {"cdf", "Color Darkfield", "Draws color darkfield pattern", "cdf.[rVal].[gVal].[bVal]) --or-- cdf.[rgbVal]) --or-- cdf"},
  {"ndpc", "navigator", "Illuminate half-circle (DPC) pattern with navigator", "ndpc.[t/b/l/r] --or-- ndpc.[top/bottom/left/right]"},

  // Single LED Scanning
  {"scf", "scanFull", "Scan all active LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.", "scf,[delay_ms]"},
  {"scb", "scanBrightfield", "Scan all brightfield LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.", "scb,[delay_ms]"},

  // Custom Sequence Scanning
  {"ssl",   "setSeqLength", "Set sequence length in terms of independent patterns", "ssl,[Sequence length]"},
  {"ssv",   "setSeqValue", "Set sequence value", "ssl.[1st LED #]. [1st rVal]. [1st gVal]. [1st bVal]. [2nd LED #]. [2nd rVal]. [2nd gVal]. [2nd bVal] ..."},
  {"rseq",  "runSequence", "Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.", "rseq,[Delay between each pattern in ms].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2] "},
  {"rseqf",  "runSequenceFast", "Runs sequence with specified delay between each update. Uses parallel digital IO to acheive very fast speeds. Only available on certain LED arrays.", "rseqf,[Delay between each pattern in ms].[trigger mode for index 0].[trigger mode for index 1].[trigger mode for index 2] "},
  {"pseq",  "printSeq", "Prints sequence values to the terminal", "pseq"}, \
  {"pseql", "printSeqLength", "Prints sequence length to the terminal", "pseql"},
  {"sseq",  "stepSequence", "Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.", "sseq.[trigger output mode for index 0].[trigger output mode for index 1],"},
  {"reseq", "resetSeq", "Resets sequence index to start", "reseq"},
  {"ssbd", "setSeqBitDepth", "Sets bit depth of sequence values (1, 8, or 16)", "ssbd.1 --or-- ssbd.8 --or-- ssbd.16"},

  // Debugging, Low-level Access, etc.
  {"tr", "trig", "Output TTL trigger pulse to camera", "tr.[trigger index]"},
  {"trs", "trigSetup", "Set up hardware (TTL) triggering", "trs.[trigger index].[trigger pin index].['trigger delay between H and L pulses]"},
  {"ptr", "trigPrint", "Prints information about the current i/o trigger setting", "ptr"},
  {"trt", "trigTest", "Waits for trigger pulses on the defined channel", "trt.[trigger input index]"},
  {"ch", "drawChannel", "Draw LED by hardware channel (use for debugging)", "dc.[led#]"},
  {"dbg", "debug", "Toggle debug flag. Can call with or without options.", "dbg --or-- dbg.[0 --or-- 1]"},
  {"spo", "setPinOrder", "Sets pin order (R/G/B) for setup purposes. Also can flip individual leds by passing fourth argument.", "spo.[rChan].[gChan].[bChan] --or-- spo.[led#].[rChan].[gChan].[bChan]"},
  {"delay", "wait", "Simply puts the device in a loop for the amount of time in ms", "delay.[length of time in ms]"},

  // Quering System State
  {"pvals", "printVals", "Print led values for software interface", "pvals"},
  {"pp", "printParams", "Prints system parameters such as NA, LED Array z-distance, etc. in the format of a json file", "pp"},
  {"pledpos", "printLedPositions", "Prints the positions of each LED in cartesian coordinates.", "pledpos"},
  {"pledposna", "printLedPositionsNa", "Prints the positions of each LED in NA coordinates (NA_x, NA_y, NA_distance", "pledposna"},

  // Stored Patterns
  {"disco", "party", "Illuminate a random color pattern of LEDs", "disco,[Number of LEDs in pattern]"},
  {"demo", "runDemo", "Runs a demo routine to show what the array can do.", "demo"},
  {"water", "waterDrop", "Water drop demo", "water"}
};

#endif

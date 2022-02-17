/*
  Copyright (c) 2021, Zack Phillips
  Copyright (c) 2018, Zachary Phillips (UC Berkeley)
  All rights reserved.

  BSD 3-Clause License

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
      Neither the name of the UC Berkley nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA , OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMANDCONSTANTS_H
#define COMMANDCONSTANTS_H

#include "illuminate.h"

int info_func(CommandRouter *cmd, int argc, const char **argv);
int reset_func(CommandRouter *cmd, int argc, const char **argv);
int version_func(CommandRouter *cmd, int argc, const char **argv);
int license_func(CommandRouter *cmd, int argc, const char **argv);
int demo_func(CommandRouter *cmd, int argc, const char **argv);
int help_func(CommandRouter *cmd, int argc, const char **argv);
int store_func(CommandRouter *cmd, int argc, const char **argv);
int recall_func(CommandRouter *cmd, int argc, const char **argv);
int autoload_func(CommandRouter *cmd, int argc, const char **argv);

int autoclear_func(CommandRouter *cmd, int argc, const char **argv);
int na_func(CommandRouter *cmd, int argc, const char **argv);
int na_inner_func(CommandRouter *cmd, int argc, const char **argv);
int color_func(CommandRouter *cmd, int argc, const char **argv);
int brightness_func(CommandRouter *cmd, int argc, const char **argv);
int array_distance_func(CommandRouter *cmd, int argc, const char **argv);

int set_led_func(CommandRouter *cmd, int argc, const char **argv);

int clear_func(CommandRouter *cmd, int argc, const char **argv);
int fill_func(CommandRouter *cmd, int argc, const char **argv);
int brightfield_func(CommandRouter *cmd, int argc, const char **argv);
int darkfield_func(CommandRouter *cmd, int argc, const char **argv);
int dpc_func(CommandRouter *cmd, int argc, const char **argv);
int cdpc_func(CommandRouter *cmd, int argc, const char **argv);
int annulus_func(CommandRouter *cmd, int argc, const char **argv);
int half_annulus_func(CommandRouter *cmd, int argc, const char **argv);
int quadrant_func(CommandRouter *cmd, int argc, const char **argv);
int color_darkfield_func(CommandRouter *cmd, int argc, const char **argv);

int scan_full_func(CommandRouter *cmd, int argc, const char **argv);
int scan_brightfield_func(CommandRouter *cmd, int argc, const char **argv);
int scan_darkfield_func(CommandRouter *cmd, int argc, const char **argv);

int set_sequence_length_func(CommandRouter *cmd, int argc, const char **argv);
int set_sequence_value_func(CommandRouter *cmd, int argc, const char **argv);
int run_sequence_func(CommandRouter *cmd, int argc, const char **argv);
int print_sequence_func(CommandRouter *cmd, int argc, const char **argv);
int step_sequence_func(CommandRouter *cmd, int argc, const char **argv);
int restart_sequence_func(CommandRouter *cmd, int argc, const char **argv);

int trigger_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_setup_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_test_func(CommandRouter *cmd, int argc, const char **argv);
int draw_channel_func(CommandRouter *cmd, int argc, const char **argv);
int debug_func(CommandRouter *cmd, int argc, const char **argv);
int set_pin_order_func(CommandRouter *cmd, int argc, const char **argv);
int wait_func(CommandRouter *cmd, int argc, const char **argv);
int set_max_current_func(CommandRouter *cmd, int argc, const char **argv);
int set_max_current_enforcement_func(CommandRouter *cmd, int argc, const char **argv);

int print_led_values_func(CommandRouter *cmd, int argc, const char **argv);
int print_led_positions_func(CommandRouter *cmd, int argc, const char **argv);
int print_parameters_func(CommandRouter *cmd, int argc, const char **argv);
int print_led_positions_na(CommandRouter *cmd, int argc, const char **argv);

int disco_func(CommandRouter *cmd, int argc, const char **argv);
int demo_mode_func(CommandRouter *cmd, int argc, const char **argv);
int water_func(CommandRouter *cmd, int argc, const char **argv);

int get_sn_func(CommandRouter *cmd, int argc, const char **argv);
int get_pn_func(CommandRouter *cmd, int argc, const char **argv);

int run_fpm_func(CommandRouter *cmd, int argc, const char **argv);
int run_dpc_func(CommandRouter *cmd, int argc, const char **argv);

int set_baud_rate_func(CommandRouter *cmd, int argc, const char **argv);
int set_gsclk_func(CommandRouter *cmd, int argc, const char **argv);

int set_human_mode_func(CommandRouter *cmd, int argc, const char **argv);
int set_machine_mode_func(CommandRouter *cmd, int argc, const char **argv);

int power_sensing_func(CommandRouter *cmd, int argc, const char **argv);
int power_source_voltage_func(CommandRouter *cmd, int argc, const char **argv);

int trigger_input_timeout_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_output_pulse_width_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_input_polarity_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_output_polarity_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_output_delay_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_input_pin_func(CommandRouter *cmd, int argc, const char **argv);
int trigger_output_pin_func(CommandRouter *cmd, int argc, const char **argv);

int cosine_func(CommandRouter *cmd, int argc, const char **argv);
int hw_initialize_function(CommandRouter *cmd, int argc, const char **argv);
int set_single_color_func(CommandRouter *cmd, int argc, const char **argv);
// Syntax is: {short command, long command, description, syntax}
command_item_t command_list[] = {

  // High-level Commands
  {"about", "Displays information about this device", "about", info_func},
  {"reset", "Runs setup routine again, for resetting LED array", "reboot", reset_func},
  {"version", "Display controller version number", "version", version_func},
  {"license", "Display software license for firmware residing on this teensy", "license", license_func},
  {"?", "Display human-readable help information.", "license", help_func},
  {"store", "Store device parameters", "store", store_func},
  {"recall", "Recall stored device parameters", "recall", recall_func},
  {"autoload", "Toggle/set whether previously stored settings are loaded on power-up. Value is persistant.", "autoload [or] autoload.1", autoload_func},

  // System Parameters
  {"ac", "Toggle clearing of array between led updates. Calling without options toggles the state.", "ac --or-- ac.[0/1]", autoclear_func},
  {"na", "Set na used for bf/df/dpc/cdpc patterns", "na.[na*100]", na_func},
  {"nai", "Sets the inner NA. (nai.20 sets an inner NA of 0.20)  Respected by bf, dpc, and rdpc commands. Default is 0", "nai.20", na_inner_func},
  {"sc", "Set LED array color", "sc,[rgbVal] --or-- sc.[rVal].[gVal].[bVal]", color_func},
  {"ssc", "Set single color channel by index", "ssc.[channel].[cVal]", set_single_color_func},
  {"sb", "Set LED array brightness", "sb,[rgbVal] --or-- sb.[rVal].[gVal].[bVal]", brightness_func},
  {"sad", "Set LED array distance", "sad,[dist (mm)]", array_distance_func},

  // Single (or multiple) LED Display
  {"l", "Turn on a single LED (or multiple LEDs in a list)", "l.[led #].[led #], ...", set_led_func},

  // General Display
  {"x", "Clear the LED array.", "x", clear_func},
  {"ff", "Fill the LED array with default color.", "ff", fill_func},
  {"bf", "Display brightfield pattern", "bf", brightfield_func},
  {"df", "Display darkfield pattern", "df", darkfield_func},
  {"dpc", "Illuminate half-circle (DPC) pattern", "dpc.[t/b/l/r] --or-- dpc.[top/bottom/left/right] --or-- dpc (will raw first pattern)", dpc_func},
  {"cdpc", "Illuminate color DPC (cDPC) pattern", "cdpc.[rVal],[gVal].[bVal]) --or-- cdpc.[rgbVal]) --or-- cdpc", cdpc_func},
  {"an", "Display annulus pattern set by min/max na", "an.[minNA*100].[maxNA*100]", annulus_func},
  {"ha", "Illuminate half annulus", "ha.[type].[minNA*100].[maxNA*100]", half_annulus_func},
  {"dq", "Draws single quadrant", "dq --or-- dq.[quadrant index]", quadrant_func},
  {"cdf", "Draws color darkfield pattern", "cdf.[rVal].[gVal].[bVal]) --or-- cdf.[rgbVal]) --or-- cdf", color_darkfield_func},

  // Single LED Scanning
  {"scf",  "Scan all active LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.", "scf,[delay_ms]", scan_full_func},
  {"scb",  "Scan all brightfield LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.", "scb,[delay_ms]", scan_brightfield_func},
  {"scd",  "Scan all darkfield LEDs. Sends trigger pulse in between images. Outputs LED list to serial terminal.", "scb,[delay_ms]", scan_darkfield_func},

  // Custom Sequence Scanning
  {"ssl",   "Set sequence length in terms of independent patterns", "ssl,[Sequence length]", set_sequence_length_func},
  {"ssv",   "Set sequence value", "ssl.[# Number of LEDs], [LED number 0], [LED number 1]], [LED number 2], ...", set_sequence_value_func},
  {"rseq",  "Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.", "rseq,[Delay between each pattern in ms].[number of times to repeat pattern].[trigger output 0 mode].[trigger input 0 mode].[trigger output 1 mode].[trigger input 1 mode]", run_sequence_func},
  {"pseq",  "Prints sequence values to the terminal", "pseq", print_sequence_func},
  {"sseq",  "Runs sequence with specified delay between each update. If update speed is too fast, a :( is shown on the LED array.", "sseq.[trigger output mode for index 0].[trigger output mode for index 1]", step_sequence_func},
  {"xseq",  "Sets sequence index to start", "xseq", restart_sequence_func},

  // Pre-defined sequences
  {"rdpc", "Runs a DPC sequence with specified delay between each update. If update speed is too fast, a warning message will print.", "rdpc,[Delay between each pattern in ms (can be zero)].[Number of acquisitions].[trigger output mode for trigger output 0].[trigger input mode for trigger input 0].[trigger output mode for trigger output 1].[trigger input mode for trigger input 1]", run_dpc_func},
  {"rfpm", "Runs a FPM sequence with specified delay between each update. If update speed is too fast, a warning message will print.", "rfpm,[Delay between each pattern in ms (can be zero)].[Number of acquisitions].[Maximum NA * 100 (e.g. 0.25NA would be 25].[trigger output mode for trigger output 0].[trigger input mode for trigger input 0].[trigger output mode for trigger output 1].[trigger input mode for trigger input 1]", run_fpm_func},

  // Debugging, Low-level Access, etc.
  {"tr",    "Output TTL trigger pulse to camera", "tr.[trigger index]", trigger_func},
  {"trs",   "Set up hardware (TTL) triggering", "trs.[trigger index].[trigger pin index].['trigger delay between H and L pulses]", trigger_setup_func},
  {"trt",   "Waits for trigger pulses on the defined channel", "trt.[trigger input index]", trigger_test_func},
  {"ch",    "Draw LED by hardware channel (use for debugging)", "dc.[led#]", draw_channel_func},
  {"debug", "Toggle debug flag. Can call with or without options.", "dbg.[command router debug].[LED array (generic) debug].[LED interface debug] --or-- dbg (toggles all between level 1 or 0)", debug_func},
  {"spo",   "Sets pin order (R/G/B) for setup purposes. Also can flip individual leds by passing fourth argument.", "spo.[rChan].[gChan].[bChan] --or-- spo.[led#].[rChan].[gChan].[bChan]", set_pin_order_func},
  {"delay", "Simply puts the device in a loop for the amount of time in ms", "delay.[length of time in ms]", wait_func},
  {"smc",   "Sets max current in amps", "smc.[current limit in amps]", set_max_current_func},
  {"smce",  "Sets whether or not max current limit is enforced (0 is no, all other values are yes)", "smce.[0, 1]", set_max_current_enforcement_func},

  // Quering System State
  {"pvals", "Print led values for software interface", "pvals", print_led_values_func},
  {"pledpos", "Prints the positions of each LED in cartesian coordinates.", "pledpos", print_led_positions_func},
  {"pp",    "Prints system parameters such as NA, LED Array z-distance, etc. in the format of a json file", "pp", print_parameters_func},
  {"pledposna", "Prints the positions of each LED in NA coordinates (NA_x, NA_y, NA_distance", "pledposna", print_led_positions_na},

  // Stored Patterns
  {"disco", "Illuminate a random color pattern of LEDs", "disco.[Number of LEDs in pattern]", disco_func},
  {"demo", "Runs a demo routine to show what the array can do.", "demo", demo_func},
  {"water", "Water drop demo", "water", water_func},

  // Set part and serial number in EEPROM
  {"sn", "Gets device serial number", "sn", get_sn_func},
  {"pn", "Gets device part number", "pn", get_pn_func},

  // Functions to set baud rate and gsclk frequency (for TLC5955 based boards)
  {"sbr", "Sets SPI baud rate for TLC5955 Chips in Hz (baud)", "sbr.1000000", set_baud_rate_func},
  {"sgs", "Sets GSCLK frequency in Hz", "sgs.1000000", set_gsclk_func},

  {"human", "Sets command mode to human-readable", "human", set_human_mode_func},
  {"machine", "Sets command mode to machine-readable", "machine", set_machine_mode_func},

  {"pwrs", "Toggle power source sensing on or off.", "pwrs", power_sensing_func},
  {"pwrv", "Print power sourve voltage.", "pwrv", power_source_voltage_func},

  {"trinputtimeout", "Sets the trigger input timeout in seconds. Default is 3600", "trinputtimeout.10", trigger_input_timeout_func},
  {"troutputpulsewidth", "Sets the trigger pulse width in microseconds, default is 1000.", "troutputpulsewidth.1000", trigger_output_pulse_width_func},
  {"trinputpolarity", "Sets the trigger input polarity. 1=active high, 0=active low. Default is 1.", "trinputpolarity.1", trigger_input_polarity_func},
  {"troutputpolarity", "Sets the trigger output polarity. 1=active high, 0=active low. Default is 1.", "troutputpolarity.1", trigger_output_polarity_func},
  {"troutputdelay", "Sets the trigger delay in microseconds. Default is zero.", "troutputdelay.0", trigger_output_delay_func},
  {"trinputpin", "Returns the Teensy pin of the trigger inputsignal. Used only for debugging.", "trinputpin", trigger_input_pin_func},
  {"troutputpin", "Returns the Teensy pin of the trigger outputsignal. Used only for debugging.", "troutputpin", trigger_output_pin_func},

  {"cos", "Returns or sets the cosine factor, used to scale LED intensity (so outer LEDs are brighter). Input is cos.[integer cosine factor]", "cos.2", cosine_func},

  {"hwinit", "Manufacturer hardware initialization. Modifies persistant settings - do not use unless you know what you're doing.", "hwinit.[sn].[pn]", hw_initialize_function},

  {nullptr, nullptr, nullptr, nullptr}
};

#endif

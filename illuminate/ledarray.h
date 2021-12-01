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

#ifndef LEDARRAY_H
#define LEDARRAY_H

#include "illuminate.h"
#include "ledarrayinterface.h"
#include "ledsequence.h"
#include "constants.h"
#include <Arduino.h>

class LedArray {
  public:

    // LED array control object
    LedArrayInterface * led_array_interface;

    // Device setup and demo
    int reset(uint16_t argc, char ** argv);   // Reset the Array
    int run_demo();    // Run a demo which tests the functions below

    // Pattern commands
    int drawLedList(uint16_t argc, char ** argv);          // Draw a list of LEDs (or just 1)
    int scanBrightfieldLeds(uint16_t argc, char ** argv);  // Scan brightfield LEDs
    int scanDarkfieldLeds(uint16_t argc, char ** argv);  // Scan brightfield LEDs
    int scanAllLeds(uint16_t argc, char ** argv);
    int drawDpc(uint16_t argc, char ** argv);
    int drawBrightfield(uint16_t argc, char ** argv);;
    int drawHalfAnnulus(uint16_t argc, char * *argv);
    int drawColorDarkfield(uint16_t argc, char * * argv);
    int drawAnnulus(uint16_t argc, char * * argv);
    int drawQuadrant(uint16_t argc, char * *argv);
    int drawDarkfield(uint16_t argc, char * *argv);
    int drawCdpc(uint16_t argc, char * *argv);
    int drawNavigator(uint16_t argc, char * *argv);
    int fillArray();
    int clear();
    int drawDiscoPattern();
    int waterDrop();

    // Drawing primatives
    void draw_primative_quadrant(int quadrant_number, float start_na, float end_na, bool include_center);
    void draw_primative_circle(float start_na, float end_na);
    void draw_primative_half_circle(int8_t half_circle_type, float start_na, float end_na);
    void draw_primative_led_scan(uint16_t delay_ms, float start_na, float end_na, bool print_indicies);

    // Triggering
    bool getTriggerState(int trigger_index);
    bool waitForTriggerState(int trigger_index, bool state);
    int triggerInputTest(uint16_t channel);
    int triggerSetup(uint16_t argc, char ** argv);
    int sendTriggerPulse(int trigger_index, bool show_output);
    int setTriggerState(int trigger_index, bool state, bool show_output);

    // Setting system parameters
    int setNa(uint16_t argc, char ** argv);
    int setInnerNa(uint16_t argc, char ** argv);
    int setArrayDistance(uint16_t argc, char ** argv);
    int setColor(int16_t argc, char ** argv);
    int setBrightness(int16_t argc, char ** argv);
    void buildNaList(float boardDistance);
    int toggleAutoClear(uint16_t argc, char ** argv);
    int setMaxCurrentEnforcement(uint16_t argc, char ** argv);
    int setMaxCurrentLimit(uint16_t argc, char ** argv);

    // Sequencing
    int runSequence(uint16_t argc, char ** argv);
    int runSequenceDpc(uint16_t argc, char ** argv);
    int runSequenceFpm(uint16_t argc, char ** argv);
    int runSequenceFast(uint16_t argc, char ** argv);
    int stepSequence(uint16_t argc, char ** argv);
    int setSequenceValue(uint16_t argc, char ** argv);
    int printSequence(uint16_t argc, char ** argv);
    int printSequenceLength(uint16_t argc, char ** argv);
    int resetSequence(uint16_t argc, char ** argv);
    int setSequenceLength(uint16_t argc, char ** argv);
    int setSequenceZeros(uint16_t argc, char ** argv);
    int setSequenceBitDepth(uint16_t argc, char ** argv);
    int setPartNumber(uint16_t argc, char ** argv);
    int setSerialNumber(uint16_t argc, char ** argv);
    int _getSequenceLength();
    int _getSequenceBitDepth();

    int setDebug(uint16_t argc, char ** argv);

    // Printing system state and information
    int printLedPositions(uint16_t argc, char * *argv, bool print_na);
    int printCurrentLedValues(uint16_t argc, char * *argv);
    int printAbout(uint16_t argc, char * *argv);
    int printSystemParams(uint16_t argc, char * *argv);
    int printVersion(uint16_t argc, char * *argv);

    // Internal functions
    void setup();   // Setup command
    int getArgumentLedNumberPitch(char * command_header);
    void setInterface(LedArrayInterface * interface);
    void notImplemented(const char * command_name);
    int drawChannel(uint16_t argc, char * *argv);
    int setPinOrder(uint16_t argc, char * *argv);
    int getColorChannelCount();
    static void tripTimer();
    static void patternIncrementFast();
    void scanLedRange(uint16_t delay_ms, float start_na, float end_na, bool print_indicies);

    int printMacAddress();
    int setBaudRate(uint16_t argc, char ** argv);
    int setGsclkFreq(uint16_t argc, char ** argv);
    int setCommandMode(const char * mode);

    // Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
    int setLed(int16_t led_number, int16_t color_channel_index, uint16_t value);     // LED brightness (16-bit)
    int setLed(int16_t led_number, int16_t color_channel_index, uint8_t value);      // LED brightness (8-bit)
    int setLed(int16_t led_number, int16_t color_channel_index, bool value);         // LED brightness (boolean)

    // Device-specific commands
    uint8_t getDeviceCommandCount();
    const char * getDeviceCommandNameShort(int device_command_index);
    const char * getDeviceCommandNameLong(int device_command_index);
    int deviceCommand(int device_command_index, uint16_t argc, char * *argv);

    // Demo mode
    int setDemoMode(uint16_t argc, char ** argv);

    // Printing
    void print(const char * short_command, const char * long_command);
    void clearOutputBuffers();

    // Short and long responses
    char output_buffer_short[MAX_RESPONSE_LENGTH_SHORT];
    char output_buffer_long[MAX_RESPONSE_LENGTH_LONG];

    // Source voltage sensing
    int print_power_supply_plugged(uint16_t argc, char ** argv);
    int set_power_supply_sensing(uint16_t argc, char ** argv);
    int print_power_supply_voltage(uint16_t argc, char ** argv);

    // Trigger Configuration
    int setTriggerInputTimeout(uint16_t argc, char ** argv);
    int setTriggerOutputPulseWidth(uint16_t argc, char ** argv);
    int setTriggerOutputDelay(uint16_t argc, char ** argv);
    int setTriggerInputPolarity(uint16_t argc, char ** argv);
    int setTriggerOutputPolarity(uint16_t argc, char ** argv);
    int getTriggerInputPins(uint16_t argc, char ** argv);
    int getTriggerOutputPins(uint16_t argc, char ** argv);

    int setCosineFactor(uint16_t argc, char ** argv);
    void calculate_max_na();

    int store_parameters(uint16_t argc, char * *argv);
    int recall_parameters(uint16_t argc, char * *argv);

  private:

    /* DPC Commands */
    const char * DPC_RIGHT1 = "r";
    const char * DPC_RIGHT2 = "right";

    const char * DPC_LEFT1 = "l";
    const char * DPC_LEFT2 = "left";

    const char * DPC_TOP1 = "t";
    const char * DPC_TOP2 = "top";

    const char * DPC_BOTTOM1 = "b";
    const char * DPC_BOTTOM2 = "bottom";

    // Defualt brightness
    const uint8_t LED_VALUE_DEFAULT = 10;

    // LED sequence object for storage and retreival
    static LedSequence led_sequence;

    // LED Controller Parameters
    boolean auto_clear_flag = true;
    boolean initial_setup = true;
    int debug_level = 0;
    float objective_na = 0.25;
    float inner_na = 0.0;
    float led_array_distance_z = 60.0;
    int color_channel_count = 3;
    char * device_name;
    int8_t default_brightness = 63;
    uint8_t cosine_factor;
    bool normalize_color = true;

    // Trigger Input (feedback) Settings
    static volatile float trigger_feedback_timeout_ms;
    static volatile uint32_t * trigger_output_pulse_width_list_us;
    static volatile uint32_t * trigger_output_start_delay_list_us;
    static volatile int * trigger_input_mode_list;
    static volatile int * trigger_output_mode_list;
    static volatile bool * trigger_input_polarity_list;
    static volatile bool * trigger_output_polarity_list;
    static volatile int trigger_input_count;
    static volatile int trigger_output_count;
    static volatile float trigger_input_timeout;
    static const int * trigger_output_pin_list;
    static const int * trigger_input_pin_list;

    // Default illumination
    uint8_t * led_value; // Current led values for each channel
    uint8_t * led_color;          // 8-bit color balance
    uint8_t led_brightness;  // 8-bit brightness

    // Sequence stepping index
    uint16_t sequence_number_displayed = 0;

    // timer variable
    static volatile uint16_t pattern_index;
    static volatile uint16_t frame_index;

    // Command mode
    bool command_mode = COMMAND_MODE_LONG;

    // These should be made persistant
    const float led_array_distance_z_default = 50.0;
    const float na_default = 1.0;
    const uint8_t led_brightness_default = 255;
    const float inner_na_default = 0.0;
    const uint8_t cosine_factor_default = 0.0;

    // Internal variable for maximum na of this array, at this geometry.
    float max_na;
};

#endif

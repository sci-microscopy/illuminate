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

#ifndef LED_ARRAY_H
#define LED_ARRAY_H

#include "ledarrayinterface.h"
#include "ledsequence.h"
#include "illuminate.h"
#include <Arduino.h>

// Trigger mode constants
#define TRIG_MODE_NONE 0
#define TRIG_MODE_ITERATION -1   // trigger at the start of each iteration (when the user
#define TRIG_MODE_START -2   // Triggering at the start of each acquisition

// Trigger timing constants
#define TRIGGER_OUTPUT_PULSE_WIDTH_DEFAULT 500
#define TRIGGER_OUTPUT_DELAY_DEFAULT 0
#define TRIGGER_TIMEOUT_DEFAULT 3600.0
#define TRIGGER_INPUT_POLARITY_DEFAULT 1
#define TRIGGER_OUTPUT_POLARITY_DEFAULT 1

// Misc constants
#define PVALS_USE_UINT8 1     // Whether to return uint8 for pvals instead of 16-bit (Default is on)
#define MIN_SEQUENCE_DELAY 5  // Min deblur pattern delay in ms (set by hardware)
#define MIN_SEQUENCE_DELAY_FAST 2 // Min deblur pattern delay for fast sequence in us (set by hardware)
#define DELAY_MAX 2000        // Global maximum amount to wait inside loop
#define INVALID_NA -2000.0    // Rep```resents an invalid NA
#define DEFAULT_NA 1.0         // 100 * default NA, int

#define LED_BRIGHTNESS_DEFAULT 255

// Command mode constants
#define COMMAND_MODE_LONG 1
#define COMMAND_MODE_SHORT 0

// Response Lengths
#define MAX_RESPONSE_LENGTH_SHORT 100
#define MAX_RESPONSE_LENGTH_LONG 100

// Bit depth to use
#define USE_8_BIT_VALUES 1

class LedArray {
  public:

      // LED array control object
    LedArrayInterface * led_array_interface;

    // Device setup and demo
    void reset();   // Reset the Array
    void demo();    // Run a demo which tests the functions below

    // Pattern commands
    void drawLedList(uint16_t argc, char ** argv);          // Draw a list of LEDs (or just 1)
    void scanBrightfieldLeds(uint16_t argc, char ** argv);  // Scan brightfield LEDs
    void scanAllLeds(uint16_t argc, char ** argv);
    void drawDpc(uint16_t argc, char ** argv);
    void drawBrightfield(uint16_t argc, char ** argv);;
    void drawHalfAnnulus(int argc, char * *argv);
    void drawColorDarkfield(int argc, char * * argv);
    void drawAnnulus(int argc, char * * argv);
    void drawDarkfield();
    void drawCdpc(int argc, char * *argv);
    void drawNavDpc();
    void fillArray();
    void clear();
    void drawDiscoPattern();
    void waterDrop();

    // Drawing primatives
    void drawQuadrant(int quadrant_number, float start_na, float end_na, bool include_center);
    void drawCircle(float start_na, float end_na);
    void drawHalfCircle(int8_t half_circle_type, float start_na, float end_na);
    void scanLedRange(uint16_t delay_ms, float start_na, float end_na, bool print_indicies);

    // Triggering
    bool getTriggerState(int trigger_index);
    bool waitForTriggerState(int trigger_index, bool state);
    void triggerInputTest(uint16_t channel);
    void triggerSetup(int argc, char ** argv);
    void sendTriggerPulse(int trigger_index, bool show_output);
    void setTriggerState(int trigger_index, bool state, bool show_output);

    // Setting system parameters
    void setNa(int argc, char ** argv);
    void setInnerNa(int argc, char ** argv);
    void setArrayDistance(int argc, char ** argv);
    void setColor(int16_t argc, char ** argv);
    void setBrightness(int16_t argc, char ** argv);
    void buildNaList(float boardDistance);
    void toggleAutoClear(uint16_t argc, char ** argv);
    void setMaxCurrentEnforcement(int argc, char ** argv);
    void setMaxCurrentLimit(int argc, char ** argv);

    // Sequencing
    int getSequenceBitDepth();
    void runSequence(uint16_t argc, char ** argv);
    void runSequenceDpc(uint16_t argc, char ** argv);
    void runSequenceFpm(uint16_t argc, char ** argv);
    void runSequenceFast(uint16_t argc, char ** argv);
    void stepSequence(uint16_t argc, char ** argv);
    void setSequenceValue(uint16_t argc, void ** led_values, int16_t * led_numbers);
    void printSequence();
    void printSequenceLength();
    void resetSequence();
    void setSequenceLength(uint16_t new_seq_length, bool quiet);
    int getSequenceLength();
    void setSequenceBitDepth(uint8_t bit_depth, bool quiet);
    void setSequenceZeros(uint16_t argc, char ** argv);

    // Printing system state and information
    void printLedPositions(uint16_t argc, char * *argv, bool print_na);
    void printCurrentLedValues(uint16_t argc, char * *argv);
    void printAbout();
    void printSystemParams();
    void printVersion();

    // Internal functions
    void setDebug(uint16_t new_debug_level);
    void setup();   // Setup command
    int getArgumentLedNumberPitch(char * command_header);
    void setInterface(LedArrayInterface * interface);
    void notImplemented(const char * command_name);
    void drawChannel(int argc, char * *argv);
    void setPinOrder(int argc, char * *argv);
    void notFinished(const char * command_name);
    int getColorChannelCount();
    static void tripTimer();
    static void patternIncrementFast();
    uint16_t getSerialNumber();
    uint16_t getPartNumber();
    void setPartNumber(uint16_t part_number);
    void setSerialNumber(uint16_t serial_number);
    void printMacAddress();
    void setBaudRate(uint16_t argc, char ** argv);
    void setGsclkFreq(uint16_t argc, char ** argv);
    void setCommandMode(const char * mode);
    
    // Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
    void setLed(int16_t led_number, int16_t color_channel_index, uint16_t value);     // LED brightness (16-bit)
    void setLed(int16_t led_number, int16_t color_channel_index, uint8_t value);      // LED brightness (8-bit)
    void setLed(int16_t led_number, int16_t color_channel_index, bool value);         // LED brightness (boolean)

    // Device-specific commands
    uint8_t getDeviceCommandCount();
    const char * getDeviceCommandNameShort(int device_command_index);
    const char * getDeviceCommandNameLong(int device_command_index);
    void deviceCommand(int device_command_index, int argc, char * *argv);

    // Demo mode
    void setDemoMode(int8_t mode);
    int8_t getDemoMode();

    // Printing
    void print(const char * short_command, const char * long_command);
    void clearOutputBuffers();

    // Short and long responses
    char output_buffer_short[MAX_RESPONSE_LENGTH_SHORT];
    char output_buffer_long[MAX_RESPONSE_LENGTH_LONG];

    // Error codes
    void error(int16_t error_code, const char * calling_function);

    // Source voltage sensing
    void isPowerSourcePluggedIn();
    void togglePowerSupplySensing();
    void printPowerSourceVoltage();

    // Trigger Configuration
    void setTriggerInputTimeout(int argc, char ** argv);
    void setTriggerOutputPulseWidth(int argc, char ** argv);
    void setTriggerOutputDelay(int argc, char ** argv);
    void setTriggerInputPolarity(int argc, char ** argv);
    void setTriggerOutputPolarity(int argc, char ** argv);
    void getTriggerInputPins(int argc, char ** argv);
    void getTriggerOutputPins(int argc, char ** argv);

    void setCosineFactor(int argc, char ** argv);

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
    int debug = 0;
    float objective_na = 0.25;
    float inner_na = 0.0;
    float led_array_distance_z = 60.0;
    int color_channel_count = 3;
    char * device_name;
    int8_t default_brightness = 63;
    int8_t cosine_factor;
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
    uint8_t led_brightness = 10;  // 8-bit brightness

    // Sequence stepping index
    uint16_t sequence_number_displayed = 0;

    // timer variable
    static volatile uint16_t pattern_index;
    static volatile uint16_t frame_index;

    // Command mode
    bool command_mode = COMMAND_MODE_LONG;
};
#endif

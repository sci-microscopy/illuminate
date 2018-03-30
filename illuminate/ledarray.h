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

#ifndef LED_ARRAY_H
#define LED_ARRAY_H

#include "ledarrayinterface.h"
#include "ledsequence.h"
#include "illuminate.h"

#include <Arduino.h>
#include <EEPROM.h>

// Trigger mode constants
#define TRIG_MODE_NONE 0
#define TRIG_MODE_ITERATION -1   // trigger at the start of each iteration (when the user
#define TRIG_MODE_START -2   // Triggering at the start of each acquisition

// Trigger timing constants
#define TRIGGER_PULSE_WIDTH_DEFAULT 500
#define TRIGGER_DELAY_DEFAULT 0
#define MAX_TRIGGER_WAIT_TIME_S 20.0

// Serial and part number addresses in EEPROM
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// Annulus constants
#define ANNULUS_START_OFFSET 0.03
#define ANNULUS_END_OFFSET 0.1

// Misc constants
#define PVALS_USE_UINT8 1     // Whether to return uint8 for pvals instead of 16-bit (Default is on)
#define MIN_SEQUENCE_DELAY 5  // Min deblur pattern delay in ms (set by hardware)
#define MIN_SEQUENCE_DELAY_FAST 2 // Min deblur pattern delay for fast sequence in us (set by hardware)
#define DELAY_MAX 2000        // Global maximum amount to wait inside loop
#define INVALID_NA -2000.0    // Represents an invalid NA
#define DEFAULT_NA 0.25         // 100 * default NA, int

class LedArray {
  public:
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
    void printTriggerSettings();
    void waitForTriggerState(int trigger_index, bool state);
    void triggerInputTest(uint16_t channel);
    void triggerSetup(int argc, char ** argv);
    void sendTriggerPulse(int trigger_index, bool show_output);
    void setTriggerState(int trigger_index, bool state, bool show_output);

    // Setting system parameters
    void setNa(int argc, char ** argv);
    void setDistanceZ(int argc, char ** argv);
    void setColor(int16_t argc, char ** argv);
    void clearNaList();
    void buildNaList(float boardDistance);
    void toggleAutoClear(uint16_t argc, char ** argv);

    // Sequencing
    int getSequenceBitDepth();
    void runSequence(uint16_t argc, char ** argv);
    void runSequenceFast(uint16_t argc, char ** argv);
    void stepSequence(uint16_t argc, char ** argv);
    void setSequenceValue(uint16_t argc, void ** led_values, int16_t * led_numbers);
    void printSequence();
    void printSequenceLength();
    void resetSequence();
    void setSequenceLength(uint16_t new_seq_length, bool quiet);
    int getSequenceLength();
    void setSequenceBitDepth(uint8_t bit_depth, bool quiet);

    // Printing system state and information
    void printLedPositions(bool print_na);
    void printCurrentLedValues();
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

    // LED Positions in NA coordinates
    float * * led_position_list_na;

    // Controller version
    const float version = 0.2;

    // LED array control object
    LedArrayInterface * led_array_interface;

    // LED sequence object for storage and retreival
    static LedSequence led_sequence;

    // LED Controller Parameters
    boolean auto_clear_flag = true;
    int debug = 0;
    float objective_na = 0.25;
    float led_array_distance_z = 60.0;
    int color_channel_count = 3;
    char * device_name;
    int8_t default_brightness = 63;

    // Trigger Input (feedback) Settings
    float trigger_feedback_timeout_ms = 1000;
    uint16_t * trigger_pulse_width_list_us;
    uint32_t * trigger_start_delay_list_us;
    int * trigger_input_mode_list;
    int * trigger_output_mode_list;

    // Default illumination
    uint8_t * led_value;

    // Sequence stepping index
    uint16_t sequence_number_displayed = 0;

    // timer variable
    static volatile bool timer_tripped;
    static volatile uint16_t pattern_index;

};
#endif


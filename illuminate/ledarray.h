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
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LED_ARRAY_H
#define LED_ARRAY_H

#include "quasidome.h"
#include "ledsequence.h"

#include <Arduino.h>

// Trigger Mode constants
#define TRIG_MODE_NONE 0
#define TRIG_MODE_ITERATION -1   // trigger at the start of each iteration (when the user
#define TRIG_MODE_START -2   // Triggering at the start of each acquisition

// Annulus constants
#define ANNULUS_START_OFFSET 0.03
#define ANNULUS_END_OFFSET 0.1

// Misc constants
#define PVALS_USE_UINT8 1     // Whether to return uint8 for pvals instead of 16-bit (Default is on)
#define MIN_SEQUENCE_DELAY 5  // Min deblur pattern delay in ms (set by hardware)
#define MIN_SEQUENCE_DELAY_FAST 2 // Min deblur pattern delay for fast sequence in us (set by hardware)
#define DELAY_MAX 2000        // Global maximum amount to wait inside loop
#define INVALID_NA -2000.0    // Represents an invalid NA
#define DEFAULT_NA 25         // 100 * default NA, int

class LedArray {
  public:
    void showVersion();
    void notFinished(const char * command_name);
    void printAbout();
    void printSystemParams();
    void resetArray();
    void drawDiscoPattern(uint16_t nLed);
    //    void printLedPositionsNa(uint16_t argc, char ** argv, char seperator);
    void setup();
    void demo();
    void clearNaList();
    void buildNaList(float boardDistance);
    void fillArray();
    void clearArray();
    void setNa(int8_t new_na);
    void printTriggerSettings();
    void drawDarkfield();
    void drawCdpcParser(int argc, char * *argv);
    void drawHalfAnnulusParser(int argc, char * *argv);
    void drawColorDarkfieldParser(int argc, char * * argv);
    void drawNavdpcParser(int argc, char * * argv);
    void drawAnnulusParser(int argc, char * * argv);
    void drawChannel(uint16_t tmp);
    void setPinOrder(int argc, char * *argv);
    void drawPulsePattern(uint16_t tmp);
    void triggerSetup(int argc, char ** argv);
    void sendTriggerPulse(int trigger_index, bool show_output);
    void setTriggerState(int trigger_index, bool state, bool show_output);
    void drawSingleLed(int16_t led_number);
    void drawLedList(uint16_t argc, char ** argv);
    void scanBrightfieldLeds(uint16_t argc, char ** argv);
    void scanAllLeds(uint16_t argc, char ** argv);
    void setColor(int16_t argc, char ** argv);
    void drawQuadrant(int8_t qNumber);
    void drawHalfCircle(int8_t halfCircleType);
    void drawDpc(char * optionalParam);
    void drawBrightfield();
    void setSequenceLength(uint16_t new_seq_length, bool quiet);
    void setSequenceBitDepth(uint8_t bit_depth, bool quiet);
    int getArgumentLedNumberPitch(char * command_header);
    void setSequenceValue(uint16_t argc, void ** led_values, int16_t * led_numbers);
    void printSequence();
    void printSequenceLength();
    void resetSequence();
    void waitForTriggerState(int trigger_index, bool state);
    void triggerInputTest(uint16_t channel);
    void runSequence(uint16_t argc, char ** argv);
    void runSequenceFast(uint16_t argc, char ** argv);
    void stepSequence(uint16_t argc, char ** argv);
    void setDistanceZ(float new_z);
    int getSequenceBitDepth();
    void toggleDebug(uint16_t argc, char ** argv);
    void toggleAutoClear(uint16_t argc, char ** argv);
    void drawSpiralPattern(uint16_t delay_ms);
    void setInterface(LedArrayInterface * interface);
    void printLedPositions();
    void printCurrentLedValues();
    void printDeviceName();

    int color_channel_count = 1;
    char * device_name;

    // LED Positions in NA coordinates
    float * * led_position_list_na;
  private:

    // Interrupt service routines
    static void isr0();
    static void isr1();

    /* DPC Commands */
    const char * DPC_RIGHT1 = "r";
    const char * DPC_RIGHT2 = "right";

    const char * DPC_LEFT1 = "l";
    const char * DPC_LEFT2 = "left";

    const char * DPC_TOP1 = "t";
    const char * DPC_TOP2 = "top";

    const char * DPC_BOTTOM1 = "b";
    const char * DPC_BOTTOM2 = "bottom";

    // Controller version
    const float version = 0.2;

    // LED array control object
    LedArrayInterface * led_array_interface;

    // LED sequence object for storage and retreival
    LedSequence led_sequence;

    // LED Controller Parameters
    boolean auto_clear_flag = true;
    bool opt_flag = false;
    bool debug = 1;
    float objective_na = 0.25;
    float led_array_interface_distance_z = 50.0;

    // Trigger Input (feedback) Settings
    float trigger_feedback_timeout_ms = 1000;
    uint16_t * trigger_pulse_width_list_us;
    uint16_t * trigger_start_delay_list_ms;
    int * trigger_input_mode_list;
    int * trigger_output_mode_list;

    // Default illumination
    uint16_t * led_value;

    // Sequence stepping index
    uint16_t sequence_number_displayed = 0;

};
#endif

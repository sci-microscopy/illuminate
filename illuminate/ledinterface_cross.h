/*
Copyright (c) 2018, Zachary Phillips (UC Berkeley)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
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

#ifndef LED_ARRAY_INTERFACE_H
#define LED_ARRAY_INTERFACE_H

#include <Arduino.h>

// Pin definitions (used internally)
#define GSCLK 6 // 10 on Arduino Mega
#define LAT 2   // 44 on Arduino Mega
#define SPI_MOSI 11
#define SPI_CLK 13
#define TRIGGER_OUTPUT_PIN_0 23
#define TRIGGER_OUTPUT_PIN_1 22
#define TRIGGER_INPUT_PIN_0 20
#define TRIGGER_INPUT_PIN_1 19
#define TRIGGER_OUTPUT_COUNT 2
#define TRIGGER_INPUT_COUNT 2

#define Q1_PIN 5
#define Q2_PIN 6
#define Q3_PIN 9
#define Q4_PIN 10

#define SN 10001

class LedArrayInterface {
  public:
    void deviceSetup();
    void setLed(uint16_t led_number, uint8_t value);
    void setLedFast(uint16_t led_number, bool value);
    void setAllLeds(uint8_t value);
    void setAllLedsFast(bool value);
    void clear();
    int sendTriggerPulse(int trigger_index, uint16_t delay_us, bool inverse_polarity);
    int setTriggerState(int trigger_index, bool state);
    int getInputTriggerState(int input_trigger_index);
    void update();
    static void triggerInputChange_0();
    static void triggerInputChange_1();
    uint16_t getLedValue(uint16_t led_number);
    void getTriggerPins(int * * pin_numbers);
    bool getDebug();
    void setDebug(bool state);
    int * getTriggerOutputPins();
    int * getTriggerInputPins();
    
    // Device and Software Descriptors
    const char * device_name = "cross-array";
    const int serial_number = SN;
    const char * device_hardware_revision = "1.0";
    const float max_na = 1.0;
    const int16_t led_count = 4;
    const uint16_t center_led = 0;
    const int trigger_output_count = 2;
    const int trigger_input_count = 2;
    const int color_channel_count = 1;
    const char * color_channel_names[1] = {"w"};
    const float color_channel_center_wavelengths[1] = {0.53};
    const int bit_depth = 8;

    // Triggering Variables
    static int trigger_output_pin_list[TRIGGER_OUTPUT_COUNT];
    static int trigger_input_pin_list[TRIGGER_INPUT_COUNT];

    // Trigger state Variables
    static volatile bool trigger_input_state[TRIGGER_INPUT_COUNT];
    static volatile bool debug;

    // Current LED values
    uint8_t led_values[4] = {0, 0, 0, 0};

  private:
    int pin_numbers[4] = {Q1_PIN, Q2_PIN, Q3_PIN, Q4_PIN};
};


// FORMAT: LED number, channel, 100*x, 100*y, 100*z
PROGMEM const int16_t ledMap[4][5] = {
  {0, 1, -1, 1, 4000,},
  {1, 0, 1, 1, 4000,},
  {2, 2, 1, -1, 4000,},
  {3, 3, -1, -1, 4000,}
};

#endif

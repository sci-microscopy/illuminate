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


#ifndef LED_ARRAY_INTERFACE_H
#define LED_ARRAY_INTERFACE_H

#include <Arduino.h>
#include <EEPROM.h>

class LedArrayInterface {
  public:
    void deviceSetup();
    void deviceReset();

    // Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
    void setLed(int16_t led_number, int16_t color_channel_index, uint16_t value);     // LED brightness (16-bit)
    void setLed(int16_t led_number, int16_t color_channel_index, uint8_t value);      // LED brightness (8-bit)
    void setLed(int16_t led_number, int16_t color_channel_index, bool value);         // LED brightness (boolean)

    // Fast LED update
    void setLedFast(int16_t led_number, int color_channel_index, bool value);

    // Get LED Value
    uint16_t getLedValue(uint16_t led_number, int color_channel_index);

    // Clear array
    void clear();

    // Get and set trigger state
    int sendTriggerPulse(int trigger_index, uint16_t delay_us, bool inverse_polarity);
    int setTriggerState(int trigger_index, bool state);
    int getInputTriggerState(int input_trigger_index);

    // Update array
    void update();

    // Debug
    bool getDebug();
    void setDebug(int state);

    // Set channel
    void setChannel(int16_t channel_number, int16_t color_channel_number, uint16_t value);
    void setChannel(int16_t channel_number, int16_t color_channel_number, uint8_t value);
    void setChannel(int16_t channel_number, int16_t color_channel_number, bool value);
    void setChannelFast(uint16_t channel_number, int color_channel_index, bool value);

    // Set pin order
    void setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position);

    // Maximum current limits
    void setMaxCurrentEnforcement(bool enforce);
    void setMaxCurrentLimit(float limit);

    // Not implemented function
    void notImplemented(const char * command_name);

    // Device and Software Descriptors
    static const char * device_name;
    static const char * device_hardware_revision;
    static const float max_na;
    static const float default_na;
    static const int16_t led_count;
    static const uint16_t center_led;
    static const int trigger_output_count;
    static const int trigger_input_count;
    static const int color_channel_count;
    static const char color_channel_names[];
    static const float color_channel_center_wavelengths[];
    static const int bit_depth;
    static const int16_t tlc_chip_count;
    static const bool supports_fast_sequence;
    static const float led_array_distance_z_default;
    static const char * deviceCommandNamesShort[];
    static const char * deviceCommandNamesLong[];
    static const uint8_t device_command_count;
    static const uint16_t device_command_pattern_dimensions[][2];
    static const uint16_t *device_command_pattern_list[];
    static const int power_sense_pin;
    static const float min_source_voltage;

    // Debug flag
    static int debug;

    // Triggering Variables
    static const int trigger_output_pin_list[];
    static const int trigger_input_pin_list[];
    static bool trigger_input_state[];

    // LED positions
    static const int16_t PROGMEM led_positions[][5];
    static const int16_t PROGMEM arbitrary_led_list[][1];
    static float led_position_list_na[][2];

    // Device-specific commands
    uint8_t getDeviceCommandCount();
    const char * getDeviceCommandNameShort(int device_command_index);
    const char * getDeviceCommandNameLong(int device_command_index);
    uint32_t getDeviceCommandLedListSize(int device_command_index);
    uint16_t getDeviceCommandLedListElement(int device_command_index, uint16_t pattern_index, uint16_t led_index);

    // Serial and part numbers
    uint16_t getSerialNumber();
    uint16_t getPartNumber();
    void setPartNumber(uint16_t part_number);
    void setSerialNumber(uint16_t serial_number);

    // Hardware-related functions
    void setBaudRate(uint32_t new_baud_rate);
    uint32_t getBaudRate();
    void setGsclkFreq(uint32_t new_gsclk_freq);
    uint32_t getGsclkFreq();
    float getSourceVoltage();

    // Demo Mode
    void setDemoMode(int8_t mode);
    int8_t getDemoMode();
};


#endif

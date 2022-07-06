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

#ifndef LEDARRAYINTERFACE_H
#define LEDARRAYINTERFACE_H

#include "illuminate.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "constants.h"

class LedArrayInterface {
  public:
    static int8_t device_setup();
    int8_t device_reset();

    // Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
    void set_led(int16_t led_number, int16_t color_channel_index, uint16_t value);     // LED brightness (16-bit)
    void set_led(int16_t led_number, int16_t color_channel_index, uint8_t value);      // LED brightness (8-bit)
    void set_led(int16_t led_number, int16_t color_channel_index, bool value);         // LED brightness (boolean)

    // Get LED Value
    uint16_t get_led_value(uint16_t led_number, int color_channel_index);

    // Clear array
    static void clear();

    // Get and set trigger state
    int send_trigger_pulse(int trigger_index, uint16_t delay_us, bool inverse_polarity);
    int set_trigger_state(int trigger_index, bool state);
    int get_input_trigger_state(int input_trigger_index);

    // Update array
    static void update();

    // Debug
    bool get_debug();
    void set_debug(int state);

    // Set channel
    void set_channel(int16_t channel_number, int16_t color_channel_number, uint16_t value);
    void set_channel(int16_t channel_number, int16_t color_channel_number, uint8_t value);
    void set_channel(int16_t channel_number, int16_t color_channel_number, bool value);

    // Set pin order
    void set_pin_order(int16_t led_number, int16_t color_channel_index, uint8_t position);

    // Maximum current limits
    void set_max_current_enforcement(bool enforce);
    bool get_max_current_enforcement();
    void set_max_current_limit(float limit);
    float get_max_current_limit();

    // Not implemented function
    void not_implemented(const char * command_name);
    
    // Device and Software Descriptors
    static const char * device_name;
    static const char * device_hardware_revision;
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
    static const char * device_commandNamesShort[];
    static const char * device_commandNamesLong[];
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

    // LED positionsde
    static const int16_t PROGMEM led_positions[][5];
    static const int16_t PROGMEM arbitrary_led_list[][1];
    static float led_position_list_na[][2];

    // Device-specific commands
    uint8_t get_device_command_count();
    const char * get_device_command_name_short(int device_command_index);
    const char * get_device_command_name_long(int device_command_index);
    uint32_t get_device_command_led_list_size(int device_command_index);
    uint16_t get_device_command_led_list_element(int device_command_index, uint16_t pattern_index, uint16_t led_index);

    // Serial and part numbers
    uint16_t get_serial_number();
    uint16_t get_part_number();
    void set_part_number(uint16_t part_number);
    void set_serial_number(uint16_t serial_number);

    // Hardware-related functions
    void set_sclk_baud_rate(uint32_t new_baud_rate);
    uint32_t get_sclk_baud_rate();
    void set_gsclk_frequency(uint32_t new_gsclk_freq);
    uint32_t get_gsclk_frequency();

    // Source voltage checking
    int16_t source_sense_pin = -1;
    int16_t source_reference_value = -1;
    float get_power_source_voltage();
    bool is_power_source_plugged_in();
    int16_t get_device_power_sensing_capability();
    bool get_power_source_monitoring_state();
    void set_power_source_monitoring_state(int state);
    void source_change_interrupt();

    int8_t set_register(uint32_t register, int8_t value);
    int8_t get_register(uint32_t register);
};

#endif

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
    int draw_led_list(uint16_t argc, char ** argv);          // Draw a list of LEDs (or just 1)
    int scan_brightfield_leds(uint16_t argc, char ** argv);  // Scan brightfield LEDs
    int scan_darkfield_leds(uint16_t argc, char ** argv);  // Scan brightfield LEDs
    int scan_all_leds(uint16_t argc, char ** argv);
    int draw_dpc(uint16_t argc, char ** argv);
    int draw_brightfield(uint16_t argc, char ** argv);;
    int draw_half_annulus(uint16_t argc, char * *argv);
    int draw_color_darkfield(uint16_t argc, char * * argv);
    int draw_annulus(uint16_t argc, char * * argv);
    int draw_quadrant(uint16_t argc, char * *argv);
    int draw_darkfield(uint16_t argc, char * *argv);
    int draw_cdpc(uint16_t argc, char * *argv);
    int fill_array();
    int clear();
    int disco();
    int water_drop();

    // Drawing primatives
    void draw_primative_quadrant(int quadrant_number, float start_na, float end_na, bool include_center);
    void draw_primative_circle(float start_na, float end_na);
    void draw_primative_half_circle(int8_t half_circle_type, float start_na, float end_na);
    void draw_primative_led_scan(uint16_t delay_ms, float start_na, float end_na, bool print_indicies);

    // Triggering
    bool get_trigger_state(int trigger_index);
    bool wait_for_trigger_state(int trigger_index, bool state);
    int trigger_input_test(uint16_t channel);
    int trigger_setup(uint16_t argc, char ** argv);
    int send_trigger_pulse(int trigger_index, bool show_output);
    int set_trigger_state(int trigger_index, bool state, bool show_output);

    // Setting system parameters
    int set_na(uint16_t argc, char ** argv);
    int set_inner_na(uint16_t argc, char ** argv);
    int set_array_distance(uint16_t argc, char ** argv);
    int set_color(int16_t argc, char ** argv);
    int set_brightness(int16_t argc, char ** argv);
    void build_na_list(float boardDistance);
    int set_auto_clear(uint16_t argc, char ** argv);
    int set_max_current_enforcement(uint16_t argc, char ** argv);
    int set_max_current_limit(uint16_t argc, char ** argv);

    // Sequencing
    int run_sequence(uint16_t argc, char ** argv);
    int run_sequence_dpc(uint16_t argc, char ** argv);
    int run_sequence_fpm(uint16_t argc, char ** argv);
    int step_sequence(uint16_t argc, char ** argv);
    int set_sequence_value(uint16_t argc, char ** argv);
    int print_sequence(uint16_t argc, char ** argv);
    int restart_sequence(uint16_t argc, char ** argv);
    int set_sequence_length(uint16_t argc, char ** argv);

    int print_part_number(uint16_t argc, char ** argv);
    int print_serial_number(uint16_t argc, char ** argv);

    int set_debug(uint16_t argc, char ** argv);

    // Printing system state and information
    int print_led_positions(uint16_t argc, char * *argv, bool print_na);
    int print_led_values(uint16_t argc, char * *argv);
    int print_about(uint16_t argc, char * *argv);
    int print_system_parameters(uint16_t argc, char * *argv);
    int print_version(uint16_t argc, char * *argv);

    // Internal functions
    void setup();   // Setup command
    void set_interface(LedArrayInterface * interface);
    void not_implemented(const char * command_name);
    int draw_channel(uint16_t argc, char * *argv);
    int set_pin_order(uint16_t argc, char * *argv);
    void scan_led_range(uint16_t delay_ms, float start_na, float end_na, bool print_indicies);

    int print_mac_address();
    int set_sclk_baud_rate(uint16_t argc, char ** argv);
    int set_gsclk_frequency(uint16_t argc, char ** argv);
    int set_command_mode(const char * mode);

    // Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
    int set_led(int16_t led_number, int16_t color_channel_index, uint16_t value);     // LED brightness (16-bit)
    int set_led(int16_t led_number, int16_t color_channel_index, uint8_t value);      // LED brightness (8-bit)
    int set_led(int16_t led_number, int16_t color_channel_index, bool value);         // LED brightness (boolean)

    // Device-specific commands
    uint8_t get_device_command_count();
    const char * get_device_command_name_short(int device_command_index);
    const char * get_device_command_name_long(int device_command_index);
    int device_command(int device_command_index, uint16_t argc, char * *argv);

    // Demo mode
    int set_demo_mode(uint16_t argc, char ** argv);

    // Printing
    void print(const char * short_command, const char * long_command);
    void clear_output_buffers();

    // Short and long responses
    char output_buffer_short[MAX_RESPONSE_LENGTH_SHORT];
    char output_buffer_long[MAX_RESPONSE_LENGTH_LONG];

    // Source voltage sensing
    int print_power_supply_plugged(uint16_t argc, char ** argv);
    int set_power_supply_sensing(uint16_t argc, char ** argv);
    int print_power_supply_voltage(uint16_t argc, char ** argv);

    // Trigger Configuration
    int set_trigger_input_timeout(uint16_t argc, char ** argv);
    int set_trigger_output_pulse_width(uint16_t argc, char ** argv);
    int set_trigger_output_delay(uint16_t argc, char ** argv);
    int set_trigger_input_polarity(uint16_t argc, char ** argv);
    int set_trigger_output_polarity(uint16_t argc, char ** argv);
    int get_trigger_input_pins(uint16_t argc, char ** argv);
    int get_trigger_output_pins(uint16_t argc, char ** argv);

    int set_cosine_factor(uint16_t argc, char ** argv);
    void calculate_max_na();

    int8_t store_parameters();
    int8_t recall_parameters(bool quiet);

    // Demo Mode
    int8_t set_demo_mode(int8_t mode);
    int8_t get_demo_mode();

    int8_t set_autoload_on_reboot(uint16_t argc, char ** argv);

    uint16_t get_serial_number();
    void set_serial_number(uint16_t serial_number);
    uint16_t get_part_number();
    void set_part_number(uint16_t part_number);

    int8_t initialize_hardware(uint16_t argc, char ** argv);

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
    const float na_default = 0.25;
    const uint8_t led_brightness_default = 255;
    const float inner_na_default = 0.0;
    const uint8_t cosine_factor_default = 0.0;

    // Internal variable for maximum na of this array, at this geometry.
    float max_na;
};

#endif

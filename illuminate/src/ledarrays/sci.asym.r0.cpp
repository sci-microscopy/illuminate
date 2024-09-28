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

// Define which LED Array is used
#include "../../illuminate.h"

#ifdef USE_SCI_ASYM_ARRAY
#include "../../ledarrayinterface.h"
#include "../TLC5955/TLC5955.h"

// Global shutter state
bool global_shutter_state = true;

// Pin definitions (used internally)
const int GSCLK = 6;
const int LAT = 3;
const int SPI_MOSI = 11;
const int SPI_CLK = 13;
const int TRIGGER_OUTPUT_PIN_0 = 22;
const int TRIGGER_INPUT_PIN_0 = 23;
const int TRIGGER_OUTPUT_PIN_1 = 19;
const int TRIGGER_INPUT_PIN_1 = 18;
const int TRIGGER_OUTPUT_COUNT = 2;
const int TRIGGER_INPUT_COUNT = 2;

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "sci.asym";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const int16_t LedArrayInterface::led_count = 48;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths_nm[] = {480.0, 525.0, 625.0};
const float LedArrayInterface::color_channel_fwhm_wavelengths_nm[] = {20.0, 20.0, 20.0};
const int LedArrayInterface::bit_depth = 16;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
int LedArrayInterface::debug = 0;
const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};
bool LedArrayInterface::trigger_input_state[] = {false, false};
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];

const uint8_t TLC5955::chip_count = 100;    // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 10.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc;                            // TLC5955 object

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 1;
const char * LedArrayInterface::device_commandNamesShort[] = {"c"};
const char * LedArrayInterface::device_commandNamesLong[] = {"center"};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {{1,5}}; // Number of commands, number of LEDs in each command.

PROGMEM const int16_t center_led_list[1][3] = {
  {0, 1, 2}
};

// Define LED positions in cartesian coordinates (LED#, channel, x * 100mm, y * 100mm, z * 100mm)
PROGMEM const int16_t LedArrayInterface::led_positions[48][5] = {
    {0, 7, -381, -225, 6000},
    {1, 30, 381, -225, 6000},
    {2, 37, 0, 450, 6000},
    {3, 9, -573, -557, 6000},
    {4, 8, -769, -217, 6000},
    {5, 36, -196, 775, 6000},
    {6, 38, 196, 775, 6000},
    {7, 27, 769, -217, 6000},
    {8, 17, 573, -557, 6000},
    {9, 14, -775, -1043, 6000},
    {10, 13, -1291, -150, 6000},
    {11, 22, 1125, -650, 6000},
    {12, 44, -515, 1193, 6000},
    {13, 35, 0, 1300, 6000},
    {14, 39, 515, 1193, 6000},
    {15, 20, 1291, -150, 6000},
    {16, 18, 775, -1043, 6000},
    {17, 15, -1125, -650, 6000},
    {18, 10, -1026, -1478, 6000},
    {19, 3, -1410, -1118, 6000},
    {20, 12, -1673, -662, 6000},
    {21, 6, -1793, -150, 6000},
    {22, 32, -766, 1628, 6000},
    {23, 33, -263, 1780, 6000},
    {24, 34, 263, 1780, 6000},
    {25, 45, 766, 1628, 6000},
    {26, 21, 1793, -150, 6000},
    {27, 16, 1673, -662, 6000},
    {28, 29, 1410, -1118, 6000},
    {29, 19, 1026, -1478, 6000},
    {30, 5, -1277, -1912, 6000},
    {31, 11, -1604, -1647, 6000},
    {32, 2, -1878, -1327, 6000},
    {33, 4, -2088, -962, 6000},
    {34, 1, -2229, -565, 6000},
    {35, 0, -2295, -150, 6000},
    {36, 46, -1017, 2062, 6000},
    {37, 40, -624, 2213, 6000},
    {38, 41, -210, 2290, 6000},
    {39, 42, 210, 2290, 6000},
    {40, 47, 624, 2213, 6000},
    {41, 43, 1017, 2062, 6000},
    {42, 28, 2295, -150, 6000},
    {43, 31, 2229, -565, 6000},
    {44, 25, 2088, -962, 6000},
    {45, 24, 1878, -1327, 6000},
    {46, 23, 1604, -1647, 6000},
    {47, 26, 1277, -1912, 6000}
};
bool LedArrayInterface::get_max_current_enforcement()
{
        return TLC5955::enforce_max_current;
}

float LedArrayInterface::get_max_current_limit()
{
       return TLC5955::max_current_amps;
}

void LedArrayInterface::set_max_current_enforcement(bool enforce)
{
        TLC5955::enforce_max_current = enforce;
}

void LedArrayInterface::set_max_current_limit(float limit)
{
        if (limit > 0)
                TLC5955::max_current_amps = limit;
}

void LedArrayInterface::set_pin_order(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
        tlc.set_pin_order_single(led_number, color_channel_index, position);
}

void LedArrayInterface::not_implemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
}

uint16_t LedArrayInterface::get_led_value(uint16_t led_number, int color_channel_index)
{
        int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
        if (channel_number >= 0)
                return tlc.get_single_channel(channel_number);
        else
        {
                Serial.print(F("ERROR (LedArrayInterface::get_led_value) - invalid LED number ("));
                Serial.print(led_number);
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
                return 0;
        }
}

// Debug Variables
bool LedArrayInterface::get_debug()
{
        return (LedArrayInterface::debug);
}

void LedArrayInterface::set_debug(int state)
{
        LedArrayInterface::debug = state;
        Serial.printf(F("(LedArrayInterface::set_debug): Set debug level to %d \n"), debug);
}

int LedArrayInterface::send_trigger_pulse(int trigger_index, uint16_t delay_us, bool inverse_polarity)
{
        // Get trigger pin
        int trigger_pin = trigger_output_pin_list[trigger_index];

        if (trigger_pin > 0)
        {
                // Write active state
                if (inverse_polarity)
                        digitalWriteFast(trigger_pin, LOW);
                else
                        digitalWriteFast(trigger_pin, HIGH);

                // Delay if desired
                if (delay_us > 0)
                        delayMicroseconds(delay_us);

                // Write normal state
                if (inverse_polarity)
                        digitalWriteFast(trigger_pin, HIGH);
                else
                        digitalWriteFast(trigger_pin, LOW);
                return (1);
        } else {
                return (-1);
        }
}

void LedArrayInterface::set_global_shutter_state(bool state)
{
    if (debug >= 1)
    {
        Serial.print("Setting Global Shutter state to ");
        Serial.print(state);
        Serial.print(SERIAL_LINE_ENDING);
    }

    // Store current state
    global_shutter_state = state;

    // Call the internal update method
    update();
}

bool LedArrayInterface::get_global_shutter_state()
{
    if (debug >= 1)
    {
        Serial.print("Getting Global Shutter state: ");
        Serial.print(global_shutter_state);
        Serial.print(SERIAL_LINE_ENDING);
    }
    
    return global_shutter_state;
}


void LedArrayInterface::update()
{
    if (global_shutter_state)
        tlc.update();
    else
        tlc.set_all(0);
}

void LedArrayInterface::clear()
{
        tlc.set_all(0);
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
    if (debug >= 2)
    {
        Serial.print(F("Drawing channel #"));
        Serial.print(channel_number);
        Serial.print(F(", color_channel #"));
        Serial.print(color_channel_number);
        Serial.print(F(" to value "));
        Serial.print(value);
        Serial.print(SERIAL_LINE_ENDING);
    }

    if (channel_number >= 0)
    {
        if (color_channel_number < 0)
            tlc.set_single(channel_number, value);
        else 
            tlc.set_single_rgb(channel_number, color_channel_number, value);
    }
    else
    {
            Serial.print(F("Error (LedArrayInterface::set_channel): Invalid channel ("));
            Serial.print(channel_number);
            Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
    }
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
{
    set_channel(channel_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, bool value)
{
    set_channel(channel_number, color_channel_number, (uint16_t) (value > 0 * UINT16_MAX));
}

void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
        if (debug >= 2)
        {
                Serial.print("U16 Setting led #");
                Serial.print(led_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(" to value ");
                Serial.print(value);
                Serial.print(SERIAL_LINE_ENDING);
        }
        if (led_number < 0)
        {
            for (uint16_t led_index = 0; led_index < led_count; led_index++)
            {
                    int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));
                    set_channel(channel_number, color_channel_number, value);
            }
        }
        else
        {
            int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
            set_channel(channel_number, color_channel_number, value);
        }
}



void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, uint8_t value)
{
        if (debug >= 2)
        {
                Serial.print("U8 Setting led #");
                Serial.print(led_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        set_led(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, bool value)
{
        if (debug >= 2)
        {
                Serial.print("B Setting led #");
                Serial.print(led_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        set_led(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

int8_t LedArrayInterface::device_reset()
{
  return device_setup();
}

int8_t LedArrayInterface::device_setup()
{
        // Initialize TLC5955
        tlc.init(LAT, SPI_MOSI, SPI_CLK, GSCLK);

        // We must set dot correction values, so set them all to the brightest adjustment
        tlc.set_all_dc_data(127);

        // Set Max Current Values (see TLC5955 datasheet)
        tlc.set_max_current(3, 3, 3); // Go up to 7

        // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
        // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT
        tlc.set_function_data(true, true, true, true, true);

        // Set all LED current levels to max (127)
        int currentR = 127;
        int currentB = 127;
        int currentG = 127;
        tlc.set_brightness_current(currentR, currentB, currentG);

        // Update vontrol register
        tlc.update_control();
        clear();
        tlc.update_control();

        // Set RGB pin order
        tlc.set_rgb_pin_order(0, 1, 2);

        // Update the GS register
        clear();

        // Output trigger Pins
        for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
        {
                pinMode(trigger_output_pin_list[trigger_index], OUTPUT);
                digitalWriteFast(trigger_output_pin_list[trigger_index], LOW);
        }

        // Input trigger pins
        attachInterrupt(digitalPinToInterrupt(trigger_input_pin_list[0]), trigger_pin_interrupt_0, CHANGE);
        attachInterrupt(digitalPinToInterrupt(trigger_input_pin_list[1]), trigger_pin_interrupt_1, CHANGE);
        
        return NO_ERROR;
}

void LedArrayInterface::trigger_pin_interrupt_0()
{
        bool previous_state = trigger_input_state[0];
        trigger_input_state[0] = digitalReadFast(trigger_input_pin_list[0]);
        bool new_state = trigger_input_state[0];
        if (debug >= 2)
            Serial.printf("Recieved trigger pulse on pin 0. Previous state: %s New state: %s%s", previous_state ? "HIGH" : "LOW", new_state ? "HIGH" : "LOW", SERIAL_LINE_ENDING);
}

void LedArrayInterface::trigger_pin_interrupt_1()
{
    bool previous_state = trigger_input_state[1];
    trigger_input_state[1] = digitalReadFast(trigger_input_pin_list[1]);
    bool new_state = trigger_input_state[1];
    if (debug >= 2)
        Serial.printf("Recieved trigger pulse on pin 1. Previous state: %s New state: %s%s", previous_state ? "HIGH" : "LOW", new_state ? "HIGH" : "LOW", SERIAL_LINE_ENDING);
}

void LedArrayInterface::source_change_interrupt()
{
        Serial.printf(F("ERROR (LedArrayInterface::source_change_interrupt): PSU Monitoring not supported on this device."), SERIAL_LINE_ENDING);
}

float LedArrayInterface::get_power_source_voltage()
{
    return -1.0;
}

bool LedArrayInterface::get_power_source_monitoring_state()
{
  return false;
}

int16_t LedArrayInterface::get_device_power_sensing_capability()
{
      return NO_PSU_SENSING;
}

void LedArrayInterface::set_power_source_monitoring_state(int new_state)
{
        Serial.printf(F("ERROR (LedArrayInterface::set_power_source_monitoring_state): PSU Monitoring not supported on this device."), SERIAL_LINE_ENDING);
}

bool LedArrayInterface::is_power_source_plugged_in()
{
  return true;
}

uint8_t LedArrayInterface::get_device_command_count()
{
  return (LedArrayInterface::device_command_count);
}

const char * LedArrayInterface::get_device_command_name_short(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::device_commandNamesShort[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

const char * LedArrayInterface::get_device_command_name_long(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::device_commandNamesLong[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

uint32_t LedArrayInterface::get_device_command_led_list_size(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
        {
                // Get stored pattern cound and led per pattern for this command
                uint16_t pattern_count = LedArrayInterface::device_command_pattern_dimensions[device_command_index][0];
                uint16_t leds_per_pattern = LedArrayInterface::device_command_pattern_dimensions[device_command_index][1];

                // Concatenate these two into 32-bit unsigned integer
                uint32_t concatenated = ((uint32_t)pattern_count) << 16 | leds_per_pattern;
                return (concatenated);
        }
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

uint16_t LedArrayInterface::get_device_command_led_list_element(int device_command_index, uint16_t pattern_index, uint16_t led_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
        {
                uint32_t concatenated = get_device_command_led_list_size(device_command_index);
                uint16_t pattern_count  = (uint16_t)(concatenated >> 16);
                uint16_t leds_per_pattern = (uint16_t)concatenated;

                if ((pattern_index < pattern_count) && (led_index < leds_per_pattern))
                {
                     if (device_command_index == 0)
                       return (uint16_t)pgm_read_word(&(center_led_list[pattern_index][led_index]));
                     else
                       return 0;

                }
                else
                {
                        Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid pattern index (%d) / led index (%d)"), pattern_index, led_index, SERIAL_LINE_ENDING);
                        return (0);
                }
        }
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

void LedArrayInterface::set_gsclk_frequency(uint32_t gsclk_frequency)
{
  tlc.set_gsclk_frequency(gsclk_frequency);
}

uint32_t LedArrayInterface::get_gsclk_frequency()
{
  return tlc.get_gsclk_frequency();
}

void LedArrayInterface::set_sclk_baud_rate(uint32_t new_baud_rate)
{
    tlc.set_sclk_frequency(new_baud_rate);
}

uint32_t LedArrayInterface::get_sclk_baud_rate()
{
  return tlc.get_sclk_frequency();
}

int8_t LedArrayInterface::set_register(uint32_t address, int8_t value)
{
    EEPROM.write(address, value);
    return NO_ERROR;
}

int8_t LedArrayInterface::get_register(uint32_t address)
{
    return EEPROM.read(address);
}


#endif

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

#ifdef USE_SCI_ROUND_ARRAY
#include "../../ledarrayinterface.h"
#include "../TLC5955/TLC5955.h"
#include <EEPROM.h>

// Pin definitions (used internally)
const int GSCLK = 6;
const int LAT = 3;
const int SPI_MOSI = 11;
const int SPI_CLK = 13;
const int TRIGGER_OUTPUT_PIN_0 = 22;
const int TRIGGER_INPUT_PIN_0 = 21;
const int TRIGGER_OUTPUT_COUNT = 1;
const int TRIGGER_INPUT_COUNT = 1;

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "Sci-Round";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 0.65;
const float LedArrayInterface::default_na = 0.4;
const int16_t LedArrayInterface::led_count = 257;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 1;
const int LedArrayInterface::trigger_input_count = 1;
const int LedArrayInterface::color_channel_count = 1;
const char LedArrayInterface::color_channel_names[] = {'g'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.53};
const int LedArrayInterface::bit_depth = 16;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];

const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0};
bool LedArrayInterface::trigger_input_state[] = {false};

int LedArrayInterface::debug = 0;

const uint8_t TLC5955::chip_count = 52;          // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 8.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc; // TLC5955 object
uint32_t gsclk_frequency = 1000000;

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 1;
const char * LedArrayInterface::device_commandNamesShort[] = {"c"};
const char * LedArrayInterface::device_commandNamesLong[] = {"center"};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {{1,5}}; // Number of commands, number of LEDs in each command.

/**** Part number and Serial number addresses in EEPROM ****/
uint16_t pn_address = 100;
uint16_t sn_address = 200;

PROGMEM const int16_t center_led_list[1][5] = {
  {0, 1, 2, 3, 4}
};

// Initialize LED positions
const int16_t PROGMEM LedArrayInterface::led_positions[][5] = {
        {0, 1, 0, 0, 0},
        {1, 96, 0, -620, 0},
        {2, 113, -438, -438, 0},
        {3, 48, -620, 0, 0},
        {4, 65, -438, 438, 0},
        {5, 0, 0, 620, 0},
        {6, 17, 438, 438, 0},
        {7, 144, 620, 0, 0},
        {8, 161, 438, -438, 0},
        {9, 109, 0, -1240, 0},
        {10, 98, -475, -1146, 0},
        {11, 111, -877, -877, 0},
        {12, 101, -1146, -475, 0},
        {13, 61, -1240, 0, 0},
        {14, 50, -1146, 475, 0},
        {15, 63, -877, 877, 0},
        {16, 53, -475, 1146, 0},
        {17, 13, 0, 1240, 0},
        {18, 2, 475, 1146, 0},
        {19, 15, 877, 877, 0},
        {20, 5, 1146, 475, 0},
        {21, 157, 1240, 0, 0},
        {22, 146, 1146, -475, 0},
        {23, 159, 877, -877, 0},
        {24, 149, 475, -1146, 0},
        {25, 122, 0, -1860, 0},
        {26, 110, -481, -1797, 0},
        {27, 108, -930, -1611, 0},
        {28, 112, -1315, -1315, 0},
        {29, 99, -1611, -930, 0},
        {30, 100, -1797, -481, 0},
        {31, 74, -1860, 0, 0},
        {32, 62, -1797, 481, 0},
        {33, 60, -1611, 930, 0},
        {34, 64, -1315, 1315, 0},
        {35, 51, -930, 1611, 0},
        {36, 52, -481, 1797, 0},
        {37, 26, 0, 1860, 0},
        {38, 14, 481, 1797, 0},
        {39, 12, 930, 1611, 0},
        {40, 16, 1315, 1315, 0},
        {41, 3, 1611, 930, 0},
        {42, 4, 1797, 481, 0},
        {43, 170, 1860, 0, 0},
        {44, 158, 1797, -481, 0},
        {45, 156, 1611, -930, 0},
        {46, 160, 1315, -1315, 0},
        {47, 147, 930, -1611, 0},
        {48, 148, 481, -1797, 0},
        {49, 121, 0, -2480, 0},
        {50, 134, -431, -2442, 0},
        {51, 133, -848, -2330, 0},
        {52, 132, -1240, -2148, 0},
        {53, 120, -1594, -1900, 0},
        {54, 104, -1900, -1594, 0},
        {55, 103, -2148, -1240, 0},
        {56, 116, -2330, -848, 0},
        {57, 102, -2442, -431, 0},
        {58, 73, -2480, 0, 0},
        {59, 86, -2442, 431, 0},
        {60, 85, -2330, 848, 0},
        {61, 84, -2148, 1240, 0},
        {62, 72, -1900, 1594, 0},
        {63, 56, -1594, 1900, 0},
        {64, 55, -1240, 2148, 0},
        {65, 68, -848, 2330, 0},
        {66, 54, -431, 2442, 0},
        {67, 25, 0, 2480, 0},
        {68, 38, 431, 2442, 0},
        {69, 37, 848, 2330, 0},
        {70, 36, 1240, 2148, 0},
        {71, 24, 1594, 1900, 0},
        {72, 8, 1900, 1594, 0},
        {73, 7, 2148, 1240, 0},
        {74, 20, 2330, 848, 0},
        {75, 6, 2442, 431, 0},
        {76, 169, 2480, 0, 0},
        {77, 182, 2442, -431, 0},
        {78, 181, 2330, -848, 0},
        {79, 180, 2148, -1240, 0},
        {80, 168, 1900, -1594, 0},
        {81, 152, 1594, -1900, 0},
        {82, 151, 1240, -2148, 0},
        {83, 164, 848, -2330, 0},
        {84, 150, 431, -2442, 0},
        {85, 125, 0, -3100, 0},
        {86, 123, -405, -3073, 0},
        {87, 137, -802, -2994, 0},
        {88, 136, -1186, -2864, 0},
        {89, 138, -1550, -2685, 0},
        {90, 126, -1887, -2459, 0},
        {91, 142, -2192, -2192, 0},
        {92, 130, -2459, -1887, 0},
        {93, 119, -2685, -1550, 0},
        {94, 105, -2864, -1186, 0},
        {95, 115, -2994, -802, 0},
        {96, 114, -3073, -405, 0},
        {97, 77, -3100, 0, 0},
        {98, 75, -3073, 405, 0},
        {99, 89, -2994, 802, 0},
        {100, 88, -2864, 1186, 0},
        {101, 90, -2685, 1550, 0},
        {102, 78, -2459, 1887, 0},
        {103, 94, -2192, 2192, 0},
        {104, 82, -1887, 2459, 0},
        {105, 71, -1550, 2685, 0},
        {106, 57, -1186, 2864, 0},
        {107, 67, -802, 2994, 0},
        {108, 66, -405, 3073, 0},
        {109, 29, 0, 3100, 0},
        {110, 27, 405, 3073, 0},
        {111, 41, 802, 2994, 0},
        {112, 40, 1186, 2864, 0},
        {113, 42, 1550, 2685, 0},
        {114, 30, 1887, 2459, 0},
        {115, 46, 2192, 2192, 0},
        {116, 34, 2459, 1887, 0},
        {117, 23, 2685, 1550, 0},
        {118, 9, 2864, 1186, 0},
        {119, 19, 2994, 802, 0},
        {120, 18, 3073, 405, 0},
        {121, 173, 3100, 0, 0},
        {122, 171, 3073, -405, 0},
        {123, 185, 2994, -802, 0},
        {124, 184, 2864, -1186, 0},
        {125, 186, 2685, -1550, 0},
        {126, 174, 2459, -1887, 0},
        {127, 190, 2192, -2192, 0},
        {128, 178, 1887, -2459, 0},
        {129, 167, 1550, -2685, 0},
        {130, 153, 1186, -2864, 0},
        {131, 163, 802, -2994, 0},
        {132, 162, 405, -3073, 0},
        {133, 124, 0, -3720, 0},
        {134, 135, -417, -3697, 0},
        {135, 140, -828, -3627, 0},
        {136, 139, -1229, -3511, 0},
        {137, 128, -1614, -3352, 0},
        {138, 127, -1979, -3150, 0},
        {139, 143, -2319, -2908, 0},
        {140, 141, -2630, -2630, 0},
        {141, 131, -2908, -2319, 0},
        {142, 129, -3150, -1979, 0},
        {143, 118, -3352, -1614, 0},
        {144, 117, -3511, -1229, 0},
        {145, 106, -3627, -828, 0},
        {146, 107, -3697, -417, 0},
        {147, 76, -3720, 0, 0},
        {148, 87, -3697, 417, 0},
        {149, 92, -3627, 828, 0},
        {150, 91, -3511, 1229, 0},
        {151, 80, -3352, 1614, 0},
        {152, 79, -3150, 1979, 0},
        {153, 95, -2908, 2319, 0},
        {154, 93, -2630, 2630, 0},
        {155, 83, -2319, 2908, 0},
        {156, 81, -1979, 3150, 0},
        {157, 70, -1614, 3352, 0},
        {158, 69, -1229, 3511, 0},
        {159, 58, -828, 3627, 0},
        {160, 59, -417, 3697, 0},
        {161, 28, 0, 3720, 0},
        {162, 39, 417, 3697, 0},
        {163, 44, 828, 3627, 0},
        {164, 43, 1229, 3511, 0},
        {165, 32, 1614, 3352, 0},
        {166, 31, 1979, 3150, 0},
        {167, 47, 2319, 2908, 0},
        {168, 45, 2630, 2630, 0},
        {169, 35, 2908, 2319, 0},
        {170, 33, 3150, 1979, 0},
        {171, 22, 3352, 1614, 0},
        {172, 21, 3511, 1229, 0},
        {173, 10, 3627, 828, 0},
        {174, 11, 3697, 417, 0},
        {175, 172, 3720, 0, 0},
        {176, 183, 3697, -417, 0},
        {177, 188, 3627, -828, 0},
        {178, 187, 3511, -1229, 0},
        {179, 176, 3352, -1614, 0},
        {180, 175, 3150, -1979, 0},
        {181, 191, 2908, -2319, 0},
        {182, 189, 2630, -2630, 0},
        {183, 179, 2319, -2908, 0},
        {184, 177, 1979, -3150, 0},
        {185, 166, 1614, -3352, 0},
        {186, 165, 1229, -3511, 0},
        {187, 154, 828, -3627, 0},
        {188, 155, 417, -3697, 0},
        {189, 97, 0, -4340, 0},
        {190, 231, -400, -4321, 0},
        {191, 232, -797, -4266, 0},
        {192, 214, -1188, -4174, 0},
        {193, 213, -1568, -4047, 0},
        {194, 215, -1935, -3885, 0},
        {195, 202, -2285, -3690, 0},
        {196, 201, -2615, -3463, 0},
        {197, 203, -2924, -3207, 0},
        {198, 211, -3207, -2924, 0},
        {199, 210, -3463, -2615, 0},
        {200, 212, -3690, -2285, 0},
        {201, 196, -3885, -1935, 0},
        {202, 195, -4047, -1568, 0},
        {203, 197, -4174, -1188, 0},
        {204, 208, -4266, -797, 0},
        {205, 207, -4321, -400, 0},
        {206, 209, -4340, 0, 0},
        {207, 193, -4321, 400, 0},
        {208, 192, -4266, 797, 0},
        {209, 194, -4174, 1188, 0},
        {210, 205, -4047, 1568, 0},
        {211, 204, -3885, 1935, 0},
        {212, 206, -3690, 2285, 0},
        {213, 218, -3463, 2615, 0},
        {214, 216, -3207, 2924, 0},
        {215, 217, -2924, 3207, 0},
        {216, 230, -2615, 3463, 0},
        {217, 228, -2285, 3690, 0},
        {218, 229, -1935, 3885, 0},
        {219, 221, -1568, 4047, 0},
        {220, 219, -1188, 4174, 0},
        {221, 220, -797, 4266, 0},
        {222, 233, -400, 4321, 0},
        {223, 49, 0, 4340, 0},
        {224, 252, 400, 4321, 0},
        {225, 254, 797, 4266, 0},
        {226, 266, 1188, 4174, 0},
        {227, 264, 1568, 4047, 0},
        {228, 265, 1935, 3885, 0},
        {229, 278, 2285, 3690, 0},
        {230, 276, 2615, 3463, 0},
        {231, 277, 2924, 3207, 0},
        {232, 269, 3207, 2924, 0},
        {233, 267, 3463, 2615, 0},
        {234, 268, 3690, 2285, 0},
        {235, 281, 3885, 1935, 0},
        {236, 279, 4047, 1568, 0},
        {237, 280, 4174, 1188, 0},
        {238, 284, 4266, 797, 0},
        {239, 282, 4321, 400, 0},
        {240, 283, 4340, 0, 0},
        {241, 272, 4321, -400, 0},
        {242, 270, 4266, -797, 0},
        {243, 271, 4174, -1188, 0},
        {244, 287, 4047, -1568, 0},
        {245, 285, 3885, -1935, 0},
        {246, 244, 3690, -2285, 0},
        {247, 243, 3463, -2615, 0},
        {248, 245, 3207, -2924, 0},
        {249, 256, 2924, -3207, 0},
        {250, 255, 2615, -3463, 0},
        {251, 257, 2285, -3690, 0},
        {252, 241, 1935, -3885, 0},
        {253, 240, 1568, -4047, 0},
        {254, 242, 1188, -4174, 0},
        {255, 253, 797, -4266, 0},
        {256, 145, 400, -4321, 0}
};

void LedArrayInterface::set_max_current_enforcement(bool enforce)
{
        TLC5955::enforce_max_current = enforce;
}

void LedArrayInterface::set_max_current_limit(float limit)
{
        if (limit > 0)
                TLC5955::max_current_amps = limit;
}

void LedArrayInterface::not_implemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device. %s"), SERIAL_LINE_ENDING);
}

uint16_t LedArrayInterface::get_led_value(uint16_t led_number, int color_channel_index)
{
        int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
        if (channel_number >= 0)
                return tlc.getChannelValue(channel_number, color_channel_index);
        else
        {
                Serial.print(F("ERROR (LedArrayInterface::get_led_value) - invalid LED number ("));
                Serial.print(led_number);
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
                return 0;
        }
}

void LedArrayInterface::set_pin_order(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
        not_implemented("set_pin_order");
}

void LedArrayInterface::set_ledFast(int16_t led_number, int color_channel_index, bool value)
{
        not_implemented("set_ledFast");
}

// Debug Variables
bool LedArrayInterface::get_debug()
{
        return (LedArrayInterface::debug);
}

void LedArrayInterface::set_debug(int state)
{
        debug = state;
        Serial.printf(F("(LedArrayInterface::set_debug): Set debug level to %d \n"), debug);
}

int LedArrayInterface::set_trigger_state(int trigger_index, bool state)
{
        // Get trigger pin
        int trigger_pin = trigger_output_pin_list[trigger_index];
        if (trigger_pin > 0)
        {
                if (state)
                        digitalWriteFast(trigger_pin, HIGH);
                else
                        digitalWriteFast(trigger_pin, LOW);
                return (1);
        } else {
                return (-1);
        }
}

int LedArrayInterface::get_input_trigger_state(int input_trigger_index)
{
        // Get trigger pin
        int trigger_pin = trigger_input_pin_list[input_trigger_index];
        if (trigger_pin > 0)
                return (LedArrayInterface::trigger_input_state[trigger_pin]);
        else
                return (-1);
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
void LedArrayInterface::update()
{
        tlc.updateLeds();
}

void LedArrayInterface::clear()
{
        tlc.setAllLed(0);
        tlc.updateLeds();
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
        if (debug >= 2)
        {
                Serial.print(F("U16 Setting channel #"));
                Serial.print(channel_number);
                Serial.print(F(" to value "));
                Serial.print(value);
                Serial.print(SERIAL_LINE_ENDING);
        }

        if (channel_number >= 0)
                tlc.set_channel(channel_number, value);
        else
        {
                Serial.print(F("Error (LedArrayInterface::set_channel): Invalid channel ("));
                Serial.print(channel_number);
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
        }
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
{
        if (debug >= 2)
        {
                Serial.print("U8 Setting channel #");
                Serial.print(channel_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        set_channel(channel_number, color_channel_number, (uint16_t)( value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, bool value)
{
        if (debug >= 2)
        {
                Serial.print("U8 Setting channel #");
                Serial.print(channel_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        set_channel(channel_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
        int16_t channel_number = -1;

        if (led_number < 0)
        {
                for (uint16_t led_index = 0; led_index < led_count; led_index++)
                {
                        channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));
                        set_channel(channel_number, color_channel_number, value);
                }
        }
        else
        {
                channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
                set_channel(channel_number, color_channel_number, value);
        }

        if (debug >= 2)
        {
                Serial.print("U16 Set led #");
                Serial.print(led_number);
                Serial.print(", channel #");
                Serial.print(channel_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(" to value ");
                Serial.print(value);
                Serial.print(SERIAL_LINE_ENDING);
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

void LedArrayInterface::device_reset()
{
        device_setup();
}

uint16_t LedArrayInterface::get_serial_number()
{
  uint16_t sn_read = (EEPROM.read(SN_ADDRESS + 1) << 8) | EEPROM.read(SN_ADDRESS);
  return (sn_read);
}

void LedArrayInterface::setSerialNumber(uint16_t serial_number)
{
	byte lower_8bits_sn = serial_number & 0xff;
	byte upper_8bits_sn = (serial_number >> 8) & 0xff;
	EEPROM.write(SN_ADDRESS, lower_8bits_sn);
	EEPROM.write(SN_ADDRESS + 1, upper_8bits_sn);
}

uint16_t LedArrayInterface::get_part_number()
{
  uint16_t pn_read = (EEPROM.read(PN_ADDRESS + 1) << 8) | EEPROM.read(PN_ADDRESS);
  return (pn_read);
}

void LedArrayInterface::set_part_number(uint16_t part_number)
{
	byte lower_8bits_pn = part_number & 0xff;
	byte upper_8bits_pn = (part_number >> 8) & 0xff;
	EEPROM.write(PN_ADDRESS, lower_8bits_pn);
	EEPROM.write(PN_ADDRESS + 1, upper_8bits_pn);
}

void LedArrayInterface::device_setup()
{
    // Initialize TLC5955
    tlc.init(LAT, SPI_MOSI, SPI_CLK, GSCLK);

    // We must set dot correction values, so set them all to the brightest adjustment
    tlc.set_all_dc_data(127);

    // Set Max Current Values (see TLC5955 datasheet)
    tlc.set_max_current(3, 3, 3); // Go up to 7

    // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
    // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT
    tlc.set_function_data(true, true, false, true, true); // WORKS with fast update

    // set all brightness levels to max (127)
    int currentR = 127;
    int currentB = 127;
    int currentG = 127;
    tlc.set_brightness_current(currentR, currentB, currentG);

    // Update Control Register
    tlc.update_control();

    // Update the GS register (ideally LEDs should be dark up to here)
    tlc.setAllLed(0);
    tlc.updateLeds();

    // Output trigger Pins
    for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
    {
            pinMode(LedArrayInterface::trigger_output_pin_list[trigger_index], OUTPUT);
            digitalWriteFast(LedArrayInterface::trigger_output_pin_list[trigger_index], LOW);
    }

    // Input trigger Pins
    for (int trigger_index = 0; trigger_index < trigger_input_count; trigger_index++)
            pinMode(LedArrayInterface::trigger_input_pin_list[trigger_index], INPUT);

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

void LedArrayInterface::set_power_source_monitoring_state(bool new_state)
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
                        //      if (device_command_index == 0)
                        //        return (uint16_t)pgm_read_word(&(hole_led_list[pattern_index][led_index]));
                        //      else if (device_command_index == 1)
                        //        return (uint16_t)pgm_read_word(&(uv_led_list[pattern_index][led_index]));
                        //      else
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

int8_t LedArrayInterface::getDemoMode()
{
  int8_t demo_mode_read = EEPROM.read(DEMO_MODE_ADDRESS);
  return (demo_mode_read);
}

void LedArrayInterface::setDemoMode(int8_t demo_mode)
{
	EEPROM.write(DEMO_MODE_ADDRESS, demo_mode);
}

#endif

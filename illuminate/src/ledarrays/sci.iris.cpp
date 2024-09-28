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

#ifdef USE_SCI_IRIS

#include "../../ledarrayinterface.h"
#include "../../constants.h"
#include "../TLC5955/TLC5955.h"
#include <EEPROM.h>

// Power monitoring commands
#define DEVICE_SUPPORTS_POWER_SENSING 0
#define DEVICE_SUPPORTS_ACTIVE_POWER_MONITORING 0
#define PSU_ACTIVE_MONITORING_COMPARATOR_MODE 5

#if DEVICE_SUPPORTS_ACTIVE_POWER_MONITORING
#include "../TeensyComparator/TeensyComparator.h"
#endif

// Power sensing pin
const int POWER_SENSE_PIN = 23;

// Pin definitions (used internally)
const int GSCLK = 6;
const int LAT = 3;
const int SPI_MOSI = 11;
const int SPI_CLK = 13;
const int TRIGGER_OUTPUT_PIN_0 = 22;
const int TRIGGER_INPUT_PIN_0 = 21;
const int TRIGGER_OUTPUT_COUNT = 1;
const int TRIGGER_INPUT_COUNT = 1;
const int COLOR_CHANNEL_COUNT = 1;
const int LEDS_PER_CHIP = 48;

// Power source sensing variables
bool _psu_is_connected = true;
bool _power_source_sensing_is_enabled = false;
elapsedMillis _time_elapsed_debounce;
uint32_t _warning_delay_ms = 10;
bool global_shutter_state = true;

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "sci.iris";
const char * LedArrayInterface::device_hardware_revision = "r0";
const int16_t LedArrayInterface::led_count = 256;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 1;
const int LedArrayInterface::trigger_input_count = 1;
const int LedArrayInterface::color_channel_count = 1;
const char LedArrayInterface::color_channel_names[] = {'r'};
const float LedArrayInterface::color_channel_center_wavelengths_nm[] = {740.0};
const float LedArrayInterface::color_channel_fwhm_wavelengths_nm[] = {20.0};
const int LedArrayInterface::bit_depth = 8;
const bool LedArrayInterface::supports_fast_sequence = false;
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];

const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0};
bool LedArrayInterface::trigger_input_state[] = {false};

int LedArrayInterface::debug = 0;

const uint8_t TLC5955::chip_count = 6;          // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 8.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::chip_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc; // TLC5955 object

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 9;
const char * LedArrayInterface::device_commandNamesShort[] = {};
const char * LedArrayInterface::device_commandNamesLong[] = {};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {{}}; // Number of commands, number of LEDs in each command.

/**** Part number and Serial number addresses in EEPROM ****/
uint16_t pn_address = 100;
uint16_t sn_address = 200;

// Initialize LED positions
const int16_t PROGMEM LedArrayInterface::led_positions[][5] = {
       {0, 96, 0, -800, 0},
{1, 113, -570, -570, 0},
{2, 48, -800, 0, 0},
{3, 65, -570, 570, 0},
{4, 0, 0, 800, 0},
{5, 17, 570, 570, 0},
{6, 144, 800, 0, 0},
{7, 161, 570, -570, 0},
{8, 109, 0, -1240, 0},
{9, 98, -475, -1146, 0},
{10, 111, -877, -877, 0},
{11, 101, -1146, -475, 0},
{12, 61, -1240, 0, 0},
{13, 50, -1146, 475, 0},
{14, 63, -877, 877, 0},
{15, 53, -475, 1146, 0},
{16, 13, 0, 1240, 0},
{17, 2, 475, 1146, 0},
{18, 15, 877, 877, 0},
{19, 5, 1146, 475, 0},
{20, 157, 1240, 0, 0},
{21, 146, 1146, -475, 0},
{22, 159, 877, -877, 0},
{23, 149, 475, -1146, 0},
{24, 122, 0, -1860, 0},
{25, 110, -481, -1797, 0},
{26, 108, -930, -1611, 0},
{27, 112, -1315, -1315, 0},
{28, 99, -1611, -930, 0},
{29, 100, -1797, -481, 0},
{30, 74, -1860, 0, 0},
{31, 62, -1797, 481, 0},
{32, 60, -1611, 930, 0},
{33, 64, -1315, 1315, 0},
{34, 51, -930, 1611, 0},
{35, 52, -481, 1797, 0},
{36, 26, 0, 1860, 0},
{37, 14, 481, 1797, 0},
{38, 12, 930, 1611, 0},
{39, 16, 1315, 1315, 0},
{40, 3, 1611, 930, 0},
{41, 4, 1797, 481, 0},
{42, 170, 1860, 0, 0},
{43, 158, 1797, -481, 0},
{44, 156, 1611, -930, 0},
{45, 160, 1315, -1315, 0},
{46, 147, 930, -1611, 0},
{47, 148, 481, -1797, 0},
{48, 121, 0, -2480, 0},
{49, 134, -431, -2442, 0},
{50, 133, -848, -2330, 0},
{51, 132, -1240, -2148, 0},
{52, 120, -1594, -1900, 0},
{53, 104, -1900, -1594, 0},
{54, 103, -2148, -1240, 0},
{55, 116, -2330, -848, 0},
{56, 102, -2442, -431, 0},
{57, 73, -2480, 0, 0},
{58, 86, -2442, 431, 0},
{59, 85, -2330, 848, 0},
{60, 84, -2148, 1240, 0},
{61, 72, -1900, 1594, 0},
{62, 56, -1594, 1900, 0},
{63, 55, -1240, 2148, 0},
{64, 68, -848, 2330, 0},
{65, 54, -431, 2442, 0},
{66, 25, 0, 2480, 0},
{67, 38, 431, 2442, 0},
{68, 37, 848, 2330, 0},
{69, 36, 1240, 2148, 0},
{70, 24, 1594, 1900, 0},
{71, 8, 1900, 1594, 0},
{72, 7, 2148, 1240, 0},
{73, 20, 2330, 848, 0},
{74, 6, 2442, 431, 0},
{75, 169, 2480, 0, 0},
{76, 182, 2442, -431, 0},
{77, 181, 2330, -848, 0},
{78, 180, 2148, -1240, 0},
{79, 168, 1900, -1594, 0},
{80, 152, 1594, -1900, 0},
{81, 151, 1240, -2148, 0},
{82, 164, 848, -2330, 0},
{83, 150, 431, -2442, 0},
{84, 125, 0, -3100, 0},
{85, 123, -405, -3073, 0},
{86, 137, -802, -2994, 0},
{87, 136, -1186, -2864, 0},
{88, 138, -1550, -2685, 0},
{89, 126, -1887, -2459, 0},
{90, 142, -2192, -2192, 0},
{91, 130, -2459, -1887, 0},
{92, 119, -2685, -1550, 0},
{93, 105, -2864, -1186, 0},
{94, 115, -2994, -802, 0},
{95, 114, -3073, -405, 0},
{96, 77, -3100, 0, 0},
{97, 75, -3073, 405, 0},
{98, 89, -2994, 802, 0},
{99, 88, -2864, 1186, 0},
{100, 90, -2685, 1550, 0},
{101, 78, -2459, 1887, 0},
{102, 94, -2192, 2192, 0},
{103, 82, -1887, 2459, 0},
{104, 71, -1550, 2685, 0},
{105, 57, -1186, 2864, 0},
{106, 67, -802, 2994, 0},
{107, 66, -405, 3073, 0},
{108, 29, 0, 3100, 0},
{109, 27, 405, 3073, 0},
{110, 41, 802, 2994, 0},
{111, 40, 1186, 2864, 0},
{112, 42, 1550, 2685, 0},
{113, 30, 1887, 2459, 0},
{114, 46, 2192, 2192, 0},
{115, 34, 2459, 1887, 0},
{116, 23, 2685, 1550, 0},
{117, 9, 2864, 1186, 0},
{118, 19, 2994, 802, 0},
{119, 18, 3073, 405, 0},
{120, 173, 3100, 0, 0},
{121, 171, 3073, -405, 0},
{122, 185, 2994, -802, 0},
{123, 184, 2864, -1186, 0},
{124, 186, 2685, -1550, 0},
{125, 174, 2459, -1887, 0},
{126, 190, 2192, -2192, 0},
{127, 178, 1887, -2459, 0},
{128, 167, 1550, -2685, 0},
{129, 153, 1186, -2864, 0},
{130, 163, 802, -2994, 0},
{131, 162, 405, -3073, 0},
{132, 124, 0, -3720, 0},
{133, 135, -417, -3697, 0},
{134, 140, -828, -3627, 0},
{135, 139, -1229, -3511, 0},
{136, 128, -1614, -3352, 0},
{137, 127, -1979, -3150, 0},
{138, 143, -2319, -2908, 0},
{139, 141, -2630, -2630, 0},
{140, 131, -2908, -2319, 0},
{141, 129, -3150, -1979, 0},
{142, 118, -3352, -1614, 0},
{143, 117, -3511, -1229, 0},
{144, 106, -3627, -828, 0},
{145, 107, -3697, -417, 0},
{146, 76, -3720, 0, 0},
{147, 87, -3697, 417, 0},
{148, 92, -3627, 828, 0},
{149, 91, -3511, 1229, 0},
{150, 80, -3352, 1614, 0},
{151, 79, -3150, 1979, 0},
{152, 95, -2908, 2319, 0},
{153, 93, -2630, 2630, 0},
{154, 83, -2319, 2908, 0},
{155, 81, -1979, 3150, 0},
{156, 70, -1614, 3352, 0},
{157, 69, -1229, 3511, 0},
{158, 58, -828, 3627, 0},
{159, 59, -417, 3697, 0},
{160, 28, 0, 3720, 0},
{161, 39, 417, 3697, 0},
{162, 44, 828, 3627, 0},
{163, 43, 1229, 3511, 0},
{164, 32, 1614, 3352, 0},
{165, 31, 1979, 3150, 0},
{166, 47, 2319, 2908, 0},
{167, 45, 2630, 2630, 0},
{168, 35, 2908, 2319, 0},
{169, 33, 3150, 1979, 0},
{170, 22, 3352, 1614, 0},
{171, 21, 3511, 1229, 0},
{172, 10, 3627, 828, 0},
{173, 11, 3697, 417, 0},
{174, 172, 3720, 0, 0},
{175, 183, 3697, -417, 0},
{176, 188, 3627, -828, 0},
{177, 187, 3511, -1229, 0},
{178, 176, 3352, -1614, 0},
{179, 175, 3150, -1979, 0},
{180, 191, 2908, -2319, 0},
{181, 189, 2630, -2630, 0},
{182, 179, 2319, -2908, 0},
{183, 177, 1979, -3150, 0},
{184, 166, 1614, -3352, 0},
{185, 165, 1229, -3511, 0},
{186, 154, 828, -3627, 0},
{187, 155, 417, -3697, 0},
{188, 97, 0, -4340, 0},
{189, 231, -400, -4321, 0},
{190, 232, -797, -4266, 0},
{191, 214, -1188, -4174, 0},
{192, 213, -1568, -4047, 0},
{193, 215, -1935, -3885, 0},
{194, 202, -2285, -3690, 0},
{195, 201, -2615, -3463, 0},
{196, 203, -2924, -3207, 0},
{197, 211, -3207, -2924, 0},
{198, 210, -3463, -2615, 0},
{199, 212, -3690, -2285, 0},
{200, 196, -3885, -1935, 0},
{201, 195, -4047, -1568, 0},
{202, 197, -4174, -1188, 0},
{203, 208, -4266, -797, 0},
{204, 207, -4321, -400, 0},
{205, 209, -4340, 0, 0},
{206, 193, -4321, 400, 0},
{207, 192, -4266, 797, 0},
{208, 194, -4174, 1188, 0},
{209, 205, -4047, 1568, 0},
{210, 204, -3885, 1935, 0},
{211, 206, -3690, 2285, 0},
{212, 218, -3463, 2615, 0},
{213, 216, -3207, 2924, 0},
{214, 217, -2924, 3207, 0},
{215, 230, -2615, 3463, 0},
{216, 228, -2285, 3690, 0},
{217, 229, -1935, 3885, 0},
{218, 221, -1568, 4047, 0},
{219, 219, -1188, 4174, 0},
{220, 220, -797, 4266, 0},
{221, 233, -400, 4321, 0},
{222, 49, 0, 4340, 0},
{223, 252, 400, 4321, 0},
{224, 254, 797, 4266, 0},
{225, 266, 1188, 4174, 0},
{226, 264, 1568, 4047, 0},
{227, 265, 1935, 3885, 0},
{228, 278, 2285, 3690, 0},
{229, 276, 2615, 3463, 0},
{230, 277, 2924, 3207, 0},
{231, 269, 3207, 2924, 0},
{232, 267, 3463, 2615, 0},
{233, 268, 3690, 2285, 0},
{234, 281, 3885, 1935, 0},
{235, 279, 4047, 1568, 0},
{236, 280, 4174, 1188, 0},
{237, 284, 4266, 797, 0},
{238, 282, 4321, 400, 0},
{239, 283, 4340, 0, 0},
{240, 272, 4321, -400, 0},
{241, 270, 4266, -797, 0},
{242, 271, 4174, -1188, 0},
{243, 287, 4047, -1568, 0},
{244, 285, 3885, -1935, 0},
{245, 244, 3690, -2285, 0},
{246, 243, 3463, -2615, 0},
{247, 245, 3207, -2924, 0},
{248, 256, 2924, -3207, 0},
{249, 255, 2615, -3463, 0},
{250, 257, 2285, -3690, 0},
{251, 241, 1935, -3885, 0},
{252, 240, 1568, -4047, 0},
{253, 242, 1188, -4174, 0},
{254, 253, 797, -4266, 0},
{255, 145, 400, -4321, 0},
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
        return tlc.get_single_channel(channel_number);
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
        return 1;
    }
    else
        return -1;
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

bool LedArrayInterface::get_max_current_enforcement()
{
        return TLC5955::enforce_max_current;
}

float LedArrayInterface::get_max_current_limit()
{
       return TLC5955::max_current_amps;
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
        tlc.set_single_channel(channel_number, value);
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


int8_t LedArrayInterface::device_reset()
{
        return device_setup();
}

int8_t LedArrayInterface::device_setup()
{   
    pinMode(LAT, OUTPUT);
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
    tlc.set_all(0);
    tlc.update();

    // Output trigger Pins
    for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
    {
        pinMode(LedArrayInterface::trigger_output_pin_list[trigger_index], OUTPUT);
        digitalWriteFast(LedArrayInterface::trigger_output_pin_list[trigger_index], LOW);
    }


        // Output trigger pins
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
    Serial.printf(F("ERROR (LedArrayInterface::trigger_pin_interrupt_1): This board has only one trigger pin.%s"), SERIAL_LINE_ENDING);
}

void LedArrayInterface::source_change_interrupt()
{
    Serial.printf(F("ERROR (LedArrayInterface::source_change_interrupt): PSU Monitoring not supported on this device.%s"), SERIAL_LINE_ENDING);
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
        return 0;
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
            return 0;
        }
    }
    else
    {
        Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
        return 0;
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

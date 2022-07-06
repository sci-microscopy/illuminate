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

#ifdef USE_QUAD_LULED_ARRAY
#include "../../ledarrayinterface.h"
#include <FastLED.h>

// Power monitoring commands
#define DEVICE_SUPPORTS_POWER_SENSING 0
#define DEVICE_SUPPORTS_ACTIVE_POWER_MONITORING 0
#define PSU_ACTIVE_MONITORING_COMPARATOR_MODE 5

#if DEVICE_SUPPORTS_ACTIVE_POWER_MONITORING
  #include "../TeensyComparator/TeensyComparator.h"
#endif

// Pin definitions (used internally)
const int TRIGGER_OUTPUT_PIN_0 = 23;
const int TRIGGER_INPUT_PIN_0 = 22;
const int TRIGGER_OUTPUT_COUNT = 1;
const int TRIGGER_INPUT_COUNT = 1;

// Power sensing pin
const int POWER_SENSE_PIN = 23;

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "LuLed Quad";
const char * LedArrayInterface::device_hardware_revision = "0.01";
const float LedArrayInterface::max_na = 1.0;
const int16_t LedArrayInterface::led_count = 256;
const float LedArrayInterface::default_na = 0.5;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 1;
const int LedArrayInterface::trigger_input_count = 1;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.48, 0.525, 0.625};
const int LedArrayInterface::bit_depth = 8;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
int LedArrayInterface::debug = 0;
const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0};
bool LedArrayInterface::trigger_input_state[] = {false};
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];
const int LedArrayInterface::power_sense_pin = POWER_SENSE_PIN;

// Power source sensing variables
bool _psu_is_connected = true;
bool _power_source_sensing_is_enabled = false;
elapsedMillis _time_elapsed_debounce;
uint32_t _warning_delay_ms = 10;

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 0;
const char * LedArrayInterface::device_commandNamesShort[] = {};
const char * LedArrayInterface::device_commandNamesLong[] = {};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {}; // Number of commands, number of LEDs in each command.

//How many matrices are you using?
#define NUM_MATRICES 4
#define LEDS_PER_MATRIX 64
#define NUM_LEDS LEDS_PER_MATRIX * NUM_MATRICES

// The LuMini matrices need two data pins connected
#define DATA_PIN 16
#define CLOCK_PIN 17

// Define the array of leds
CRGB matrix[NUM_LEDS];

//The Height and Width of each individual matrix
int individualMatrixWidth = 8;
int individualMatrixHeight = 8;

const int moduleHeight = 2; //The Height of our display, measured in numbers of LuMini matrices
const int moduleWidth = 2; //The Width of our display, measured in numbers of LuMini matrices

//The following matrix represents our entire LED display, with the number that each module is in the chain. My data flows in to the bottom left matrix (matrix 0 in my chain),
//then it goes right to the bottom right matrix (matrix 1 in my chain) then up to the top right matrix (matrix 2) then to the remaining matrix 3
//The below table should have identical matrix numbers/positions as that of the multi-matrix display
int matrixMap[moduleHeight][moduleWidth] = {
  {3, 2},
  {0, 1}
};

// Define orientation
int orientation[moduleHeight * moduleWidth] = { 0, 0, 0, 1 };

// Define LED positions in cartesian coordinates (LED#, channel, x * 100mm, y * 100mm, z * 100mm)
PROGMEM const int16_t LedArrayInterface::led_positions[256][5] = {
   {0, 128, 200, 200, 0},
{1, 255, -200, 200, 0},
{2, 120, 200, -200, 0},
{3, 63, -200, -200, 0},
{4, 112, 200, -600, 0},
{5, 55, -200, -600, 0},
{6, 247, -600, 200, 0},
{7, 254, -200, 600, 0},
{8, 136, 200, 600, 0},
{9, 129, 600, 200, 0},
{10, 62, -600, -200, 0},
{11, 121, 600, -200, 0},
{12, 54, -600, -600, 0},
{13, 137, 600, 600, 0},
{14, 246, -600, 600, 0},
{15, 113, 600, -600, 0},
{16, 61, -1000, -200, 0},
{17, 130, 1000, 200, 0},
{18, 253, -200, 1000, 0},
{19, 144, 200, 1000, 0},
{20, 122, 1000, -200, 0},
{21, 47, -200, -1000, 0},
{22, 104, 200, -1000, 0},
{23, 239, -1000, 200, 0},
{24, 105, 600, -1000, 0},
{25, 46, -600, -1000, 0},
{26, 114, 1000, -600, 0},
{27, 238, -1000, 600, 0},
{28, 138, 1000, 600, 0},
{29, 53, -1000, -600, 0},
{30, 245, -600, 1000, 0},
{31, 145, 600, 1000, 0},
{32, 252, -200, 1400, 0},
{33, 60, -1400, -200, 0},
{34, 39, -200, -1400, 0},
{35, 96, 200, -1400, 0},
{36, 45, -1000, -1000, 0},
{37, 152, 200, 1400, 0},
{38, 123, 1400, -200, 0},
{39, 106, 1000, -1000, 0},
{40, 146, 1000, 1000, 0},
{41, 237, -1000, 1000, 0},
{42, 131, 1400, 200, 0},
{43, 231, -1400, 200, 0},
{44, 244, -600, 1400, 0},
{45, 153, 600, 1400, 0},
{46, 52, -1400, -600, 0},
{47, 97, 600, -1400, 0},
{48, 38, -600, -1400, 0},
{49, 139, 1400, 600, 0},
{50, 115, 1400, -600, 0},
{51, 230, -1400, 600, 0},
{52, 147, 1400, 1000, 0},
{53, 236, -1000, 1400, 0},
{54, 44, -1400, -1000, 0},
{55, 107, 1400, -1000, 0},
{56, 154, 1000, 1400, 0},
{57, 98, 1000, -1400, 0},
{58, 229, -1400, 1000, 0},
{59, 37, -1000, -1400, 0},
{60, 59, -1800, -200, 0},
{61, 160, 200, 1800, 0},
{62, 124, 1800, -200, 0},
{63, 251, -200, 1800, 0},
{64, 132, 1800, 200, 0},
{65, 31, -200, -1800, 0},
{66, 88, 200, -1800, 0},
{67, 223, -1800, 200, 0},
{68, 222, -1800, 600, 0},
{69, 30, -600, -1800, 0},
{70, 89, 600, -1800, 0},
{71, 161, 600, 1800, 0},
{72, 51, -1800, -600, 0},
{73, 243, -600, 1800, 0},
{74, 116, 1800, -600, 0},
{75, 140, 1800, 600, 0},
{76, 99, 1400, -1400, 0},
{77, 228, -1400, 1400, 0},
{78, 155, 1400, 1400, 0},
{79, 36, -1400, -1400, 0},
{80, 162, 1000, 1800, 0},
{81, 29, -1000, -1800, 0},
{82, 235, -1000, 1800, 0},
{83, 148, 1800, 1000, 0},
{84, 108, 1800, -1000, 0},
{85, 90, 1000, -1800, 0},
{86, 43, -1800, -1000, 0},
{87, 221, -1800, 1000, 0},
{88, 215, -2200, 200, 0},
{89, 133, 2200, 200, 0},
{90, 58, -2200, -200, 0},
{91, 125, 2200, -200, 0},
{92, 168, 200, 2200, 0},
{93, 23, -200, -2200, 0},
{94, 80, 200, -2200, 0},
{95, 250, -200, 2200, 0},
{96, 242, -600, 2200, 0},
{97, 214, -2200, 600, 0},
{98, 220, -1800, 1400, 0},
{99, 169, 600, 2200, 0},
{100, 141, 2200, 600, 0},
{101, 163, 1400, 1800, 0},
{102, 22, -600, -2200, 0},
{103, 227, -1400, 1800, 0},
{104, 28, -1400, -1800, 0},
{105, 91, 1400, -1800, 0},
{106, 117, 2200, -600, 0},
{107, 81, 600, -2200, 0},
{108, 156, 1800, 1400, 0},
{109, 50, -2200, -600, 0},
{110, 100, 1800, -1400, 0},
{111, 35, -1800, -1400, 0},
{112, 149, 2200, 1000, 0},
{113, 109, 2200, -1000, 0},
{114, 234, -1000, 2200, 0},
{115, 170, 1000, 2200, 0},
{116, 42, -2200, -1000, 0},
{117, 213, -2200, 1000, 0},
{118, 21, -1000, -2200, 0},
{119, 82, 1000, -2200, 0},
{120, 164, 1800, 1800, 0},
{121, 92, 1800, -1800, 0},
{122, 219, -1800, 1800, 0},
{123, 27, -1800, -1800, 0},
{124, 83, 1400, -2200, 0},
{125, 226, -1400, 2200, 0},
{126, 212, -2200, 1400, 0},
{127, 249, -200, 2600, 0},
{128, 134, 2600, 200, 0},
{129, 157, 2200, 1400, 0},
{130, 15, -200, -2600, 0},
{131, 72, 200, -2600, 0},
{132, 34, -2200, -1400, 0},
{133, 171, 1400, 2200, 0},
{134, 207, -2600, 200, 0},
{135, 176, 200, 2600, 0},
{136, 57, -2600, -200, 0},
{137, 20, -1400, -2200, 0},
{138, 101, 2200, -1400, 0},
{139, 126, 2600, -200, 0},
{140, 241, -600, 2600, 0},
{141, 118, 2600, -600, 0},
{142, 206, -2600, 600, 0},
{143, 142, 2600, 600, 0},
{144, 177, 600, 2600, 0},
{145, 14, -600, -2600, 0},
{146, 49, -2600, -600, 0},
{147, 73, 600, -2600, 0},
{148, 178, 1000, 2600, 0},
{149, 205, -2600, 1000, 0},
{150, 150, 2600, 1000, 0},
{151, 233, -1000, 2600, 0},
{152, 41, -2600, -1000, 0},
{153, 13, -1000, -2600, 0},
{154, 74, 1000, -2600, 0},
{155, 110, 2600, -1000, 0},
{156, 211, -2200, 1800, 0},
{157, 26, -2200, -1800, 0},
{158, 84, 1800, -2200, 0},
{159, 19, -1800, -2200, 0},
{160, 93, 2200, -1800, 0},
{161, 218, -1800, 2200, 0},
{162, 172, 1800, 2200, 0},
{163, 165, 2200, 1800, 0},
{164, 75, 1400, -2600, 0},
{165, 12, -1400, -2600, 0},
{166, 225, -1400, 2600, 0},
{167, 158, 2600, 1400, 0},
{168, 33, -2600, -1400, 0},
{169, 102, 2600, -1400, 0},
{170, 179, 1400, 2600, 0},
{171, 204, -2600, 1400, 0},
{172, 248, -200, 3000, 0},
{173, 184, 200, 3000, 0},
{174, 127, 3000, -200, 0},
{175, 64, 200, -3000, 0},
{176, 7, -200, -3000, 0},
{177, 135, 3000, 200, 0},
{178, 199, -3000, 200, 0},
{179, 56, -3000, -200, 0},
{180, 48, -3000, -600, 0},
{181, 65, 600, -3000, 0},
{182, 119, 3000, -600, 0},
{183, 6, -600, -3000, 0},
{184, 240, -600, 3000, 0},
{185, 198, -3000, 600, 0},
{186, 143, 3000, 600, 0},
{187, 185, 600, 3000, 0},
{188, 173, 2200, 2200, 0},
{189, 18, -2200, -2200, 0},
{190, 85, 2200, -2200, 0},
{191, 210, -2200, 2200, 0},
{192, 197, -3000, 1000, 0},
{193, 166, 2600, 1800, 0},
{194, 217, -1800, 2600, 0},
{195, 66, 1000, -3000, 0},
{196, 11, -1800, -2600, 0},
{197, 151, 3000, 1000, 0},
{198, 94, 2600, -1800, 0},
{199, 203, -2600, 1800, 0},
{200, 76, 1800, -2600, 0},
{201, 180, 1800, 2600, 0},
{202, 232, -1000, 3000, 0},
{203, 40, -3000, -1000, 0},
{204, 111, 3000, -1000, 0},
{205, 186, 1000, 3000, 0},
{206, 25, -2600, -1800, 0},
{207, 5, -1000, -3000, 0},
{208, 4, -1400, -3000, 0},
{209, 224, -1400, 3000, 0},
{210, 67, 1400, -3000, 0},
{211, 187, 1400, 3000, 0},
{212, 159, 3000, 1400, 0},
{213, 196, -3000, 1400, 0},
{214, 103, 3000, -1400, 0},
{215, 32, -3000, -1400, 0},
{216, 10, -2200, -2600, 0},
{217, 202, -2600, 2200, 0},
{218, 86, 2600, -2200, 0},
{219, 17, -2600, -2200, 0},
{220, 209, -2200, 2600, 0},
{221, 174, 2600, 2200, 0},
{222, 181, 2200, 2600, 0},
{223, 77, 2200, -2600, 0},
{224, 216, -1800, 3000, 0},
{225, 95, 3000, -1800, 0},
{226, 188, 1800, 3000, 0},
{227, 68, 1800, -3000, 0},
{228, 24, -3000, -1800, 0},
{229, 195, -3000, 1800, 0},
{230, 167, 3000, 1800, 0},
{231, 3, -1800, -3000, 0},
{232, 201, -2600, 2600, 0},
{233, 182, 2600, 2600, 0},
{234, 9, -2600, -2600, 0},
{235, 78, 2600, -2600, 0},
{236, 2, -2200, -3000, 0},
{237, 208, -2200, 3000, 0},
{238, 194, -3000, 2200, 0},
{239, 175, 3000, 2200, 0},
{240, 69, 2200, -3000, 0},
{241, 189, 2200, 3000, 0},
{242, 16, -3000, -2200, 0},
{243, 87, 3000, -2200, 0},
{244, 183, 3000, 2600, 0},
{245, 79, 3000, -2600, 0},
{246, 200, -2600, 3000, 0},
{247, 193, -3000, 2600, 0},
{248, 70, 2600, -3000, 0},
{249, 190, 2600, 3000, 0},
{250, 1, -2600, -3000, 0},
{251, 8, -3000, -2600, 0},
{252, 0, -3000, -3000, 0},
{253, 71, 3000, -3000, 0},
{254, 192, -3000, 3000, 0},
{255, 191, 3000, 3000, 0},
};

void LedArrayInterface::set_max_current_enforcement(bool enforce)
{
  not_implemented("LedArrayInterface::set_max_current_enforcement");
}

void LedArrayInterface::set_max_current_limit(float limit)
{
  not_implemented("LedArrayInterface::set_max_current_limit");
}

void LedArrayInterface::set_pin_order(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
  not_implemented("LedArrayInterface::set_pin_order");
}

void LedArrayInterface::not_implemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
}

uint16_t LedArrayInterface::get_led_value(uint16_t led_number, int color_channel_index)
{
  if ((color_channel_index >= 0) && (color_channel_index < 3))
      return matrix[led_number][color_channel_index];
  else
  return 0;
}

void LedArrayInterface::set_ledFast(int16_t led_number, int color_channel_index, bool value)
{
  not_implemented("set_ledFast");
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

int8_t LedArrayInterface::getDemoMode()
{
  int8_t demo_mode_read = EEPROM.read(DEMO_MODE_ADDRESS);
  return (demo_mode_read);
}

void LedArrayInterface::setDemoMode(int8_t demo_mode)
{
	EEPROM.write(DEMO_MODE_ADDRESS, demo_mode);
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
    return (trigger_input_state[trigger_pin]);
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
  FastLED.show();
}

void LedArrayInterface::clear()
{
  FastLED.clearData();
}

bool LedArrayInterface::get_max_current_enforcement()
{
        return false;
}

float LedArrayInterface::get_max_current_limit()
{
       return 0.0;
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
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
    {
      matrix[channel_number].setRGB(value, value, value);
    }
    else if (color_channel_number < 3)
    {
      matrix[channel_number][color_channel_number] = value;
    }
    else
      not_implemented("LedArrayInterface::set_channel");
  }
  else
  {
    Serial.print(F("Error (LedArrayInterface::set_channel): Invalid channel ("));
    Serial.print(channel_number);
    Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
  }
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
        set_channel(channel_number, color_channel_number, (uint8_t) (value * UINT8_MAX / UINT16_MAX));
}

void LedArrayInterface::set_channel(int16_t channel_number, int16_t color_channel_number, bool value)
{
        set_channel(channel_number, color_channel_number, (uint8_t) (value > 0 * UINT16_MAX));
}

void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, uint8_t value)
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

void LedArrayInterface::set_led(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.print(color_channel_number);
    Serial.print(SERIAL_LINE_ENDING);
  }
  set_led(led_number, color_channel_number, (uint8_t) (value * UINT8_MAX / UINT16_MAX));
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
  set_led(led_number, color_channel_number, (uint8_t) (value * UINT8_MAX));
}

void LedArrayInterface::device_reset()
{
  device_setup();
}

void LedArrayInterface::device_setup()
{
  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(matrix, NUM_LEDS);
  LEDS.setBrightness(128);

  // Output trigger Pins
  for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
  {
    pinMode(trigger_output_pin_list[trigger_index], OUTPUT);
    digitalWrite(trigger_output_pin_list[trigger_index], LOW);
  }

  // Input trigger Pins
  for (int trigger_index = 0; trigger_index < trigger_input_count; trigger_index++)
    pinMode(trigger_input_pin_list[trigger_index], INPUT);

  // Clear the array
  clear();

    // Output trigger Pins
    for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
    {
            pinMode(trigger_output_pin_list[trigger_index], OUTPUT);
            digitalWriteFast(trigger_output_pin_list[trigger_index], LOW);
    }

    // Input trigger Pins
    for (int trigger_index = 0; trigger_index < trigger_input_count; trigger_index++)
            pinMode(trigger_input_pin_list[trigger_index], INPUT);

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
  Serial.printf(F("ERROR (LedArrayInterface::get_device_command_led_list_size): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
  return 0;
}

void LedArrayInterface::set_gsclk_frequency(uint32_t gsclk_frequency)
{
  not_implemented("LedArrayInterface::set_gsclk_frequency");
}

uint32_t LedArrayInterface::get_gsclk_frequency()
{
  not_implemented("LedArrayInterface::get_gsclk_frequency");
  return 0;
}

void LedArrayInterface::set_sclk_baud_rate(uint32_t new_baud_rate)
{
  not_implemented("LedArrayInterface::set_sclk_baud_rate");
}

uint32_t LedArrayInterface::get_sclk_baud_rate()
{
  not_implemented("LedArrayInterface::get_sclk_baud_rate");
  return 0;
}

#endif

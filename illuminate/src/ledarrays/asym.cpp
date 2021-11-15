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

#include "../../illuminate.h"
#ifdef USE_SCI_ASYM_ARRAY
#include "../../ledarrayinterface.h"
#include "../TLC5955/TLC5955.h"

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
const float LedArrayInterface::max_na = 0.8;
const int16_t LedArrayInterface::led_count = 48;
const float LedArrayInterface::default_na = 0.4;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.48, 0.525, 0.625};
const int LedArrayInterface::bit_depth = 16;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
int LedArrayInterface::debug = 0;
const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};
bool LedArrayInterface::trigger_input_state[] = {false, false};
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];

const uint8_t TLC5955::_tlc_count = 100;    // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 10.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc;                            // TLC5955 object
uint32_t gsclk_frequency = 2000000;     // Grayscale clock speed

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 1;
const char * LedArrayInterface::deviceCommandNamesShort[] = {"c"};
const char * LedArrayInterface::deviceCommandNamesLong[] = {"center"};
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

void LedArrayInterface::setMaxCurrentEnforcement(bool enforce)
{
        TLC5955::enforce_max_current = enforce;
}

void LedArrayInterface::setMaxCurrentLimit(float limit)
{
        if (limit > 0)
                TLC5955::max_current_amps = limit;
}

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
        tlc.setPinOrderSingle(led_number, color_channel_index, position);
}

void LedArrayInterface::notImplemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
}

uint16_t LedArrayInterface::getLedValue(uint16_t led_number, int color_channel_index)
{
        int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
        if (channel_number >= 0)
                return tlc.getChannelValue(channel_number, color_channel_index);
        else
        {
                Serial.print(F("ERROR (LedArrayInterface::getLedValue) - invalid LED number ("));
                Serial.print(led_number);
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
                return 0;
        }
}

void LedArrayInterface::setLedFast(int16_t led_number, int color_channel_index, bool value)
{
        notImplemented("setLedFast");
}

uint16_t LedArrayInterface::getSerialNumber()
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

uint16_t LedArrayInterface::getPartNumber()
{
  uint16_t pn_read = (EEPROM.read(PN_ADDRESS + 1) << 8) | EEPROM.read(PN_ADDRESS);
  return (pn_read);
}

void LedArrayInterface::setPartNumber(uint16_t part_number)
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
bool LedArrayInterface::getDebug()
{
        return (LedArrayInterface::debug);
}

void LedArrayInterface::setDebug(int state)
{
        LedArrayInterface::debug = state;
        Serial.printf(F("(LedArrayInterface::setDebug): Set debug level to %d \n"), debug);
}

int LedArrayInterface::setTriggerState(int trigger_index, bool state)
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

int LedArrayInterface::getInputTriggerState(int input_trigger_index)
{
        // Get trigger pin
        int trigger_pin = trigger_input_pin_list[input_trigger_index];
        if (trigger_pin > 0)
                return (trigger_input_state[trigger_pin]);
        else
                return (-1);
}

int LedArrayInterface::sendTriggerPulse(int trigger_index, uint16_t delay_us, bool inverse_polarity)
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
        update();
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
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
                        tlc.setLed(channel_number, value);
                else if (color_channel_number == 0)
                        tlc.setLed(channel_number, value, tlc.getChannelValue(channel_number, 1), tlc.getChannelValue(channel_number, 2));
                else if (color_channel_number == 1)
                        tlc.setLed(channel_number, tlc.getChannelValue(channel_number, 0), value, tlc.getChannelValue(channel_number, 2));
                else if (color_channel_number == 2)
                        tlc.setLed(channel_number, tlc.getChannelValue(channel_number, 0), tlc.getChannelValue(channel_number, 1), value);
        }
        else
        {
                Serial.print(F("Error (LedArrayInterface::setChannel): Invalid channel ("));
                Serial.print(channel_number);
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
        }
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
{
        setChannel(channel_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, bool value)
{
        setChannel(channel_number, color_channel_number, (uint16_t) (value > 0 * UINT16_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint16_t value)
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
                        setChannel(channel_number, color_channel_number, value);
                }
        }
        else
        {
                int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
                setChannel(channel_number, color_channel_number, value);
        }
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint8_t value)
{
        if (debug >= 2)
        {
                Serial.print("U8 Setting led #");
                Serial.print(led_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, bool value)
{
        if (debug >= 2)
        {
                Serial.print("B Setting led #");
                Serial.print(led_number);
                Serial.print(", color channel #");
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::deviceReset()
{
  deviceSetup();
}

void LedArrayInterface::deviceSetup()
{
        // Initialize TLC5955
        tlc.init(LAT, SPI_MOSI, SPI_CLK, GSCLK);

        // We must set dot correction values, so set them all to the brightest adjustment
        tlc.setAllDcData(127);

        // Set Max Current Values (see TLC5955 datasheet)
        tlc.setMaxCurrent(3, 3, 3); // Go up to 7

        // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
        // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT
        tlc.setFunctionData(true, true, true, true, true); // WORKS with fast update

        // Set all LED current levels to max (127)
        int currentR = 127;
        int currentB = 127;
        int currentG = 127;
        tlc.setBrightnessCurrent(currentR, currentB, currentG);

        // Update vontrol register
        tlc.updateControl();
        clear();
        tlc.updateControl();

        // Set RGB pin order
        tlc.setRgbPinOrder(0, 1, 2);

        // Update the GS register
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

void LedArrayInterface::sourceChangeIsr()
{
        Serial.printf(F("ERROR (LedArrayInterface::sourceChangeIsr): PSU Monitoring not supported on this device."), SERIAL_LINE_ENDING);
}

float LedArrayInterface::getPowerSourceVoltage()
{
    return -1.0;
}

bool LedArrayInterface::getPowerSourceMonitoringState()
{
  return false;
}

int16_t LedArrayInterface::getDevicePowerSensingCapability()
{
      return NO_PSU_SENSING;
}

void LedArrayInterface::setPowerSourceMonitoringState(bool new_state)
{
        Serial.printf(F("ERROR (LedArrayInterface::setPowerSourceMonitoringState): PSU Monitoring not supported on this device."), SERIAL_LINE_ENDING);
}

bool LedArrayInterface::isPowerSourcePluggedIn()
{
  return true;
}

uint8_t LedArrayInterface::getDeviceCommandCount()
{
  return (LedArrayInterface::device_command_count);
}

const char * LedArrayInterface::getDeviceCommandNameShort(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::deviceCommandNamesShort[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

const char * LedArrayInterface::getDeviceCommandNameLong(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::deviceCommandNamesLong[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

uint32_t LedArrayInterface::getDeviceCommandLedListSize(int device_command_index)
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
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

uint16_t LedArrayInterface::getDeviceCommandLedListElement(int device_command_index, uint16_t pattern_index, uint16_t led_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
        {
                uint32_t concatenated = getDeviceCommandLedListSize(device_command_index);
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
                        Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid pattern index (%d) / led index (%d)"), pattern_index, led_index, SERIAL_LINE_ENDING);
                        return (0);
                }
        }
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

void LedArrayInterface::setGsclkFreq(uint32_t gsclk_frequency)
{
  tlc.setGsclkFreq(gsclk_frequency);
}

uint32_t LedArrayInterface::getGsclkFreq()
{
  return tlc.getGsclkFreq();
}

void LedArrayInterface::setBaudRate(uint32_t new_baud_rate)
{
  tlc.setSpiBaudRate(new_baud_rate);
}

uint32_t LedArrayInterface::getBaudRate()
{
  return tlc.getSpiBaudRate();
}

#endif

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
   DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "../../illuminate.h"
#ifdef USE_LUMINI_RING

#include "../../ledarrayinterface.h"
#include <FastLED.h>

// Power monitoring commands
#define DEVICE_SUPPORTS_POWER_SENSING 0
#define PSU_ACTIVE_MONITORING_COMPARATOR_MODE 5

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
const int16_t LedArrayInterface::led_count = 20;
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
const char * LedArrayInterface::deviceCommandNamesShort[] = {};
const char * LedArrayInterface::deviceCommandNamesLong[] = {};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {}; // Number of commands, number of LEDs in each command.

// LED Count
#define NUM_LEDS 20

// The LuMini matrices need two data pins connected
#define DATA_PIN 16
#define CLOCK_PIN 17

// Define the array of leds
CRGB ring[NUM_LEDS];

// Define LED positions in cartesian coordinates (LED#, channel, x * 100mm, y * 100mm, z * 100mm)
PROGMEM const int16_t LedArrayInterface::led_positions[20][5] = {
{0, 0, 1270, 0, 6000},
{1, 1, 1201, 412, 6000},
{2, 2, 1002, 780, 6000},
{3, 3, 694, 1063, 6000},
{4, 4, 311, 1231, 6000},
{5, 5, -104, 1265, 6000},
{6, 6, -510, 1163, 6000},
{7, 7, -860, 934, 6000},
{8, 8, -1116, 604, 6000},
{9, 9, -1252, 209, 6000},
{10, 10, -1252, -209, 6000},
{11, 11, -1116, -604, 6000},
{12, 12, -860, -934, 6000},
{13, 13, -510, -1163, 6000},
{14, 14, -104, -1265, 6000},
{15, 15, 311, -1231, 6000},
{16, 16, 694, -1063, 6000},
{17, 17, 1002, -780, 6000},
{18, 18, 1201, -412, 6000},
{19, 19, 1270, 0, 6000},
};

void LedArrayInterface::setMaxCurrentEnforcement(bool enforce)
{
  notImplemented("LedArrayInterface::setMaxCurrentEnforcement");
}

void LedArrayInterface::setMaxCurrentLimit(float limit)
{
  notImplemented("LedArrayInterface::setMaxCurrentLimit");
}

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
  notImplemented("LedArrayInterface::setPinOrder");
}

void LedArrayInterface::notImplemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
}

uint16_t LedArrayInterface::getLedValue(uint16_t led_number, int color_channel_index)
{
  if ((color_channel_index >= 0) && (color_channel_index < 3))
      return ring[led_number][color_channel_index];
  else
  return 0;
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
  FastLED.show();
}

void LedArrayInterface::clear()
{
  FastLED.clearData();
  update();
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
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
      ring[channel_number].setRGB(value, value, value);
    }
    else if (color_channel_number < 3)
    {
      ring[channel_number][color_channel_number] = value;
    }
    else
      notImplemented("LedArrayInterface::setChannel");
  }
  else
  {
    Serial.print(F("Error (LedArrayInterface::setChannel): Invalid channel ("));
    Serial.print(channel_number);
    Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
  }
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
        setChannel(channel_number, color_channel_number, (uint8_t) (value * UINT8_MAX / UINT16_MAX));
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, bool value)
{
        setChannel(channel_number, color_channel_number, (uint8_t) (value > 0 * UINT16_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint8_t value)
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

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.print(color_channel_number);
    Serial.print(SERIAL_LINE_ENDING);
  }
  setLed(led_number, color_channel_number, (uint8_t) (value * UINT8_MAX / UINT16_MAX));
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
  setLed(led_number, color_channel_number, (uint8_t) (value * UINT8_MAX));
}

void LedArrayInterface::deviceReset()
{
  deviceSetup();
}

void LedArrayInterface::sourceChangeIsr()
{
}

float LedArrayInterface::getPowerSourceVoltage()
{
  return -1.0;
}

bool LedArrayInterface::getPowerSourceMonitoringState()
{
  return _power_source_sensing_is_enabled;
}

int16_t LedArrayInterface::getDevicePowerSensingCapability()
{
  return NO_PSU_SENSING;
}

void LedArrayInterface::setPowerSourceMonitoringState(bool new_state)
{
  ;
}

bool LedArrayInterface::isPowerSourcePluggedIn()
{
  return true;
}

void LedArrayInterface::deviceSetup()
{

  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(ring, 20);
  LEDS.setBrightness(255);

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
  Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
  return 0;
}

void LedArrayInterface::setGsclkFreq(uint32_t gsclk_frequency)
{
  notImplemented("LedArrayInterface::setGsclkFreq");
}

uint32_t LedArrayInterface::getGsclkFreq()
{
  notImplemented("LedArrayInterface::getGsclkFreq");
  return 0;
}

void LedArrayInterface::setBaudRate(uint32_t new_baud_rate)
{
  notImplemented("LedArrayInterface::setBaudRate");
}

uint32_t LedArrayInterface::getBaudRate()
{
  notImplemented("LedArrayInterface::getBaudRate");
  return 0;
}

#endif

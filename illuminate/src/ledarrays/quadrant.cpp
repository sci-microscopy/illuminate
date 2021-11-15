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
#ifdef USE_QUADRANT_ARRAY
#include "../../ledarrayinterface.h"

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
const int TRIGGER_OUTPUT_PIN_0 = 23;
const int TRIGGER_INPUT_PIN_0 = 22;
const int TRIGGER_OUTPUT_PIN_1 = 20;
const int TRIGGER_INPUT_PIN_1 = 19;
const int TRIGGER_OUTPUT_COUNT = 2;
const int TRIGGER_INPUT_COUNT = 2;

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// Quadrant pins
#define Q1_PIN 6
#define Q2_PIN 9
#define Q3_PIN 5
#define Q4_PIN 10

// Power source sensing variables
bool _psu_is_connected = true;
bool _power_source_sensing_is_enabled = false;
elapsedMillis _time_elapsed_debounce;
uint32_t _warning_delay_ms = 10;

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "quadrant-array";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 1.0;
const int16_t LedArrayInterface::led_count = 4;
const uint16_t LedArrayInterface::center_led = 0;
const float LedArrayInterface::default_na = 0.4;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 1;
const char LedArrayInterface::color_channel_names[] = {'w'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.53};
const int LedArrayInterface::bit_depth = 8;
const int16_t LedArrayInterface::tlc_chip_count = 0;
const bool LedArrayInterface::supports_fast_sequence = true;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];

// Fast pins
volatile const int fast_pin_list[] = {Q1_PIN, Q2_PIN, Q3_PIN, Q4_PIN};
volatile const int fast_pin_count = 4;

// Set up trigger pins
const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};

bool LedArrayInterface::trigger_input_state[] = {false, false};

int LedArrayInterface::debug = 0;
bool digital_mode = true;

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 0;
const char * LedArrayInterface::deviceCommandNamesShort[] = {};
const char * LedArrayInterface::deviceCommandNamesLong[] = {};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {};

// FORMAT: LED number, channel, 100*x, 100*y, 100*z
const int16_t PROGMEM LedArrayInterface::led_positions[][5] = {
  {1, 0, 1, 1, 4000,},
  {2, 1, -1, 1, 4000,},
  {3, 2, 1, -1, 4000,},
  {4, 3, -1, -1, 4000,}
};

/* Device-specific variables */
int pin_numbers[LedArrayInterface::led_count] = {Q1_PIN, Q2_PIN, Q3_PIN, Q4_PIN};
uint8_t led_values[LedArrayInterface::led_count] = {0, 0, 0, 0};

/**** Part number and Serial number addresses in EEPROM ****/
uint16_t pn_address = 100;
uint16_t sn_address = 200;

void LedArrayInterface::setMaxCurrentEnforcement(bool enforce)
{
  ;
}

void LedArrayInterface::setMaxCurrentLimit(float limit)
{
  ;
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
	EEPROM.write(sn_address, lower_8bits_sn);
	EEPROM.write(sn_address + 1, upper_8bits_sn);
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
	EEPROM.write(pn_address, lower_8bits_pn);
	EEPROM.write(pn_address + 1, upper_8bits_pn);
}

void LedArrayInterface::notImplemented(const char * command_name)
{
  Serial.print(F("Command "));
  Serial.print(command_name);
  Serial.println(F(" is not implemented for this device."));
}

// Debug Variables
bool LedArrayInterface::getDebug()
{
  return (LedArrayInterface::debug);
}

void LedArrayInterface::setDebug(int state)
{
  debug = state;
  Serial.printf(F("(LedArrayInterface::setDebug): Set debug level to %d \n"), debug);
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_index, uint8_t value)
{
  if (color_channel_index == 0 || color_channel_index == -1)
  {
    if (channel_number < 0)
      Serial.println(F("ERROR (LedArrayInterface::setChannel): Invalid channel."));
    else
    {
      // Find the led number corresponding to this channel
      int16_t current_channel_number;
      for (uint16_t led_number = 0; led_number < LedArrayInterface::led_count; led_number++)
      {
        current_channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
        if (current_channel_number == channel_number)
          led_values[led_number] = value;
      }
    }
  }
  else
  {
    Serial.print(F("ERROR (LedArrayInterface::setLed): Invalid color channel "));
    Serial.println(color_channel_index);
  }
}


void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_index, uint16_t value)
{
  setChannel(channel_number, color_channel_index, (uint8_t)round(value * UINT8_MAX / UINT16_MAX));
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_index, bool value)
{
  setChannel(channel_number, color_channel_index, (uint8_t) (value * UINT8_MAX));
}

void LedArrayInterface::setChannelFast(uint16_t channel_number, int color_channel_index, bool value)
{
  if (channel_number < 0)
    Serial.println(F("ERROR (LedArrayInterface::setChannelFast): invalid channel."));
  else
  {
    pinMode(pin_numbers[channel_number], OUTPUT);
    digitalWriteFast(pin_numbers[channel_number], value);
    led_values[channel_number] = value;
  }
}

uint16_t LedArrayInterface::getLedValue(uint16_t led_number, int color_channel_index)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
  if (channel_number >= 0)
    return led_values[channel_number];
  else
  {
    Serial.print(F("ERROR (LedArrayInterface::getLedValue) - invalid LED number ("));
    Serial.print(led_number);
    Serial.println(F(")"));
    return 0;
  }
}

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
  notImplemented("SetPinOrder");
}

void LedArrayInterface::setLedFast(int16_t led_number, int color_channel_index, bool value)
{
  if (led_number < 0)
  {
    for (uint16_t led_index = 0; led_index < led_count; led_index++)
    {
      // Update
      if (pin_numbers[led_index] >= 0)
      {
        if (debug >= 2)
        {
          Serial.print(F("Quickly Setting pin #"));
          Serial.print(pin_numbers[led_index]);
          Serial.print(F(" to "));
          Serial.println(value);
        }
        if (!digital_mode)
          pinMode(pin_numbers[led_index], OUTPUT);
        digitalWriteFast(pin_numbers[led_index], value);
        led_values[led_index] = value;
      }
      else
      {
        Serial.print(F("ERROR (LedArrayInterface::setLedFast, all leds): Invalid LED number (led# "));
        Serial.print(led_number);
        Serial.print(F(", pin #"));
        Serial.println(pin_numbers[led_index]);
      }
    }
  }
  else
  {
    // Update
    if (pin_numbers[led_number] >= 0)
    {
      if (debug >= 2)
      {
        Serial.print(F("Quickly Setting pin #"));
        Serial.print(pin_numbers[led_number]);
        Serial.print(F(" to "));
        Serial.println(value);
      }
      if (!digital_mode)
        pinMode(pin_numbers[led_number], OUTPUT);
      digitalWriteFast(pin_numbers[led_number], value);
      led_values[led_number] = value;
    }
    else
      Serial.println(F("ERROR (LedArrayInterface::setLedFast, single led): Invalid LED number."));
  }
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
  noInterrupts();
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

    //  Write inactive state
    if (inverse_polarity)
      digitalWriteFast(trigger_pin, HIGH);
    else
      digitalWriteFast(trigger_pin, LOW);
    interrupts();
    if (debug >= 2)
      Serial.printf(F("(LedArrayInterface::sendTriggerPulse): Sent trigger pulse on pin %d with delay %d\n"), trigger_index, delay_us);
    return (1);

  } else {
    return (-1);
  }
}

void LedArrayInterface::update()
{
  // Indicate we are now in analog mode, and will need to re-call pinMode to use setLedFast again
  digital_mode = false;

  // Send illuminaiton values
  for (uint16_t led_index = 0; led_index < led_count; led_index++)
  {
    // Get channel number
    int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));

    // Update
    digitalWriteFast(pin_numbers[channel_number], led_values[led_index]);
  }
}

void LedArrayInterface::clear()
{
  digital_mode = false; // ensure pin mode gets configured
  setLedFast(-1, -1, false);
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_index, uint8_t value)
{
  if (color_channel_index == 0 || color_channel_index == -1)
  {
    if (led_number < 0)
    {
      for (uint16_t led_index = 0; led_index < LedArrayInterface::led_count; led_index++)
      {
        // Get channel number
        int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));

        if (channel_number >= 0)
          led_values[led_index] = value;
        else
          Serial.println(F("ERROR (LedArrayInterface::setLed): Invalid led number."));
      }
    }
    else if (led_number < LedArrayInterface::led_count)
    {
      int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));

      if (channel_number >= 0)
        led_values[led_number] = value;
      else
        Serial.println(F("ERROR (LedArrayInterface::setLed): Invalid led number."));
    }
    else
      Serial.println(F("ERROR (LedArrayInterface::setLed): Invalid led number."));
  }
  else
  {
    Serial.print(F("ERROR (LedArrayInterface::setLed): Invalid color channel "));
    Serial.println(color_channel_index);
  }
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_index, uint16_t value)
{
  setLed(led_number, color_channel_index, (uint8_t)round(value * UINT8_MAX / UINT16_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_index, bool value)
{
  setLed(led_number, color_channel_index, (uint8_t) (value * UINT8_MAX));
}

void LedArrayInterface::deviceReset()
{
  deviceSetup();
}

void LedArrayInterface::deviceSetup()
{
  // Now set the GSCK to an output and a 50% PWM duty-cycle
  // For simplicity all three grayscale clocks are tied to the same pin
  for (uint16_t led_index = 0; led_index < led_count; led_index++)
  {
    int16_t channel = (int16_t)pgm_read_word(&(led_positions[led_index][1]));
    pinMode(pin_numbers[channel], OUTPUT);
  }

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  // Adjust PWM timer for 8-bit precision
  analogWriteResolution(8);

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
                        //      if (device_command_index == 0)
                        //        return (uint16_t)pgm_read_word(&(hole_led_list[pattern_index][led_index]));
                        //      else if (device_command_index == 1)
                        //        return (uint16_t)pgm_read_word(&(uv_led_list[pattern_index][led_index]));
                        //      else
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
  notImplemented("This LED array does not have a GSCLK.");
}

uint32_t LedArrayInterface::getGsclkFreq()
{
  return 0;
}

void LedArrayInterface::setBaudRate(uint32_t new_baud_rate)
{
  notImplemented("This LED array does not have a TLC chip.");
}

uint32_t LedArrayInterface::getBaudRate()
{
  return 0;
}

#endif

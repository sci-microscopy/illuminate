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

#include "ledarrayinterface.h"

// Pin definitions (used internally)
#define GSCLK 6 // 10 on Arduino Mega
#define LAT 3   // 44 on Arduino Mega
#define SPI_MOSI 11
#define SPI_CLK 13
#define TRIGGER_OUTPUT_PIN_0 23
#define TRIGGER_INPUT_PIN_0 20
#define TRIGGER_OUTPUT_COUNT 1
#define TRIGGER_INPUT_COUNT 1

#define SN 1

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "sci-target";
const int LedArrayInterface::serial_number = SN;
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 0.5;
const int16_t LedArrayInterface::led_count = 257;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 1;
const int LedArrayInterface::trigger_input_count = 1;
const int LedArrayInterface::color_channel_count = 1;
const float color_channel_center_wavelengths[] = {0.53};
const char LedArrayInterface::color_channel_names[] = {'g'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.53};
const int LedArrayInterface::bit_depth = 16;
const int16_t LedArrayInterface::tlc_chip_count = 6;
const bool LedArrayInterface::supports_fast_sequence = false;
const int trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0};
const int trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0};


int LedArrayInterface::debug = 0;

/**** Device-specific variables ****/
TLC5955 tlc; // TLC5955 object
uint32_t gsclk_frequency = 5000000;
int update_write_count = 2;

void LedArrayInterface::notImplemented(const char * command_name)
{
  Serial.print(F("Command "));
  Serial.print(command_name);
  Serial.println(F(" is not implemented for this device."));
}

uint16_t LedArrayInterface::getLedValue(uint16_t led_number, int color_channel_index)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_number][1]));
  if (channel_number >= 0)
    return tlc.getChannelValue(channel_number, color_channel_index);
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

void LedArrayInterface::setLedFast(uint16_t led_number, int color_channel_index, bool value)
{
  notImplemented("setLedFast");
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
  tlc.updateLeds();
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
  if (debug >= 2)
  {
    Serial.print(F("U16 Setting channel #"));
    Serial.print(channel_number);
    Serial.print(F(" to value "));
    Serial.println(value);
  }

  if (channel_number >= 0)
    tlc.setChannel(channel_number, value);
  else
  {
    Serial.print(F("Error (LedArrayInterface::setChannel): Invalid channel ("));
    Serial.print(channel_number);
    Serial.println(F(")"));
  }
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting channel #");
    Serial.print(channel_number);
    Serial.print(", color channel #");
    Serial.println(color_channel_number);
  }
  setChannel(channel_number, color_channel_number, (uint16_t)( value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, bool value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting channel #");
    Serial.print(channel_number);
    Serial.print(", color channel #");
    Serial.println(color_channel_number);
  }
  setChannel(channel_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
  int16_t channel_number = -1;

  if (led_number < 0)
  {
    for (uint16_t led_index = 0; led_index < led_count; led_index++)
    {
      channel_number = (int16_t)pgm_read_word(&(ledMap[led_index][1]));
      setChannel(channel_number, color_channel_number, value);
    }
  }
  else
  {
    channel_number = (int16_t)pgm_read_word(&(ledMap[led_number][1]));
    setChannel(channel_number, color_channel_number, value);
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
    Serial.println(value);
  }
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint8_t value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.println(color_channel_number);
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
    Serial.println(color_channel_number);
  }
  setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::deviceSetup()
{
  // Now set the GSCK to an output and a 50% PWM duty-cycle
  // For simplicity all three grayscale clocks are tied to the same pin
  pinMode(GSCLK, OUTPUT);
  pinMode(LAT, OUTPUT);

  // Adjust PWM timer for maximum GSCLK frequency (5 MHz)
  analogWriteFrequency(GSCLK, gsclk_frequency);
  analogWriteResolution(1);
  analogWrite(GSCLK, 1);

  // The library does not ininiate SPI for you, so as to prevent issues with other SPI libraries
  SPI.setMOSI(SPI_MOSI);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);

  tlc.init(tlc_chip_count, true, LAT, SPI_MOSI, SPI_CLK);

  // We must set dot correction values, so set them all to the brightest adjustment
  tlc.setAllDcData(127);

  // Set Max Current Values (see TLC5955 datasheet)
  tlc.setMaxCurrent(3, 3, 3); // Go up to 7

  // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
  // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT
  tlc.setFunctionData(true, true, false, true, true); // WORKS with fast update

  // set all brightness levels to max (127)
  int currentR = 127;
  int currentB = 127;
  int currentG = 127;
  tlc.setBrightnessCurrent(currentR, currentB, currentG);

  // Update Control Register
  tlc.updateControl();

  // Update the GS register (ideally LEDs should be dark up to here)
  tlc.setAllLed(0);
  tlc.updateLeds();

  // Set up trigger pins
  trigger_output_pin_list[0] = TRIGGER_OUTPUT_PIN_0;
  trigger_input_pin_list[0] = TRIGGER_INPUT_PIN_0;

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

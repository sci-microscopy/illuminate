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
#ifdef USE_QUADRANT_ARRAY
#include "../../ledarrayinterface.h"
#include "../TLC5955/TLC5955.h"

// Pin definitions (used internally)
#define TRIGGER_OUTPUT_PIN_0 23
#define TRIGGER_OUTPUT_PIN_1 22
#define TRIGGER_INPUT_PIN_0 20
#define TRIGGER_INPUT_PIN_1 19
#define TRIGGER_OUTPUT_COUNT 2
#define TRIGGER_INPUT_COUNT 2

#define Q1_PIN 6
#define Q2_PIN 5
#define Q3_PIN 9
#define Q4_PIN 10

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "quadrant-array";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 1.0;
const int16_t LedArrayInterface::led_count = 4;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 1;
const char LedArrayInterface::color_channel_names[] = {'w'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.53};
const int LedArrayInterface::bit_depth = 8;
const int16_t LedArrayInterface::tlc_chip_count = 0;
const bool LedArrayInterface::supports_fast_sequence = true;
const float LedArrayInterface::led_array_distance_z_default = 50.0;

// Set up trigger pins
const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};

bool LedArrayInterface::trigger_input_state[] = {false, false};

int LedArrayInterface::debug = 0;
bool digital_mode = true;

// FORMAT: LED number, channel, 100*x, 100*y, 100*z
const int16_t PROGMEM LedArrayInterface::led_positions[][5] = {
        {0, 0, 1, 1, 4000,},
        {1, 1, -1, 1, 4000,},
        {2, 2, 1, -1, 4000,},
        {3, 3, -1, -1, 4000,}
};

/* Device-specific variables */
int pin_numbers[4] = {Q1_PIN, Q2_PIN, Q3_PIN, Q4_PIN};
uint8_t led_values[4] = {0, 0, 0, 0};

void LedArrayInterface::notImplemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
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
                        Serial.printf(F("ERROR (LedArrayInterface::setChannel): Invalid channel.%s"), SERIAL_LINE_ENDING);
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
                Serial.print(color_channel_index);
                Serial.print(SERIAL_LINE_ENDING);
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
                Serial.printf(F("ERROR (LedArrayInterface::setChannelFast): invalid channel.%s"), SERIAL_LINE_ENDING);
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
                Serial.printf(F(")%s"),SERIAL_LINE_ENDING);
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
                                        Serial.print(value);
                                        Serial.print(SERIAL_LINE_ENDING);
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
                                Serial.print(pin_numbers[led_index]);
                                Serial.print(SERIAL_LINE_ENDING);
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
                                Serial.print(value);
                                Serial.print(SERIAL_LINE_ENDING);
                        }
                        if (!digital_mode)
                                pinMode(pin_numbers[led_number], OUTPUT);
                        digitalWriteFast(pin_numbers[led_number], value);
                        led_values[led_number] = value;
                }
                else
                        Serial.printf(F("ERROR (LedArrayInterface::setLedFast, single led): Invalid LED number. %s"), SERIAL_LINE_ENDING);
        }
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
        for (uint16_t led_index = 0; led_index < 4; led_index++)
        {
                // Get channel number
                int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));

                // Update
                digitalWriteFast(pin_numbers[channel_number], led_values[led_index]);
//    analogWrite(pin_numbers[channel_number], led_values[led_index]);
        }
}

void LedArrayInterface::clear()
{
        digital_mode = false; // ensure pin mode gets configured
        setLedFast(-1, -1, false);
        //  for (uint16_t led_index = 0; led_index < 4; led_index++)
        //    setLed(led_index, 0, false);
        //  update();
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
                                        Serial.printf(F("ERROR (LedArrayInterface::setLed): Invalid led number. %s"), SERIAL_LINE_ENDING);
                        }
                }
                else if (led_number < LedArrayInterface::led_count)
                {
                        int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));

                        if (channel_number >= 0)
                                led_values[led_number] = value;
                        else
                                Serial.printf(F("ERROR (LedArrayInterface::setLed): Invalid led number. %s"), SERIAL_LINE_ENDING);
                }
                else
                        Serial.printf(F("ERROR (LedArrayInterface::setLed): Invalid led number.%s"), SERIAL_LINE_ENDING);
        }
        else
        {
                Serial.print(F("ERROR (LedArrayInterface::setLed): Invalid color channel "));
                Serial.print(color_channel_index);
                Serial.print(SERIAL_LINE_ENDING);
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
        for (uint16_t led_index = 0; led_index < 4; led_index++)
        {
                int16_t channel = (int16_t)pgm_read_word(&(led_positions[led_index][1]));
                pinMode(pin_numbers[channel], OUTPUT);
                Serial.print(pin_numbers[channel]);
                Serial.print(SERIAL_LINE_ENDING);
        }

        // Adjust PWM timer for 8-bit precision
        analogWriteResolution(8);

        // Output trigger Pins
        for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
        {
                pinMode(trigger_output_pin_list[trigger_index], OUTPUT);
                digitalWriteFast(trigger_output_pin_list[trigger_index], LOW);
        }

        // Input trigger Pins
        for (int trigger_index = 0; trigger_index < trigger_input_count; trigger_index++)
                pinMode(trigger_input_pin_list[trigger_index], INPUT);

        // Clear the array
        clear();
}

#endif

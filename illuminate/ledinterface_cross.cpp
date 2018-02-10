/*
Copyright (c) 2018, Zachary Phillips (UC Berkeley)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ledinterface_cross.h"

// Define static members

// Triggering Variables
int LedArrayInterface::trigger_output_pin_list[TRIGGER_OUTPUT_COUNT];
int LedArrayInterface::trigger_input_pin_list[TRIGGER_INPUT_COUNT];

// Trigger state Variables
volatile bool LedArrayInterface::trigger_input_state[TRIGGER_INPUT_COUNT];
volatile bool LedArrayInterface::debug = true;

void LedArrayInterface::setAllLedsFast(bool value)
{
  for (uint16_t led_index = 0; led_index < 4; led_index++)
  {
    // Get channel number
    int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_index][1]));

    // Update
    digitalWriteFast(pin_numbers[channel_number], value);
  }
}

void LedArrayInterface::setLedFast(uint16_t led_number, bool value)
{
  // Get channel number
  int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_number][1]));

  // Update
  digitalWriteFast(pin_numbers[channel_number], value);
}

// Debug Variables
bool LedArrayInterface::getDebug()
{
  return (LedArrayInterface::debug);
}

void LedArrayInterface::setDebug(bool state)
{
  debug = state;
}

void LedArrayInterface::getTriggerPins(int * * pin_numbers)
{
  // format is #pins, pin 1, pin 2, etc..
  int output_trigger_pins[TRIGGER_OUTPUT_COUNT + 1];
  output_trigger_pins[0] = TRIGGER_OUTPUT_COUNT;
  for (int index = 0; index < TRIGGER_OUTPUT_COUNT; index++)
    output_trigger_pins[index + 1] = trigger_output_pin_list[index];

  // format is #pins, pin 1, pin 2, etc..
  int input_trigger_pins[TRIGGER_INPUT_COUNT + 1];
  input_trigger_pins[0] = TRIGGER_INPUT_COUNT;
  for (int index = 0; index < TRIGGER_INPUT_COUNT; index++)
    input_trigger_pins[index + 1] = trigger_input_pin_list[index];

  pin_numbers[0] = input_trigger_pins;
  pin_numbers[1] = output_trigger_pins;
}

void LedArrayInterface::triggerInputChange_0()
{
  cli();
  trigger_input_state[0] = digitalRead(trigger_input_pin_list[0]);
  if (true)
  {
    Serial.print("Channel 0 trigger change detected - state: ");
    Serial.println(trigger_input_state[0]);
  }
  sei();
}

void LedArrayInterface::triggerInputChange_1()
{
  cli();
  trigger_input_state[1] = digitalRead(trigger_input_pin_list[1]);
  if (true)
  {
    Serial.print("Channel 1 trigger change detected - state: ");
    Serial.println(trigger_input_state[1]);
  }
  sei();
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
  for (uint16_t led_index = 0; led_index < 4; led_index++)
  {
    // Get channel number
    int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_index][1]));

    // Update
    analogWrite(pin_numbers[channel_number], led_values[led_index]);
  }
}

void LedArrayInterface::clear()
{
  for (uint16_t led_index = 0; led_index < 4; led_index++)
    led_values[led_index] = 0;
  update();
}

void LedArrayInterface::setAllLeds(uint8_t value)
{
  for (uint16_t led_index = 0; led_index < led_count; led_index++)
    led_values[led_index] = value;
}

void LedArrayInterface::setLed(uint16_t led_number, uint8_t value)
{
  led_values[led_number] = value;
}

void LedArrayInterface::deviceSetup()
{
  // Now set the GSCK to an output and a 50% PWM duty-cycle
  // For simplicity all three grayscale clocks are tied to the same pin
  for (uint16_t led_index = 0; led_index < 4; led_index++)
  {
    pinMode(pin_numbers[led_index], OUTPUT);
    //analogWriteFrequency(pin_numbers[led_index], 375000);
  }

  // Adjust PWM timer for 8-bit precision
  analogWriteResolution(8);


  // Set up trigger pins
  trigger_output_pin_list[0] = TRIGGER_OUTPUT_PIN_0;
  trigger_output_pin_list[1] = TRIGGER_OUTPUT_PIN_1;

  trigger_input_pin_list[0] = TRIGGER_INPUT_PIN_0;
  trigger_input_pin_list[1] = TRIGGER_INPUT_PIN_1;

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

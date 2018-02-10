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
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ledinterface_quasidome.h"

// Define static members

// Triggering Variables
int LedArrayInterface::trigger_output_pin_list[TRIGGER_OUTPUT_COUNT];
int LedArrayInterface::trigger_input_pin_list[TRIGGER_INPUT_COUNT];

// Trigger state Variables
volatile bool LedArrayInterface::trigger_input_state[TRIGGER_INPUT_COUNT];
volatile bool LedArrayInterface::debug = true;

void notImplemented(const char * command_name)
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
  }
}

void LedArrayInterface::setAllLedsFast(bool value)
{
  notImplemented("setAllLedsFast");
}

void LedArrayInterface::setLedFast(uint16_t led_number, bool value)
{
  notImplemented("setLedFast");
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
  tlc.updateLeds();
}

void LedArrayInterface::clear()
{
  tlc.setAllLed(0);
  tlc.updateLeds();
}

void LedArrayInterface::setAllLeds(uint16_t value_r, uint16_t value_g, uint16_t value_b)
{
  tlc.setAllLed(value_r, value_g, value_b);
}

void LedArrayInterface::setAllLeds(uint8_t value_r, uint8_t value_g, uint8_t value_b)
{
 setAllLed((uint16_t) (value_r * UINT16_MAX / UINT8_MAX), (uint16_t) (value_b * UINT16_MAX / UINT8_MAX), (uint16_t) (value_g * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setAllLeds(bool value_r, bool value_g, bool value_b)
{
  tlc.setAllLed( (uint16_t) value_r * UINT16_MAX,  (uint16_t) value_g * UINT16_MAX,  (uint16_t) value_b * UINT16_MAX);
}

void LedArrayInterface::setAllLeds(uint16_t value)
{
  tlc.setAllLed(value);
}

void LedArrayInterface::setLed(uint16_t led_number, uint16_t value_r, uint16_t value_g, uint16_t value_b)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_number - 1][1]));
  if (channel_number >= 0)
    tlc.setLed(channel_number, value_r, value_g, value_b);
}

void LedArrayInterface::setLed(uint16_t led_number, uint8_t value_r, uint8_t value_g, uint8_t value_b)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_number - 1][1]));
  if (channel_number >= 0)
    setLed(channel_number, (uint16_t) (value_r * UINT16_MAX / UINT8_MAX), (uint16_t) (value_b * UINT16_MAX / UINT8_MAX), (uint16_t) (value_g * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setLed(uint16_t led_number, bool value_r, bool value_g, bool value_b)
{
  setLed(led_number, (uint16_t) (value_r * UINT16_MAX), (uint16_t) (value_b * UINT16_MAX), (uint16_t) (value_g * UINT16_MAX));
}

void LedArrayInterface::setLed(uint16_t led_number, uint16_t value)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(ledMap[led_number - 1][1]));
  if (channel_number >= 0)
    tlc.setLed(channel_number, value);
}
void LedArrayInterface::deviceSetup()
{
  // Now set the GSCK to an output and a 50% PWM duty-cycle
  // For simplicity all three grayscale clocks are tied to the same pin
  pinMode(GSCLK, OUTPUT);
  pinMode(LAT, OUTPUT);

  // Adjust PWM timer for maximum GSCLK frequency (5 MHz)
  analogWriteFrequency(GSCLK, 5000000);
  analogWriteResolution(1);
  analogWrite(GSCLK, 1);

  // The library does not ininiate SPI for you, so as to prevent issues with other SPI libraries
  SPI.setMOSI(SPI_MOSI);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);


  tlc.init(LAT, SPI_MOSI, SPI_CLK);

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

  // Provide corrections for LEDs which have issues
  tlc.setRgbPinOrder(2, 1, 0);

  // Update the GS register (ideally LEDs should be dark up to here)
  tlc.setAllLed(0);
  tlc.updateLeds();

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

  // Correct LED pins
  tlc.setRgbPinOrderSingle(79, 0, 1, 2); // channel 79 has B/R swapped.
  tlc.setRgbPinOrderSingle(80, 0, 1, 2); // channel 80 has B/R swapped.
  tlc.setRgbPinOrderSingle(82, 0, 1, 2); // channel 82 has B/R swapped.
  tlc.setRgbPinOrderSingle(83, 0, 1, 2); // channel 83 has B/R swapped.
  tlc.setRgbPinOrderSingle(84, 0, 1, 2); // channel 84 has B/R swapped.
  tlc.setRgbPinOrderSingle(85, 0, 1, 2); // channel 85 has B/R swapped.
  tlc.setRgbPinOrderSingle(86, 0, 1, 2); // channel 86 has B/R swapped.
  tlc.setRgbPinOrderSingle(87, 0, 1, 2); // channel 87 has B/R swapped.
  tlc.setRgbPinOrderSingle(88, 0, 1, 2); // channel 88 has B/R swapped.
  tlc.setRgbPinOrderSingle(89, 0, 1, 2); // channel 89 has B/R swapped.
  tlc.setRgbPinOrderSingle(90, 0, 1, 2); // channel 90 has B/R swapped.
  tlc.setRgbPinOrderSingle(91, 0, 1, 2); // channel 91 has B/R swapped.
  tlc.setRgbPinOrderSingle(92, 0, 1, 2); // channel 92 has B/R swapped.
  tlc.setRgbPinOrderSingle(93, 0, 1, 2); // channel 93 has B/R swapped.
  tlc.setRgbPinOrderSingle(94, 0, 1, 2); // channel 94 has B/R swapped.
  tlc.setRgbPinOrderSingle(95, 0, 1, 2); // channel 95 has B/R swapped.
  tlc.setRgbPinOrderSingle(96, 0, 1, 2); // channel 96 has B/R swapped.
  tlc.setRgbPinOrderSingle(97, 0, 1, 2); // channel 97 has B/R swapped.
  tlc.setRgbPinOrderSingle(98, 0, 1, 2); // channel 98 has B/R swapped.
  tlc.setRgbPinOrderSingle(99, 0, 1, 2); // channel 99 has B/R swapped.
  tlc.setRgbPinOrderSingle(100, 0, 1, 2); // channel 100 has B/R swapped.
  tlc.setRgbPinOrderSingle(101, 0, 1, 2); // channel 101 has B/R swapped.
  tlc.setRgbPinOrderSingle(102, 0, 1, 2); // channel 102 has B/R swapped.
  tlc.setRgbPinOrderSingle(103, 0, 1, 2); // channel 103 has B/R swapped.
  tlc.setRgbPinOrderSingle(104, 0, 1, 2); // channel 104 has B/R swapped.
  tlc.setRgbPinOrderSingle(105, 0, 1, 2); // channel 105 has B/R swapped.
  tlc.setRgbPinOrderSingle(106, 0, 1, 2); // channel 106 has B/R swapped.
  tlc.setRgbPinOrderSingle(107, 0, 1, 2); // channel 107 has B/R swapped.
  tlc.setRgbPinOrderSingle(108, 0, 1, 2); // channel 108 has B/R swapped.
  tlc.setRgbPinOrderSingle(109, 0, 1, 2); // channel 109 has B/R swapped.
  tlc.setRgbPinOrderSingle(110, 0, 1, 2); // channel 110 has B/R swapped.
  tlc.setRgbPinOrderSingle(111, 0, 1, 2); // channel 111 has B/R swapped.
  tlc.setRgbPinOrderSingle(116, 0, 1, 2); // channel 116 has B/R swapped.
  tlc.setRgbPinOrderSingle(208, 0, 1, 2); // channel 208 has B/R swapped.
  tlc.setRgbPinOrderSingle(210, 0, 1, 2); // channel 210 has B/R swapped.
  tlc.setRgbPinOrderSingle(211, 0, 1, 2); // channel 211 has B/R swapped.
  tlc.setRgbPinOrderSingle(212, 0, 1, 2); // channel 212 has B/R swapped.
  tlc.setRgbPinOrderSingle(213, 0, 1, 2); // channel 213 has B/R swapped.
  tlc.setRgbPinOrderSingle(214, 0, 1, 2); // channel 214 has B/R swapped.
  tlc.setRgbPinOrderSingle(215, 0, 1, 2); // channel 215 has B/R swapped.
  tlc.setRgbPinOrderSingle(216, 0, 1, 2); // channel 216 has B/R swapped.
  tlc.setRgbPinOrderSingle(217, 0, 1, 2); // channel 217 has B/R swapped.
  tlc.setRgbPinOrderSingle(218, 0, 1, 2); // channel 218 has B/R swapped.
  tlc.setRgbPinOrderSingle(219, 0, 1, 2); // channel 219 has B/R swapped.
  tlc.setRgbPinOrderSingle(220, 0, 1, 2); // channel 220 has B/R swapped.
  tlc.setRgbPinOrderSingle(221, 0, 1, 2); // channel 221 has B/R swapped.
  tlc.setRgbPinOrderSingle(222, 0, 1, 2); // channel 222 has B/R swapped.
  tlc.setRgbPinOrderSingle(223, 0, 1, 2); // channel 223 has B/R swapped.
  tlc.setRgbPinOrderSingle(224, 0, 1, 2); // channel 224 has B/R swapped.
  tlc.setRgbPinOrderSingle(225, 0, 1, 2); // channel 225 has B/R swapped.
  tlc.setRgbPinOrderSingle(226, 0, 1, 2); // channel 226 has B/R swapped.
  tlc.setRgbPinOrderSingle(227, 0, 1, 2); // channel 227 has B/R swapped.
  tlc.setRgbPinOrderSingle(228, 0, 1, 2); // channel 228 has B/R swapped.
  tlc.setRgbPinOrderSingle(229, 0, 1, 2); // channel 229 has B/R swapped.
  tlc.setRgbPinOrderSingle(230, 0, 1, 2); // channel 230 has B/R swapped.
  tlc.setRgbPinOrderSingle(231, 0, 1, 2); // channel 231 has B/R swapped.
  tlc.setRgbPinOrderSingle(232, 0, 1, 2); // channel 232 has B/R swapped.
  tlc.setRgbPinOrderSingle(233, 0, 1, 2); // channel 233 has B/R swapped.
  tlc.setRgbPinOrderSingle(234, 0, 1, 2); // channel 234 has B/R swapped.
  tlc.setRgbPinOrderSingle(235, 0, 1, 2); // channel 235 has B/R swapped.
  tlc.setRgbPinOrderSingle(236, 0, 1, 2); // channel 236 has B/R swapped.
  tlc.setRgbPinOrderSingle(237, 0, 1, 2); // channel 237 has B/R swapped.
  tlc.setRgbPinOrderSingle(238, 0, 1, 2); // channel 238 has B/R swapped.
  tlc.setRgbPinOrderSingle(239, 0, 1, 2); // channel 239 has B/R swapped.
  tlc.setRgbPinOrderSingle(244, 0, 1, 2); // channel 244 has B/R swapped.
  tlc.setRgbPinOrderSingle(336, 0, 1, 2); // channel 336 has B/R swapped.
  tlc.setRgbPinOrderSingle(338, 0, 1, 2); // channel 338 has B/R swapped.
  tlc.setRgbPinOrderSingle(339, 0, 1, 2); // channel 339 has B/R swapped.
  tlc.setRgbPinOrderSingle(340, 0, 1, 2); // channel 340 has B/R swapped.
  tlc.setRgbPinOrderSingle(341, 0, 1, 2); // channel 341 has B/R swapped.
  tlc.setRgbPinOrderSingle(342, 0, 1, 2); // channel 342 has B/R swapped.
  tlc.setRgbPinOrderSingle(343, 0, 1, 2); // channel 343 has B/R swapped.
  tlc.setRgbPinOrderSingle(344, 0, 1, 2); // channel 344 has B/R swapped.
  tlc.setRgbPinOrderSingle(345, 0, 1, 2); // channel 345 has B/R swapped.
  tlc.setRgbPinOrderSingle(346, 0, 1, 2); // channel 346 has B/R swapped.
  tlc.setRgbPinOrderSingle(347, 0, 1, 2); // channel 347 has B/R swapped.
  tlc.setRgbPinOrderSingle(348, 0, 1, 2); // channel 348 has B/R swapped.
  tlc.setRgbPinOrderSingle(349, 0, 1, 2); // channel 349 has B/R swapped.
  tlc.setRgbPinOrderSingle(350, 0, 1, 2); // channel 350 has B/R swapped.
  tlc.setRgbPinOrderSingle(351, 0, 1, 2); // channel 351 has B/R swapped.
  tlc.setRgbPinOrderSingle(352, 0, 1, 2); // channel 352 has B/R swapped.
  tlc.setRgbPinOrderSingle(353, 0, 1, 2); // channel 353 has B/R swapped.
  tlc.setRgbPinOrderSingle(354, 0, 1, 2); // channel 354 has B/R swapped.
  tlc.setRgbPinOrderSingle(355, 0, 1, 2); // channel 355 has B/R swapped.
  tlc.setRgbPinOrderSingle(356, 0, 1, 2); // channel 356 has B/R swapped.
  tlc.setRgbPinOrderSingle(357, 0, 1, 2); // channel 357 has B/R swapped.
  tlc.setRgbPinOrderSingle(358, 0, 1, 2); // channel 358 has B/R swapped.
  tlc.setRgbPinOrderSingle(359, 0, 1, 2); // channel 359 has B/R swapped.
  tlc.setRgbPinOrderSingle(360, 0, 1, 2); // channel 360 has B/R swapped.
  tlc.setRgbPinOrderSingle(361, 0, 1, 2); // channel 361 has B/R swapped.
  tlc.setRgbPinOrderSingle(362, 0, 1, 2); // channel 362 has B/R swapped.
  tlc.setRgbPinOrderSingle(363, 0, 1, 2); // channel 363 has B/R swapped.
  tlc.setRgbPinOrderSingle(364, 0, 1, 2); // channel 364 has B/R swapped.
  tlc.setRgbPinOrderSingle(365, 0, 1, 2); // channel 365 has B/R swapped.
  tlc.setRgbPinOrderSingle(366, 0, 1, 2); // channel 366 has B/R swapped.
  tlc.setRgbPinOrderSingle(367, 0, 1, 2); // channel 367 has B/R swapped.
  tlc.setRgbPinOrderSingle(372, 0, 1, 2); // channel 372 has B/R swapped.
  tlc.setRgbPinOrderSingle(464, 0, 1, 2); // channel 464 has B/R swapped.
  tlc.setRgbPinOrderSingle(466, 0, 1, 2); // channel 466 has B/R swapped.
  tlc.setRgbPinOrderSingle(467, 0, 1, 2); // channel 467 has B/R swapped.
  tlc.setRgbPinOrderSingle(468, 0, 1, 2); // channel 468 has B/R swapped.
  tlc.setRgbPinOrderSingle(469, 0, 1, 2); // channel 469 has B/R swapped.
  tlc.setRgbPinOrderSingle(470, 0, 1, 2); // channel 470 has B/R swapped.
  tlc.setRgbPinOrderSingle(471, 0, 1, 2); // channel 471 has B/R swapped.
  tlc.setRgbPinOrderSingle(472, 0, 1, 2); // channel 472 has B/R swapped.
  tlc.setRgbPinOrderSingle(473, 0, 1, 2); // channel 473 has B/R swapped.
  tlc.setRgbPinOrderSingle(474, 0, 1, 2); // channel 474 has B/R swapped.
  tlc.setRgbPinOrderSingle(475, 0, 1, 2); // channel 475 has B/R swapped.
  tlc.setRgbPinOrderSingle(476, 0, 1, 2); // channel 476 has B/R swapped.
  tlc.setRgbPinOrderSingle(477, 0, 1, 2); // channel 477 has B/R swapped.
  tlc.setRgbPinOrderSingle(478, 0, 1, 2); // channel 478 has B/R swapped.
  tlc.setRgbPinOrderSingle(479, 0, 1, 2); // channel 479 has B/R swapped.
  tlc.setRgbPinOrderSingle(480, 0, 1, 2); // channel 480 has B/R swapped.
  tlc.setRgbPinOrderSingle(481, 0, 1, 2); // channel 481 has B/R swapped.
  tlc.setRgbPinOrderSingle(482, 0, 1, 2); // channel 482 has B/R swapped.
  tlc.setRgbPinOrderSingle(483, 0, 1, 2); // channel 483 has B/R swapped.
  tlc.setRgbPinOrderSingle(484, 0, 1, 2); // channel 484 has B/R swapped.
  tlc.setRgbPinOrderSingle(485, 0, 1, 2); // channel 485 has B/R swapped.
  tlc.setRgbPinOrderSingle(486, 0, 1, 2); // channel 486 has B/R swapped.
  tlc.setRgbPinOrderSingle(487, 0, 1, 2); // channel 487 has B/R swapped.
  tlc.setRgbPinOrderSingle(488, 0, 1, 2); // channel 488 has B/R swapped.
  tlc.setRgbPinOrderSingle(489, 0, 1, 2); // channel 489 has B/R swapped.
  tlc.setRgbPinOrderSingle(490, 0, 1, 2); // channel 490 has B/R swapped.
  tlc.setRgbPinOrderSingle(491, 0, 1, 2); // channel 491 has B/R swapped.
  tlc.setRgbPinOrderSingle(492, 0, 1, 2); // channel 492 has B/R swapped.
  tlc.setRgbPinOrderSingle(493, 0, 1, 2); // channel 493 has B/R swapped.
  tlc.setRgbPinOrderSingle(494, 0, 1, 2); // channel 494 has B/R swapped.
  tlc.setRgbPinOrderSingle(495, 0, 1, 2); // channel 495 has B/R swapped.
  tlc.setRgbPinOrderSingle(500, 0, 1, 2); // channel 500 has B/R swapped.
  tlc.setRgbPinOrderSingle(585, 0, 1, 2); // channel 79 has B/R swapped.
}


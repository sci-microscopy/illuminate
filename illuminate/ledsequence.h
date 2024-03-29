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

#ifndef LEDSEQUENCE_H
#define LEDSEQUENCE_H

#include "Arduino.h"
#include "illuminate.h"
#include "ledarrayinterface.h"
#include "constants.h"

// Define LED Sequence Object
struct LedSequence
{
  uint16_t length = 0;                      // Length of values
  volatile uint16_t * led_counts;           // Number of LEDs in each values
  volatile uint16_t * * led_list;           // LED numbers used in each entry
  volatile uint16_t number_of_patterns_assigned = 0; // Number of patterns which have been assigned
  volatile uint16_t current_pattern_led_index = 0;   // Current led index within current pattern
  uint8_t color_channel_count = 1;
  uint8_t bit_depth = 8;
  int debug = 1;

  void reset()
  {
    deallocate();
  }

  void allocate(uint16_t values_length)
  {
    led_list = new volatile uint16_t * [values_length];
    led_counts = new volatile uint16_t [values_length];

    // Assign new vector length
    length = values_length;
  }

  void append(uint16_t led_number)
  {
    // Assign led number
    led_list[number_of_patterns_assigned - 1][current_pattern_led_index] = led_number;

    // Increment number of LEDs stored in this pattern
    current_pattern_led_index++;
  }

  bool increment(uint16_t led_count)
  {
    if (number_of_patterns_assigned < length)
    {
      // increment number of patterns assigned
      number_of_patterns_assigned++;

      // store the number of leds in this pattern
      led_counts[number_of_patterns_assigned - 1] = led_count;

      if (led_count > 0)
        led_list[number_of_patterns_assigned - 1] = new uint16_t[led_count];

      // Reset the pattern led index
      current_pattern_led_index = 0;

      // Let user know we haven't reached capacity
      return true;
    }
    else
    {
      Serial.print(F("Sequence length (")); Serial.print(length); Serial.printf(F(") reached. %s"), SERIAL_LINE_ENDING);
      return false;
    }
  }

  void deallocate()
  {
    for (uint16_t values_index = 0; values_index < number_of_patterns_assigned; values_index++)
      if (led_counts[values_index] > 0)
        delete[] led_list[values_index];

    if (number_of_patterns_assigned > 0)
    {
      delete[] led_list;
      delete[] led_counts;
    }

    number_of_patterns_assigned = 0;
    current_pattern_led_index = 0;
  }

  void print(int command_mode)
  {
    // Print header
    if (command_mode == COMMAND_MODE_LONG)
      Serial.printf(F("Sequence has %d patterns:%s"), length, SERIAL_LINE_ENDING);
    else
      Serial.printf(F("{\n  \"sequence\": [%s"), SERIAL_LINE_ENDING);

    // Print values
    for (uint16_t pattern_index = 0; pattern_index < number_of_patterns_assigned; pattern_index++)
      print(pattern_index, command_mode);

    // Print footer
    if (command_mode == COMMAND_MODE_SHORT)
      Serial.printf(F(" ]%s}%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);
  }

  void print(uint16_t pattern_index, int command_mode)
  {
    if (command_mode == COMMAND_MODE_LONG)
    {
      Serial.print("Pattern ");
      Serial.print(pattern_index);
      Serial.print(" (");
      Serial.print(led_counts[pattern_index]);
      Serial.printf(" leds):");
      for (uint16_t led_index = 0; led_index < led_counts[pattern_index]; led_index++)
      {
        Serial.print(F(" "));
        Serial.print(led_list[pattern_index][led_index]);
        if (led_index < led_counts[pattern_index] - 1)
          Serial.printf(F(","));
      }
      Serial.print(SERIAL_LINE_ENDING);
    }
    else
    {
      Serial.print("[");
      for (uint16_t led_index = 0; led_index < led_counts[pattern_index]; led_index++)
      {
        Serial.print(led_list[pattern_index][led_index]);
        if (led_index < led_counts[pattern_index] - 1)
          Serial.printf(F(","), SERIAL_LINE_ENDING);
      }
      if (pattern_index < number_of_patterns_assigned - 1)
        Serial.printf(F("],%s"), SERIAL_LINE_ENDING);
      else
        Serial.printf(F("]%s"), SERIAL_LINE_ENDING);
    }
  }
};

#endif

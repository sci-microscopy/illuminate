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

#ifndef LED_ARRAY_SEQUENCE_H
#define LED_ARRAY_SEQUENCE_H
#include "Arduino.h"
#include "illuminate.h"

// Define LED Sequence Object
struct LedSequence
{
  uint16_t length = 0;                      // Length of values
  volatile uint16_t * led_counts;           // Number of LEDs in each values
  volatile uint16_t * * led_list;           // LED numbers used in each entry
  volatile uint8_t * * values;                     // Actual LED values (will be assigned to one of the other variables
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
    // Allocate sequence with desired bit_depth
    if (bit_depth == 1)
      ; // Don't allocate anything - any LED in the LED list is considered "on" or true.
    else if (bit_depth == 8)
      values = new volatile uint8_t * [values_length];
    else if (bit_depth == 16)
      Serial.printf(F("16bit sequences not yet supported %s"), SERIAL_LINE_ENDING);
    else
    {
      Serial.printf(F("ERROR - invalid bit depth!%s"),SERIAL_LINE_ENDING);
      return;
    }

    led_list = new volatile uint16_t * [values_length];
    led_counts = new volatile uint16_t [values_length];

    // Assign new vector length
    length = values_length;
  }

  void setBitDepth(uint16_t new_bit_depth)
  {
    // Deallocate arrays with old bit depth
    deallocate();

    // Set new bit depth
    if ((new_bit_depth == 1) || (new_bit_depth == 8))
      bit_depth = new_bit_depth;
    else
      Serial.printf(F("ERROR - invalid bit depth! (allowed values are 1 or 8) %s"), SERIAL_LINE_ENDING);

    // Allocate new arrays with new bit depth (and same length as before)
    allocate(length);
  }

  void append(uint16_t led_number, uint8_t value)
  {
    if (bit_depth == 8)
      // Assign value (only if 8-bit)
      values[number_of_patterns_assigned - 1][current_pattern_led_index] = value;

    // Assign led number
    led_list[number_of_patterns_assigned - 1][current_pattern_led_index] = led_number;

    // Increment number of LEDs stored in this pattern
    current_pattern_led_index += 1;
  }

  bool incriment(uint16_t led_count)
  {
    if (number_of_patterns_assigned < length)
    {
      // Incriment number of patterns assigned
      number_of_patterns_assigned++;

      // store the number of leds in this pattern
      led_counts[number_of_patterns_assigned - 1] = led_count;

      if (led_count > 0)
      {
        if (bit_depth == 8)
          // Initialize the array for this pattern (only if 8-bit)
          values[number_of_patterns_assigned - 1] = new uint8_t[led_count * (uint16_t)color_channel_count];

        // Initialize the led number list for this pattern
        led_list[number_of_patterns_assigned - 1] = new uint16_t[led_count];
      }

      // Reset the pattern led index
      current_pattern_led_index = 0;

      // Let user know we haven't reached capacity
      return (true);
    }
    else
    {
      Serial.print(F("Sequence length (")); Serial.print(length); Serial.printf(F(") reached. %s"), SERIAL_LINE_ENDING);
      return (false);
    }
  }

  void deallocate()
  {
    for (uint16_t values_index = 0; values_index < number_of_patterns_assigned; values_index++)
    {
      if (led_counts[values_index] > 0)
      {
        if (bit_depth == 8)
          delete[] values[values_index];
        delete[] led_list[values_index];
      }
    }

    if (number_of_patterns_assigned > 0)
    {
      if (bit_depth == 8)
        delete[] values;
      delete[] led_list;
      delete[] led_counts;
    }

    number_of_patterns_assigned = 0; // Number of patterns which have been assigned
    current_pattern_led_index = 0;
  }

  void print()
  {
    for (uint16_t values_index = 0; values_index < number_of_patterns_assigned; values_index++)
    {
      print(values_index);
    }
    Serial.print(SERIAL_LINE_ENDING);
  }

  void print(uint16_t values_index)
  {
    Serial.print("Pattern ");
    Serial.print(values_index);
    Serial.print(" (");
    Serial.print(led_counts[values_index]);
    Serial.printf(" leds): %s", SERIAL_LINE_ENDING);
    for (uint16_t led_index = 0; led_index < led_counts[values_index]; led_index++)
    {
      Serial.print(F(" LED #: "));
      Serial.print(led_list[values_index][led_index]);
      Serial.print(F(", value="));
      if (bit_depth == 8)
        Serial.print(values[values_index][led_index]);
      else
        Serial.print(true);
      Serial.print(F(" (bit depth="));
      Serial.print(bit_depth);
      Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
    }
  }
};

#endif

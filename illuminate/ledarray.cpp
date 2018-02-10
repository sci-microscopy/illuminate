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

#include "ledarray.h"

void LedArray::printLedPositions()
{
  Serial.println(F("led_positions_cartesian start"));
  int16_t led_number;
  float x, y, z;
  for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    led_number = (int16_t)pgm_read_word(&(ledMap[led_index][0]));
    x = float((int16_t)pgm_read_word(&(ledMap[led_index][2]))) / 100.0;
    y = float((int16_t)pgm_read_word(&(ledMap[led_index][3]))) / 100.0;
    z = float((int16_t)pgm_read_word(&(ledMap[led_index][4]))) / 100.0;
    z = z - float((int16_t)pgm_read_word(&(ledMap[0][4]))) / 100.0 + led_array_interface_distance_z;

    Serial.print(F("#"));
    Serial.printf(F("%04d"), led_number);
    Serial.print(F(":"));
    Serial.printf(F("%02.02f"), x);
    Serial.print(F(","));
    Serial.printf(F("%02.02f"), y);
    Serial.print(F(","));
    Serial.printf(F("%02.02f\n"), z);
  }
  Serial.println(F("led_positions_cartesian end"));
}

void LedArray::printDeviceName()
{
  Serial.println(led_array_interface->device_name);
}

void LedArray::printCurrentLedValues()
{
  Serial.println(F("led_values start"));
  for (uint16_t led_number = 0; led_number < led_array_interface->led_count; led_number++)
  {
    Serial.print(F("#"));
    Serial.printf(F("%04d"), led_number);
    Serial.print(F(":"));
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    {
      if (led_array_interface->bit_depth == 16)
        Serial.printf(F("%05lu"), led_array_interface->getLedValue(led_number, color_channel_index));
      else if (led_array_interface->bit_depth == 8)
        Serial.printf(F("%03u"), led_array_interface->getLedValue(led_number, color_channel_index));
      else if (led_array_interface->bit_depth == 1)
        Serial.printf(F("%01u"), led_array_interface->getLedValue(led_number, color_channel_index));

      if (color_channel_index < (led_array_interface->color_channel_count - 1))
        Serial.print(',');
      else
        Serial.print('\n');
    }
    Serial.println(F("led_values end"));
  }
}

void LedArray::showVersion()
{
  Serial.println(version);
}

void LedArray::notImplemented(const char * command_name)
{
  Serial.print(F("Command "));
  Serial.print(command_name);
  Serial.println(F(" is not implemented for this device."));
}

void LedArray::printAbout()
{
  Serial.print("=== ");
  Serial.print(led_array_interface->device_name);
  Serial.println(F(" LED Array Controller"));
  Serial.print(F("=== HW Version: r"));
  Serial.print(led_array_interface->device_hardware_revision);
  Serial.print(F(", Controller Version: r"));
  Serial.print(version);
  Serial.println(F(")\n=== For help, type ? "));
}

void LedArray::printSystemParams()
{
  Serial.println(F("{"));
  Serial.print(F("    'device_name' : '"));
  Serial.print(led_array_interface->device_name);
  Serial.print(F("',\n    'led_count' : "));
  Serial.print(led_array_interface->led_count);
  Serial.print(F(",\n    'color_channels' : ["));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(","));
    Serial.print('\'');
    Serial.print(led_array_interface->color_channel_names[channel_index]);
    Serial.print('\'');
  }
  Serial.print(F("]"));
  Serial.print(F(",\n    'color_channel_center_wavelengths' : {"));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(","));
    Serial.print('\'');
    Serial.print(led_array_interface->color_channel_names[channel_index]);
    Serial.print('\'');
    Serial.printf(" : %.3f", led_array_interface->color_channel_center_wavelengths[channel_index]);
  }
  Serial.print(F("},\n    'trigger_input_count' : "));
  Serial.print(led_array_interface->trigger_input_count);
  Serial.print(F(",\n    'trigger_output_count' : "));
  Serial.print(led_array_interface->trigger_output_count);
  Serial.print(F(",\n    'bit_depth' : "));
  Serial.print(led_array_interface->bit_depth);
  Serial.print(F(",\n    'serial_number' : "));
  Serial.print(led_array_interface->serial_number);

  // Terminate JSON
  Serial.println("\n}");
}

void LedArray::resetArray()
{
  Serial.println(F("Resetting Array"));
  led_array_interface->clear();
  led_array_interface->deviceSetup();
}

void LedArray::drawDiscoPattern(uint16_t nLed)
{
  if (nLed <= 0)
    nLed = 10;
  while (Serial.available() == 0)
  {
    led_array_interface->setAllLeds(0);
    for (uint16_t led_index = 0; led_index < nLed; led_index++)
    {
      led_index = random(1, led_array_interface->led_count);
      led_array_interface->setLed(led_index, (uint8_t)random(0, 255));
    }
    led_array_interface->update();
    delay(1);
  }
}

void LedArray::clearNaList()
{
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    delete[] led_position_list_na[led_index];
  delete[] led_position_list_na;
}

void LedArray::buildNaList(float new_board_distance)
{
  float Na_x, Na_y, Na_d, yz, xz, x, y, z;
  // Initialize new position list
  led_position_list_na = new float * [led_array_interface->led_count];

  if (new_board_distance > 0)
    led_array_interface_distance_z = new_board_distance;

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    led_position_list_na[led_index] = new float[3];
    if ((int16_t)pgm_read_word(&(ledMap[led_index][1])) >= 0)
    {
      x = float((int16_t)pgm_read_word(&(ledMap[led_index][2]))) / 100.0;
      y = float((int16_t)pgm_read_word(&(ledMap[led_index][3]))) / 100.0;
      z = led_array_interface_distance_z;

      yz = sqrt(y * y + z * z);
      xz = sqrt(x * x + z * z);
      Na_x = sin(atan(x / yz));
      Na_y = sin(atan(y / xz));
      Na_d = sqrt(Na_x * Na_x + Na_y * Na_y);

      led_position_list_na[led_index][0] = Na_x;
      led_position_list_na[led_index][1] = Na_y;
      led_position_list_na[led_index][2] = Na_d;
    }
    else
    {
      led_position_list_na[led_index][0] = INVALID_NA; // invalid NA
      led_position_list_na[led_index][1] = INVALID_NA; // invalid NA
      led_position_list_na[led_index][2] = INVALID_NA; // invalid NA
    }
  }
  if (debug)
    Serial.println(F("Finished updating led positions."));
}

void LedArray::fillArray()
{
  led_array_interface->setAllLeds(led_value);
  led_array_interface->update();

  if (debug)
    Serial.println(F("Filled Array"));
}

void LedArray::clearArray()
{
  led_array_interface->setAllLeds(0);
  led_array_interface->update();

}

void LedArray::setNa(int8_t new_na)
{
  if ((new_na > 0) && new_na < 100 * led_array_interface->max_na)
  {
    objective_na = (float)new_na / 100.0;
    if (debug > 0)
    {
      Serial.print(F("New NA set to: "));
      Serial.println(objective_na);
    }
  } else
    Serial.println(F("ERROR - invalid NA! Make sure NA is 100*na"));
}

void LedArray::printTriggerSettings()
{
  // Input Pins
  for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
  {
    Serial.print("Trigger input pin index ");
    Serial.print(trigger_index);
    Serial.print(F(" uses Pin #"));
    Serial.println(led_array_interface->trigger_input_pin_list[trigger_index]);
  }

  // Output Pins
  for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
  {
    Serial.print(F("Trigger output pin index "));
    Serial.print(trigger_index);
    Serial.print(F(" uses Pin #"));
    Serial.print(led_array_interface->trigger_output_pin_list[trigger_index]);
    Serial.print(F(" with pulse width "));
    Serial.print(trigger_pulse_width_list_us[trigger_index]);
    Serial.print(F("us. Start delay is "));
    Serial.print(trigger_start_delay_list_ms[trigger_index]);
    Serial.println(F("ms."));
  }
}

void LedArray::drawDarkfield()
{
  notImplemented("Darkfield");
}

void LedArray::drawCdpcParser(int argc, char * *argv)
{
  notImplemented("cDPC");
}

void LedArray::drawHalfAnnulusParser(int argc, char * *argv)
{
  notImplemented("Half Annulus");
}

void LedArray::drawColorDarkfieldParser(int argc, char * * argv)
{
  notImplemented("Color Darkfield");
}

void LedArray::drawNavdpcParser(int argc, char * * argv)
{
  notImplemented("navDPC");
}

void LedArray::drawAnnulusParser(int argc, char * * argv)
{
  notImplemented("Annulus");
}

void LedArray::drawChannel(uint16_t tmp)
{
  notImplemented("Draw Channel");
}

void LedArray::setPinOrder(int argc, char * *argv)
{
  notImplemented("Set Pin Order");
}

void LedArray::drawPulsePattern(uint16_t tmp)
{
  notImplemented("Draw Pulse");
}

void LedArray::triggerSetup(int argc, char ** argv)
{
  int trigger_index = 0;
  uint16_t trigger_pulse_width_us = 0;
  uint16_t trigger_start_delay_ms = 0;

  if (argc >= 2)
  {
    trigger_index = atoi(argv[0]);
    trigger_pulse_width_us = strtoul(argv[1], NULL, 0);
    if (argc >= 3)
      trigger_start_delay_ms = strtoul(argv[2], NULL, 0);

    if (trigger_pulse_width_us >= 0)
      trigger_pulse_width_list_us[trigger_index] = trigger_pulse_width_us;

    if (trigger_start_delay_ms >= 0)
      trigger_start_delay_list_ms[trigger_index] = trigger_start_delay_ms;

    Serial.print("Trigger ");
    Serial.print(trigger_index);
    Serial.print(" now has a pulse width of ");
    Serial.print(trigger_pulse_width_list_us[trigger_index] );
    Serial.print("us and a start delay of ");
    Serial.print(trigger_start_delay_list_ms[trigger_index]);
    Serial.println(F("ms."));
  }
  else
    Serial.println(F("ERROR: Invalid number of arguments for setTriggerPulse!"));
}

void LedArray::sendTriggerPulse(int trigger_index, bool show_output)
{
  // Send trigger pulse with pulse_width
  int status = led_array_interface->sendTriggerPulse(trigger_index, trigger_pulse_width_list_us[trigger_index], false); // last argument is polarity, hard code for now

  if (status < 0)
    Serial.print(F("ERROR - pin not configured!"));
}

void LedArray::setTriggerState(int trigger_index, bool state, bool show_output)
{
  int status = led_array_interface->setTriggerState(trigger_index, state);
  if (status < 0)
    Serial.print(F("ERROR - pin not configured!"));
}

void LedArray::drawSingleLed(int16_t led_number)
{
  if (led_number < led_array_interface->led_count)
  {
    if (auto_clear_flag)
      led_array_interface->setAllLeds(0);

    led_array_interface->setLed(led_number, led_value);

    led_array_interface->update();

    if (debug)
    {
      Serial.println(F("Drew Led # "));
      Serial.println(led_number);
    }
  }
  else
    Serial.println(F("ERROR - LED not present at this position!"));
}

void LedArray::drawLedList(uint16_t argc, char ** argv)
{
  uint16_t led_number;
  if (auto_clear_flag)
    clearArray();
  for (uint16_t led_index = 0; led_index < argc; led_index++)
  {
    led_number = strtoul(argv[led_index], NULL, 0);
    led_array_interface->setLed(led_number, led_value);
  }
  led_array_interface->update();
}

void LedArray::scanBrightfieldLeds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;

  // Parse inputs
  if (argc == 1)
    delay_ms = strtoul(argv[0], NULL, 0);
  else if (argc <= TRIGGER_OUTPUT_COUNT)
  {
    for (int arg_index = 0; arg_index < argc; arg_index++)
      trigger_output_mode_list[arg_index] = atoi(argv[0]);
  }

  if ((delay_ms >= 0) && (delay_ms < DELAY_MAX))
  {
    float d;

    clearArray();

    for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
    {
      if (trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
        sendTriggerPulse(trigger_index, false);
    }

    Serial.println(F("Starting center LED scan..."));
    for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      d = led_position_list_na[led_index][2];

      if (d < objective_na)
      {
        led_array_interface->setAllLeds(0);
        led_array_interface->setLed(led_index, led_value);
        Serial.print(led_index);
        Serial.print(", ");
        led_array_interface->update();
        for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
        {
          if (trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION)
            sendTriggerPulse(trigger_index, false);
        }
        delay(delay_ms);
      }
    }
    Serial.println(" ");
    Serial.println(F("Done with brightfield LED scan."));
    clearArray();
  }
  else
    Serial.println(F("ERROR - brightfield scan delay too short/long"));
}

void LedArray::scanAllLeds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;
  if (argc == 1)
    delay_ms = strtoul(argv[0], NULL, 0);

  if ((delay_ms >= 0) && (delay_ms < DELAY_MAX))
  {
    clearArray();
    for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
    {
      if (trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
        sendTriggerPulse(trigger_index, false);
    }

    Serial.println(F("Starting full LED scan..."));
    for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      led_array_interface->setAllLeds(0);
      led_array_interface->setLed(led_index, led_value);
      Serial.print(led_index);
      Serial.print(", ");

      for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
      {
        if (trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
          sendTriggerPulse(trigger_index, false);
      }

      led_array_interface->update();
      delay(delay_ms);

    }
    Serial.println(" ");
    Serial.println(F("Done with full LED scan."));
    clearArray();
  }
  else
    Serial.println(F("ERROR - full scan delay too short/long"));
}

void LedArray::setColor(int16_t argc, char ** argv)
{
  // Check to see if we passed in a string

  if (argc == 1)
    led_value = (uint8_t) atoi(argv[0]);
  if (debug)
  {
    Serial.print(F("New led_value set to: "));
    Serial.print(led_value);
  }
}

void LedArray::drawQuadrant(int8_t qNumber)
{
  if (debug)
  {
    Serial.print(F("Drawing Quadrant "));
    Serial.println(qNumber);
  }

  if (auto_clear_flag)
    clearArray();

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    //int16_t x = (int16_t)pgm_read_word(&(ledMap[led_index][2]));
    //int16_t y = (int16_t)pgm_read_word(&(ledMap[led_index][3]));
    //int16_t d = (int16_t)pgm_read_word(&(ledMap[led_index][4]));

    float x = led_position_list_na[led_index][0];
    float y = led_position_list_na[led_index][1];
    float d = led_position_list_na[led_index][2];

    if (  (qNumber == 1 && (x < 0) && (y > 0) && (d <  objective_na))
          || (qNumber == 2 && (x > 0) && (y > 0) && (d < objective_na))
          || (qNumber == 3 && (x > 0) && (y < 0) && (d < objective_na))
          || (qNumber == 4 && (x < 0) && (y < 0) && (d < objective_na)))
    {
      led_array_interface->setLed(led_index, led_value);
    }
  }
  led_array_interface->update();
}

void LedArray::drawHalfCircle(int8_t halfCircleType)
{
  if (debug)
  {
    Serial.print(F("Drawing half circle with type index"));
    Serial.println(halfCircleType);
  }

  if (auto_clear_flag)
    clearArray();

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    //int16_t x = (int16_t)pgm_read_word(&(ledMap[led_index][2]));
    //int16_t y = (int16_t)pgm_read_word(&(ledMap[led_index][3]));
    //int16_t d = (int16_t)pgm_read_word(&(ledMap[led_index][4]));

    float x = led_position_list_na[led_index][0];
    float y = led_position_list_na[led_index][1];
    float d = led_position_list_na[led_index][2];

    if (  (halfCircleType == 1 && (y > 0) && (d < objective_na))
          || (halfCircleType == 2 && (y < 0) && (d < objective_na))
          || (halfCircleType == 3 && (x < 0) && (d < objective_na))
          || (halfCircleType == 4 && (x > 0) && (d < objective_na)))
    {
      led_array_interface->setLed(led_index, led_value);
    }

  }
  led_array_interface->update();
}

void LedArray::drawDpc(char * optionalParam)
{
  int8_t circleType = 0;
  if ( (strcmp(optionalParam, DPC_TOP1) == 0) || (strcmp(optionalParam, DPC_TOP2) == 0))
    circleType = 1;
  else if ( (strcmp(optionalParam, DPC_BOTTOM1) == 0) || (strcmp(optionalParam, DPC_BOTTOM2) == 0))
    circleType = 2;
  else if ( (strcmp(optionalParam, DPC_LEFT1) == 0) || (strcmp(optionalParam, DPC_LEFT2) == 0))
    circleType = 3;
  else if ( (strcmp(optionalParam, DPC_RIGHT1) == 0) || (strcmp(optionalParam, DPC_RIGHT2) == 0))
    circleType = 4;
  else
    Serial.println(F("ERROR - invalid dpc circle type. Options are dpc.t, dpc.b, dpc.l, dpc.r"));

  if (circleType > 0)
    drawHalfCircle(circleType);
}

void LedArray::drawBrightfield()
{
  if (debug)
    Serial.println(F("Drawing brightfield pattern"));

  if (auto_clear_flag)
    clearArray();

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    float d = led_position_list_na[led_index][2];
    if (d < ( objective_na))
      led_array_interface->setLed(led_index, led_value);
  }
  led_array_interface->update();
}

void LedArray::setSequenceLength(uint16_t new_seq_length, bool quiet)
{
  // Reset old sequence
  led_sequence.deallocate();

  // Initalize new sequence
  led_sequence.allocate(new_seq_length);

  if (!quiet)
  {
    Serial.print(F("New sequence length is: "));
    Serial.println(new_seq_length);
  }
}

void LedArray::setSequenceBitDepth(uint8_t bit_depth, bool quiet)
{
  led_sequence.setBitDepth(bit_depth);

  if (!quiet)
  {
    Serial.print(F("New sequence bit depth is: "));
    Serial.println(bit_depth);
  }
}

int LedArray::getSequenceBitDepth()
{
  return led_sequence.bit_depth;
}

void LedArray::setSequenceValue(uint16_t argc, void ** led_values, int16_t * led_numbers)
{
  // Determine the number of LEDs in this pattern
  int16_t pattern_led_count = 0;
  if (led_numbers[1] == -1)
    pattern_led_count = led_array_interface->led_count;
  else if (led_numbers[1] == -2)
    pattern_led_count = 0;
  else
    pattern_led_count = led_numbers[0];

  // Determine number of arguments to process
  int16_t led_argc = led_numbers[0];

  if (led_argc > 0 && (argc == (led_argc * led_array_interface->color_channel_count))) // Color (or white if one channel)
  {
    // Switch to new led pattern
    if (led_sequence.incriment(pattern_led_count) && (pattern_led_count > 0))
    {
      // Assign LED indicies and values
      uint8_t * values = ((uint8_t *) led_values);
      for (int led_argument_index = 0; led_argument_index < led_argc; led_argument_index++)
      {
        // If the led number is -1, append all LEDs to the sequence
        if (led_numbers[led_argument_index + 1] == -1)
        {
          for (int led_number = 0; led_number < led_array_interface->led_count; led_number++)
          {
            if (led_sequence.bit_depth == 8)
              led_sequence.append(led_number, values[led_argument_index * led_array_interface->color_channel_count]);
            else if (led_sequence.bit_depth == 1)
              led_sequence.append(led_number, true);
          }
        }
        else // Normal LED value
        {
          // Append this LED to the sequence
          if (led_sequence.bit_depth == 8)
            led_sequence.append(led_numbers[led_argument_index + 1], values[led_argument_index * led_array_interface->color_channel_count]);
          else if (led_sequence.bit_depth == 1)
            led_sequence.append(led_numbers[led_argument_index + 1], true);
        }
      }
      if (debug >= 0)
        led_sequence.print(led_sequence.number_of_patterns_assigned - 1);
    }
  }
  else if (pattern_led_count == 0)
  {
    // Add blank pattern
    led_sequence.incriment(0);
  }
  else
  {
    Serial.print(F("Error (LedArray::setSequenceValue) - invalid number of arguments (should be divisible by ")); Serial.print(led_array_interface->color_channel_count); Serial.println(F(")"));
  }
}

void LedArray::printSequence()
{
  Serial.print(F("Sequence has ")); Serial.print(led_sequence.length); Serial.print("x "); Serial.print(led_sequence.bit_depth); Serial.println(F(" bit patterns:"));
  led_sequence.print();
}

void LedArray::printSequenceLength()
{
  Serial.println(led_sequence.length);
}

void LedArray::resetSequence()
{
  sequence_number_displayed = 0;
}

void LedArray::waitForTriggerState(int trigger_index, bool state)
{
  float delayed_ms = 0;
  while ((digitalReadFast(led_array_interface->trigger_input_pin_list[trigger_index]) != state))
  {
    delayMicroseconds(1);
    delayed_ms += 0.001;
    if (delayed_ms > trigger_feedback_timeout_ms)
    {
      Serial.print(F("WARNING: Exceeding max delay for trigger input "));
      Serial.println(trigger_index);
      led_array_interface->setAllLeds(0);
      break;
    }
  }
}

void LedArray::triggerInputTest(uint16_t channel)
{
  led_array_interface->setAllLeds(0);
  led_array_interface->update();
  Serial.println(led_array_interface->trigger_input_state[channel]);
  Serial.print("Begin trigger input test for channel "); Serial.println(channel);
  waitForTriggerState(channel, !led_array_interface->trigger_input_state[channel]);
  Serial.print("Passed trigger input test for channel "); Serial.println(channel);
  led_array_interface->setAllLeds(0);
  led_array_interface->setLed(1, (uint8_t)255);
  led_array_interface->update();
}

void LedArray::runSequence(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.println(F("Starting sequence."));

  /* Format for argv:
     0: delay between acquisitions, us/ms
     1: number of times to repeat pattern
     2: trigger output 1 setting
     3: trigger output 2 setting
     4: trigger input 1 setting
     5: trigger input 2 setting
  */

  // Reset Trigger parameters
  trigger_output_mode_list[0] = 0;
  trigger_output_mode_list[1] = 0;
  trigger_input_mode_list[0] = 0;
  trigger_input_mode_list[1] = 0;

  uint16_t delay_ms = 10;
  uint16_t acquisition_count = 1;

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      delay_ms  = strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      acquisition_count  = strtoul(argv[1], NULL, 0);
    else if (argc_index >= 2 && argc_index < 4)
      trigger_output_mode_list[argc_index - 2] = atoi(argv[argc_index]);
    else if (argc_index >= 4 && argc_index < 6)
      trigger_input_mode_list[argc_index - 4] = atoi(argv[argc_index]);
    else
      Serial.println("WARNING:  Ignoring additional argument in runSequence");
  }

  if (debug)
  {
    Serial.println("OPTIONS:");
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  acquisition_count: ");
    Serial.println(acquisition_count);
    Serial.print("  trigger out 0: ");
    Serial.println(trigger_output_mode_list[0]);
    Serial.print("  trigger out 1: ");
    Serial.println(trigger_output_mode_list[1]);
    Serial.print("  trigger in 0: ");
    Serial.println(trigger_input_mode_list[0]);
    Serial.print("  trigger in 1: ");
    Serial.println(trigger_input_mode_list[1]);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    return;
  }

  // Clear LED Array
  led_array_interface->setAllLeds(0);
  led_array_interface->update();

  // Initialize variables
  uint16_t led_number;

  for (uint16_t acquisition_index = 0; acquisition_index < acquisition_count; acquisition_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < led_sequence.number_of_patterns_assigned; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
        return;

      // Sent output trigger pulses before illuminating
      for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
      {
        if (((trigger_output_mode_list[trigger_index] > 0) && (pattern_index % trigger_output_mode_list[trigger_index] == 0))
            || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == 0 && pattern_index == 0)))
        {
          sendTriggerPulse(trigger_index, false);

          if (trigger_start_delay_list_ms[trigger_index] > 0)
            delay(trigger_start_delay_list_ms[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
      {
        if (((trigger_input_mode_list[trigger_index] > 0) && (pattern_index % trigger_input_mode_list[trigger_index] == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == 0 && pattern_index == 0)))
          waitForTriggerState(trigger_index, true);
      }

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->setAllLeds(0);

      // Define pattern
      for (uint16_t led_idx = 0; led_idx < led_sequence.led_counts[pattern_index]; led_idx++)
      {
        led_number = led_sequence.led_list[pattern_index][led_idx];
        if (led_sequence.bit_depth == 1)
          led_array_interface->setLed(led_number, led_value);
        else
          led_array_interface->setLed(led_number, led_sequence.values[pattern_index][led_idx]);

        // Update pattern
        led_array_interface->update();
      }

      // Check if led_count is zero - if so, clear the array
      if (led_sequence.led_counts[pattern_index] == 0)
      {
        led_array_interface->setAllLeds(0);
        led_array_interface->update();
      }

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms))
      {
        Serial.println(F("Error - delay too short!"));
        return;
      }

      // Wait for the defined mininum amount of time (delay_ms) before checking trigger input state
      while ((float)elapsed_us_inner < (1000 * (float)delay_ms)) {} // Wait until this is true

      // Wait for all devices to stop acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
      {
        if (((trigger_input_mode_list[trigger_index] > 0) && (pattern_index % trigger_input_mode_list[trigger_index] == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == led_sequence.number_of_patterns_assigned))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == acquisition_count && pattern_index == led_sequence.number_of_patterns_assigned)))
          waitForTriggerState(trigger_index, false);
      }
    }
  }

  led_array_interface->setAllLeds(0);
  led_array_interface->update();

  // Let user know we're done
  Serial.println("Finished sending sequence.");
}

void LedArray::runSequenceFast(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.println(F("Starting fast Sequence"));

  uint16_t delay_us = 100;
  uint16_t acquisition_count = 1;
  bool use_ms = false;

  /* Format for argv:
     0: delay between acquisitions, us/ms
     1: whether to use micro-seconds for units (default is false)
     2: number of times to repeat pattern
     3: trigger output 1 setting
     4: trigger output 2 setting
     5: trigger input 1 setting
     6: trigger input 2 setting
  */

  // Reset Trigger parameters
  trigger_output_mode_list[0] = 0;
  trigger_output_mode_list[1] = 0;
  trigger_input_mode_list[0] = 0;
  trigger_input_mode_list[1] = 0;

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      delay_us  = strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      use_ms = atoi(argv[1]) > 0;
    else if (argc_index == 2)
      acquisition_count  = strtoul(argv[2], NULL, 0);
    else if (argc_index >= 3 && argc_index < 5)
      trigger_output_mode_list[argc_index - 3] = atoi(argv[argc_index]);
    else if (argc_index >= 5 && argc_index < 7)
      trigger_input_mode_list[argc_index - 5] = atoi(argv[argc_index]);
    else
      Serial.println("WARNING:  Ignoring additional argument in runSequence");
  }

  float delay_us_used = 0;
  if (use_ms)
    delay_us_used = 1000.0 * (float)delay_us;
  else
    delay_us_used = (float)delay_us;

  if (debug)
  {
    Serial.println("OPTIONS:");
    Serial.print("  delay_us: ");
    Serial.println(delay_us_used);
    Serial.print("  acquisition_count: ");
    Serial.println(acquisition_count);
    Serial.print("  trigger out 0: ");
    Serial.println(trigger_output_mode_list[0]);
    Serial.print("  trigger out 1: ");
    Serial.println(trigger_output_mode_list[1]);
    Serial.print("  trigger in 0: ");
    Serial.println(trigger_input_mode_list[0]);
    Serial.print("  trigger in 1: ");
    Serial.println(trigger_input_mode_list[1]);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_us_used < (float)MIN_SEQUENCE_DELAY_FAST))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_us_used);
    Serial.print("us) was shorter than MIN_SEQUENCE_DELAY_FAST (");
    Serial.print(MIN_SEQUENCE_DELAY_FAST);
    Serial.print("us).");
    return;
  }

  // Clear LED Array
  led_array_interface->setAllLedsFast(0);

  // Initialize variables
  uint16_t led_number;

  for (uint16_t acquisition_index = 0; acquisition_index < acquisition_count; acquisition_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < led_sequence.number_of_patterns_assigned; pattern_index++)
    {

      // Return if we send any command to interrupt.
      if (Serial.available())
        return;

      // Sent output trigger pulses before illuminating
      for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
      {
        if (((trigger_output_mode_list[trigger_index] > 0) && (pattern_index % trigger_output_mode_list[trigger_index] == 0))
            || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == 0 && pattern_index == 0)))
        {
          sendTriggerPulse(trigger_index, false);

          if (trigger_start_delay_list_ms[trigger_index] > 0)
            delay(trigger_start_delay_list_ms[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
      {
        if (((trigger_input_mode_list[trigger_index] > 0) && (pattern_index % trigger_input_mode_list[trigger_index] == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == 0 && pattern_index == 0)))
          waitForTriggerState(trigger_index, true);
      }

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->setAllLedsFast(0);

      // Define pattern
      for (uint16_t led_idx = 0; led_idx < led_sequence.led_counts[pattern_index]; led_idx++)
      {
        led_number = led_sequence.led_list[pattern_index][led_idx];
        led_array_interface->setLedFast(led_number, true); // assume the fast that there is a LED # implies this LED is on
      }

      // Check if led_count is zero - if so, clear the array
      if (led_sequence.led_counts[pattern_index] == 0)
        led_array_interface->setAllLeds(0);

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > delay_us_used)
      {
        Serial.println(F("Error - delay("));
        Serial.print((float)elapsed_us_inner);
        Serial.println(F("us) too short!"));
        return;
      }

      // Set all LEDs to zero
      led_array_interface->setAllLedsFast(0);

      // Wait for the defined mininum amount of time (delay_ms) before checking trigger input state
      while ((float)elapsed_us_inner < (delay_us_used)) {} // Wait until this is true

      // Wait for all devices to stop acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
      {
        if (((trigger_input_mode_list[trigger_index] > 0) && (pattern_index % trigger_input_mode_list[trigger_index] == 0))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == led_sequence.number_of_patterns_assigned))
            || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (acquisition_index == acquisition_count && pattern_index == led_sequence.number_of_patterns_assigned)))
          waitForTriggerState(trigger_index, false);
      }
    }
  }

  // Clear LED Array
  led_array_interface->setAllLedsFast(0);

  // Wait for all devices to finish acquiring
  for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
  {
    if (trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION || trigger_input_mode_list[trigger_index] == TRIG_MODE_START)
      waitForTriggerState(trigger_index, false);
  }

  // Print results
  if (debug)
    Serial.println(F("Finished fast Sequence"));
}

void LedArray::stepSequence(uint16_t argc, char ** argv)
{
  Serial.println(F("Stepping sequence"));

  /* Format for argv:
     0: trigger output 1 setting
     1: trigger output 2 setting
     2: trigger input 1 setting
     3: trigger input 2 setting
  */

  // Reset Trigger parameters
  trigger_output_mode_list[0] = 0;
  trigger_output_mode_list[1] = 0;
  trigger_input_mode_list[0] = 0;
  trigger_input_mode_list[1] = 0;

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index >= 0 && argc_index < 2)
      trigger_output_mode_list[argc_index] = atoi(argv[argc_index]);
    else if (argc_index >= 2 && argc_index < 4)
      trigger_input_mode_list[argc_index - 2] = atoi(argv[argc_index]);
    else
      Serial.println("WARNING:  Ignoring additional argument in stepSequence");
  }

  if (debug)
  {
    Serial.println("OPTIONS:");
    Serial.print("  trigger out 0: ");
    Serial.println(trigger_output_mode_list[0]);
    Serial.print("  trigger out 1: ");
    Serial.println(trigger_output_mode_list[1]);
    Serial.print("  trigger in 0: ");
    Serial.println(trigger_input_mode_list[0]);
    Serial.print("  trigger in 1: ");
    Serial.println(trigger_input_mode_list[1]);
  }
  // Reset Trigger parameters
  trigger_output_mode_list[0] = 0;
  trigger_output_mode_list[1] = 0;


  // Incriment counter
  sequence_number_displayed++;

  // Loop sequence counter if it's at the end
  if (sequence_number_displayed >= led_sequence.number_of_patterns_assigned)
    sequence_number_displayed = 0;

  uint16_t led_number;

  // Sent output trigger pulses before illuminating
  for (int trigger_index = 0; trigger_index < TRIGGER_OUTPUT_COUNT; trigger_index++)
  {
    if (((trigger_input_mode_list[trigger_index] > 0) && (sequence_number_displayed % trigger_input_mode_list[trigger_index] == 0))
        || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (sequence_number_displayed == 0))
        || ((trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_number_displayed == 0)))
    {
      sendTriggerPulse(trigger_index, false);

      if (trigger_start_delay_list_ms[trigger_index] > 0)
        delay(trigger_start_delay_list_ms[trigger_index]);
    }
  }

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
  {
    if (((trigger_input_mode_list[trigger_index] > 0) && (sequence_number_displayed % trigger_input_mode_list[trigger_index] == 0))
        || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (sequence_number_displayed == 0))
        || ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_number_displayed == 0)))
      waitForTriggerState(trigger_index, true);
  }

  elapsedMicros elapsed_us_inner;

  // Clear the array
  led_array_interface->setAllLeds(0);

  // Send LEDs
  for (uint16_t led_idx = 0; led_idx < led_sequence.led_counts[sequence_number_displayed]; led_idx++)
  {
    led_number = led_sequence.led_list[sequence_number_displayed][led_idx];
    led_array_interface->setLed(led_number, led_sequence.values[sequence_number_displayed][led_idx]);
  }

  // Update pattern
  led_array_interface->update();

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < TRIGGER_INPUT_COUNT; trigger_index++)
  {
    if ((trigger_input_mode_list[trigger_index] > 0) && (sequence_number_displayed % trigger_input_mode_list[trigger_index] == 0))
      waitForTriggerState(trigger_index, true);
    else if ((trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (sequence_number_displayed == 0))
      waitForTriggerState(trigger_index, true);
    else if ((trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_number_displayed == 0))
      waitForTriggerState(trigger_index, true);
  }

  // Print user feedback
  Serial.print(F("Displayed pattern # ")); Serial.print(sequence_number_displayed); Serial.print(F(" of ")); Serial.println( led_sequence.number_of_patterns_assigned);
}

void LedArray::setDistanceZ(float new_z)
{
  if (new_z > 0)
  {
    led_array_interface_distance_z = new_z / 100.0;
    Serial.print(F("Set LED Array distance to: "));
    Serial.print(led_array_interface_distance_z);
    Serial.println(F("mm"));
    clearNaList();
    buildNaList(led_array_interface_distance_z);
  }
}

void LedArray::toggleAutoClear(uint16_t argc, char ** argv)
{
  if (argc == 0)
    auto_clear_flag = !auto_clear_flag;
  else
    auto_clear_flag = (bool)atoi(argv[0]);
}

void LedArray::toggleDebug(uint16_t argc, char ** argv)
{
  if (argc == 0)
  {
    debug = !(debug > 0);
    LedArrayInterface::debug = !(LedArrayInterface::debug > 0);
  }
  else
  {
    debug = atoi(argv[0]);
    LedArrayInterface::debug = atoi(argv[0]);
  }
  Serial.print("Debug level now: "); Serial.println(debug);
}

void LedArray::isr0()
{
  LedArrayInterface::triggerInputChange_0();
}

void LedArray::isr1()
{
  LedArrayInterface::triggerInputChange_1();
}

void LedArray::setInterface(LedArrayInterface * interface)
{
  led_array_interface = interface;
}

void LedArray::setup()
{
  // Initialize led array
  led_array_interface->deviceSetup();

  // Attach interrupts
  pinMode(TRIGGER_INPUT_PIN_0, INPUT);
  pinMode(TRIGGER_INPUT_PIN_1, INPUT);
  //  attachInterrupt(TRIGGER_INPUT_PIN_0, isr0, CHANGE);
  //  attachInterrupt(TRIGGER_INPUT_PIN_1, isr1, CHANGE);

  // Populate constants related to LED array design
  color_channel_count = led_array_interface->color_channel_count;
  LedArrayInterface::trigger_input_state[0] = false; // Todo this isn't ideal
  LedArrayInterface::trigger_input_state[1] = false; // Todo this isn't ideal

  LedArrayInterface::trigger_output_pin_list[0] = TRIGGER_OUTPUT_PIN_0;
  LedArrayInterface::trigger_output_pin_list[1] = TRIGGER_OUTPUT_PIN_1;
  LedArrayInterface::trigger_input_pin_list[0] = TRIGGER_INPUT_PIN_0;
  LedArrayInterface::trigger_input_pin_list[1] = TRIGGER_INPUT_PIN_1;
  LedArrayInterface::debug = true;

  // Reset sequence
  led_sequence.deallocate();

  // Initialize sequences at every bit depth so these are defined
  led_sequence.allocate(4);
  led_sequence.incriment(1);
  led_sequence.append(0, 127);
  led_sequence.incriment(1);
  led_sequence.append(1, 127);
  led_sequence.incriment(1);
  led_sequence.append(2, 127);
  led_sequence.incriment(1);
  led_sequence.append(3, 127);

  // Build list of LED NA coordinates
  buildNaList(led_array_interface_distance_z);

  // Define default NA
  setNa(DEFAULT_NA);
}

void LedArray::drawSpiralPattern(uint16_t delay_ms)
{
  if ((delay_ms > 0) && (delay_ms < DELAY_MAX))
  {
    clearArray();

    Serial.println(F("Starting full LED scan..."));
    for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      led_array_interface->setLed(led_index, led_value);
      Serial.print(led_index);
      Serial.print(", ");
      led_array_interface->update();
      delay(delay_ms);
    }
    Serial.println(" ");
    Serial.println(F("Done with full LED scan."));
  }
  else
    Serial.println(F("ERROR - full scan delay too short/long"));
}

void LedArray::demo()
{
  while (1)
  {
    led_value = 127;

    drawBrightfield();
    delay(2000);

    for ( int16_t led_index = led_array_interface->led_count; led_index > 0; led_index
      --)
    {
      led_array_interface->setAllLeds(0);
      led_array_interface->setLed(led_index, (uint8_t)127);
    }
    led_array_interface->update();
    delay(1);
  }
}

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

#include "ledarray.h"
#include "errorcodes.h"

volatile uint16_t LedArray::pattern_index = 0;
volatile uint16_t LedArray::frame_index = 0;

volatile float trigger_feedback_timeout_ms = 1000;
volatile uint32_t * LedArray::trigger_pulse_width_list_us;
volatile uint32_t * LedArray::trigger_start_delay_list_us;
volatile int * LedArray::trigger_input_mode_list;
volatile int * LedArray::trigger_output_mode_list;
LedSequence LedArray::led_sequence;

uint8_t LedArray::getDeviceCommandCount()
{
  return led_array_interface->getDeviceCommandCount();
}

const char * LedArray::getDeviceCommandNameShort(int device_command_index)
{
  return led_array_interface->getDeviceCommandNameShort(device_command_index);
}
const char * LedArray::getDeviceCommandNameLong(int device_command_index)
{
  return led_array_interface->getDeviceCommandNameLong(device_command_index);
}

void LedArray::deviceCommand(int device_command_index, int argc, char * *argv)
{
  // Get pattern sizes (stored as a 32-bit integer with first 16 as leds per pattern pattern and second as pattern count
  uint32_t concatenated = led_array_interface->getDeviceCommandLedListSize(device_command_index);
  uint16_t pattern_count  = (uint16_t)(concatenated >> 16);
  uint16_t leds_per_pattern = (uint16_t)concatenated;

  // Get arguments
  int16_t pattern_number = 0;
  if (argc == 0)
    pattern_number = -1;
  else if (argc == 1)
    pattern_number = strtoul(argv[0], NULL, 0);
  else
  {
    Serial.printf("ERROR (LedArray::deviceCommand) Invalid number of arguments (%d) %s", argc, SERIAL_LINE_ENDING);
    return;
  }


  if (auto_clear_flag)
    led_array_interface->clear();

  if (pattern_number < 0)
  {
    for (int16_t pattern_index = 0; pattern_index < pattern_count; pattern_index++)
    {
      for (int16_t led_index = 0; led_index < leds_per_pattern; led_index++)
      {
        if (debug >= 3)
          Serial.printf("Pattern %d contains led %d %s", pattern_index, led_array_interface->getDeviceCommandLedListElement(device_command_index, pattern_index, led_index), SERIAL_LINE_ENDING);

        for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
          led_array_interface->setLed(led_array_interface->getDeviceCommandLedListElement(device_command_index, pattern_index, led_index), color_channel_index, led_value[color_channel_index]);
      }
    }
  }
  else
  {
    for (int16_t led_index = 0; led_index < leds_per_pattern; led_index++)
    {
      if (debug >= 3)
        Serial.printf("Pattern %d contains led %d %s", pattern_index, led_array_interface->getDeviceCommandLedListElement(device_command_index, pattern_index, led_index), SERIAL_LINE_ENDING);

      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_array_interface->getDeviceCommandLedListElement(device_command_index, pattern_number, led_index), color_channel_index, led_value[color_channel_index]);
    }
  }

  // Update pattern
  led_array_interface->update();
}

/* A function to print current LED positions (xyz) */
void LedArray::printLedPositions(uint16_t argc, char * *argv, bool print_na)
{
  // Parse arguments
  uint16_t start_index = 0;
  uint16_t end_index = led_array_interface->led_count;
  if (argc == 1)
    start_index = strtoul(argv[0], NULL, 0);
  else if (argc == 2)
  {
    start_index = strtoul(argv[0], NULL, 0);
    end_index = strtoul(argv[1], NULL, 0);
  }

  // Initialize working variables
  int16_t led_number;
  float na_x, na_y, x, y, z;

  if (print_na)
  {
    buildNaList(led_array_distance_z);
    Serial.printf(F("{\n    \"led_position_list_na\" : {%s"), SERIAL_LINE_ENDING);
  }
  else
    Serial.printf(F("{\n    \"led_position_list_cartesian\" : {%s"), SERIAL_LINE_ENDING);


  for (uint16_t led_index = start_index; led_index < end_index; led_index++)
  {
    led_number = (int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][0]));

    if (print_na)
    {
      na_x = LedArrayInterface::led_position_list_na[led_number][0];
      na_y = LedArrayInterface::led_position_list_na[led_number][1];

      Serial.printf(F("        \"%d\" : ["), led_number);
      Serial.printf(F("%01.03f, "), na_x);
      if (led_index != end_index - 1)
        Serial.printf(F("%01.03f],\n"), na_y);
      else
        Serial.printf(F("%01.03f]\n"), na_y);
    }
    else
    {
      x = float((int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][2]))) / 100.0;
      y = float((int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][3]))) / 100.0;
      z = float((int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][4]))) / 100.0;
      z = z - float((int16_t)pgm_read_word(&(LedArrayInterface::led_positions[0][4]))) / 100.0 + led_array_distance_z;

      Serial.printf(F("        \"%d\" : ["), led_number);
      Serial.printf(F("%02.02f, "), x);
      Serial.printf(F("%02.02f, "), y);
      if (led_index != end_index - 1)
        Serial.printf(F("%02.02f],\n"), z);
      else
        Serial.printf(F("%02.02f]\n"), z);
    }
  }
  Serial.printf(F("    }%s}"), SERIAL_LINE_ENDING);
}

/* A function to print current LED values */
void LedArray::printCurrentLedValues(uint16_t argc, char * *argv)
{
  // Parse arguments
  uint16_t start_index = 0;
  uint16_t end_index = led_array_interface->led_count;
  if (argc == 1)
    start_index = strtoul(argv[0], NULL, 0);
  else if (argc == 2)
  {
    start_index = strtoul(argv[0], NULL, 0);
    end_index = strtoul(argv[1], NULL, 0);
  }

  int16_t led_number;
  Serial.printf(F("{\n    \"led_values\" : {%s"), SERIAL_LINE_ENDING);
  for (uint16_t led_index = start_index; led_index < end_index; led_index++)
  {
    led_number = (int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][0]));

    Serial.printf(F("        \"%d\" : ["), led_number);
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    {
      if (led_array_interface->bit_depth == 16)
      {
        if (USE_8_BIT_VALUES)
          Serial.printf(F("%u"), (uint8_t) round(255.0 * ((float)led_array_interface->getLedValue(led_number, color_channel_index) / 65535.0)));
        else
          Serial.printf(F("%lu"), led_array_interface->getLedValue(led_number, color_channel_index));
      }
      else if (led_array_interface->bit_depth == 8)
        Serial.printf(F("%u"), led_array_interface->getLedValue(led_number, color_channel_index));
      else if (led_array_interface->bit_depth == 1)
        Serial.printf(F("%u"), led_array_interface->getLedValue(led_number, color_channel_index));

      if (color_channel_index < (led_array_interface->color_channel_count - 1))
        Serial.print(',');
      else
      {
        if (led_index !=  led_array_interface->led_count - 1)
          Serial.print(F("], \n"));
        else
          Serial.print(F("] \n"));
      }
    }
  }
  Serial.printf("    }\n}%s", SERIAL_LINE_ENDING);
}

/* A function to the version of this device */
void LedArray::printVersion()
{
  Serial.print(VERSION);
  Serial.print(SERIAL_LINE_ENDING);
}

/* A function to print a human-readable about page */
void LedArray::printAbout()
{
  Serial.printf("====================================================================================================%s", SERIAL_LINE_ENDING);
  Serial.print("  ");
  Serial.print(led_array_interface->device_name);
  Serial.printf(F(" LED Array Controller %s"), SERIAL_LINE_ENDING);
  Serial.print(F("  Illuminate r"));
  Serial.print(VERSION);
  Serial.print(F(" | Serial Number: "));
  Serial.printf("%04d", led_array_interface->getSerialNumber());
  Serial.print(F(" | Part Number: "));
  Serial.printf("%04d", getPartNumber());
  Serial.print(F(" | Teensy MAC address: "));
  printMacAddress();

  Serial.printf(F("\n  For help, type ? %s"), SERIAL_LINE_ENDING);
  Serial.printf("====================================================================================================%s", SERIAL_LINE_ENDING);
}

/* A function to print a json-formatted file which contains relevant system parameters */
void LedArray::printSystemParams()
{
  Serial.printf(F("{%s"), SERIAL_LINE_ENDING);
  Serial.print(F("    \"device_name\" : \""));
  Serial.print(led_array_interface->device_name);
  Serial.print(F("\",\n    \"led_count\" : "));
  Serial.print(led_array_interface->led_count);
  Serial.print(F(",\n    \"color_channels\" : ["));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(", "));
    Serial.print('\"');
    Serial.print(led_array_interface->color_channel_names[channel_index]);
    Serial.print('\"');
  }
  Serial.print(F("]"));
  Serial.print(F(",\n    \"color_channel_center_wavelengths\" : {"));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(", "));
    Serial.print('\"');
    Serial.print(LedArrayInterface::color_channel_names[channel_index]);
    Serial.print('\"');
    Serial.printf(" : %.3f", LedArrayInterface::color_channel_center_wavelengths[channel_index]);
  }
  Serial.print(F("},\n    \"trigger_input_count\" : "));
  Serial.print(led_array_interface->trigger_input_count);
  Serial.print(F(",\n    \"trigger_output_count\" : "));
  Serial.print(led_array_interface->trigger_output_count);
  Serial.print(F(",\n    \"bit_depth\" : "));
  Serial.print(led_array_interface->bit_depth);
  Serial.print(F(",\n    \"serial_number\" : "));
  Serial.print(led_array_interface->getSerialNumber());
  Serial.print(F(",\n    \"color_channel_count\" : "));
  Serial.print(led_array_interface->color_channel_count);
  Serial.print(F(",\n    \"part_number\" : "));
  Serial.print(led_array_interface->getPartNumber());
  Serial.print(F(",\n    \"mac_address\" : \""));
  printMacAddress();
  Serial.print(F("\""));
  Serial.print(F(",\n    \"interface_version\" : "));
  Serial.print(VERSION);

  // Terminate JSON
  Serial.printf("\n}", SERIAL_LINE_ENDING);
}

void LedArray::setMaxCurrentLimit(int argc, char ** argv)
{
  if (argc == 1)
    led_array_interface->setMaxCurrentLimit(atof(argv[0]));
  else
    Serial.printf(F("ERROR (LedArray::setMaxCurrentLimit): Invalid number of arguments.%s"), SERIAL_LINE_ENDING);
}

void LedArray::setMaxCurrentEnforcement(int argc, char ** argv)
{
  if (argc == 1)
    led_array_interface->setMaxCurrentEnforcement(atoi(argv[0]) > 0);
  else
    Serial.printf(F("ERROR (LedArray::setMaxCurrentEnforcement): Invalid number of arguments.%s"), SERIAL_LINE_ENDING);
}

void LedArray::printMacAddress()
{
  print_mac();
}

uint16_t LedArray::getSerialNumber()
{
  return led_array_interface->getSerialNumber();
}

void LedArray::setSerialNumber(uint16_t new_serial_number)
{
  led_array_interface->setSerialNumber(new_serial_number);
}

void LedArray::setDemoMode(int8_t mode)
{
  led_array_interface->setDemoMode(mode);
}

int8_t LedArray::getDemoMode()
{
  return led_array_interface->getDemoMode();
}

uint16_t LedArray::getPartNumber()
{
  return led_array_interface->getPartNumber();
}

void LedArray::setPartNumber(uint16_t new_part_number)
{
  led_array_interface->setPartNumber(new_part_number);
}

/* A function to reset the device to power-on state */
void LedArray::reset()
{
  Serial.printf(F("Resetting Array%s"), SERIAL_LINE_ENDING);
  led_array_interface->deviceReset();
}

/* A function to draw a random "disco" pattern. For parties, mostly. */
void LedArray::drawDiscoPattern()
{
  // Determine number of LEDs to illuminate at once
  int led_on_count = (int)round(led_array_interface->led_count / 4.0);

  // Clear the array
  led_array_interface->clear();

  // Party time
  while (Serial.available() == 0)
  {
    led_array_interface->clear();

    for (uint16_t led_index = 0; led_index < led_on_count; led_index++)
    {
      led_index = random(0, led_array_interface->led_count);
      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_index, color_channel_index, (uint8_t)random(0, 255));
    }
    led_array_interface->update();
    delay(10);
  }

  // Clear the array
  led_array_interface->clear();
}

/* A function to draw a water drop (radial sine pattern)*/
void LedArray::waterDrop()
{
  // Clear the array
  led_array_interface->clear();

  float na_period = LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][0] * LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][0];
  na_period += LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][1] * LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][1];
  na_period = sqrt(na_period) / 2.0;

  uint8_t value;
  float na;
  uint8_t max_led_value = 16;

  uint8_t phase_counter = 0;
  while (Serial.available() == 0)
  {
    // Clear array
    led_array_interface->setLed(-1, -1, false);
    for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      na = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);
      value = (uint8_t)round((1.0 + sin(((na / na_period) + ((float)phase_counter / 100.0)) * 2.0 * 3.14)) * max_led_value);
      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_index, color_channel_index, value);
    }
    led_array_interface->update();
    delay(1);
    phase_counter++;
    if (phase_counter == 100)
      phase_counter = 0;
  }

  // Clear the array
  led_array_interface->clear();
}

/* A function to calculate the NA of each LED given the XYZ position and an offset */
void LedArray::buildNaList(float new_board_distance)
{
  float Na_x, Na_y, yz, xz, x, y, z, z0;

  if (new_board_distance > 0)
    led_array_distance_z = new_board_distance;

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    if ((int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][1])) >= 0)
    {
      x =  float((int16_t) pgm_read_word(&(LedArrayInterface::led_positions[led_index][2]))) / 100.0;
      y =  float((int16_t) pgm_read_word(&(LedArrayInterface::led_positions[led_index][3]))) / 100.0;
      z0 = float((int16_t) pgm_read_word(&(LedArrayInterface::led_positions[0][4]))) / 100.0; // z position of center LED
      z =  float((int16_t) pgm_read_word(&(LedArrayInterface::led_positions[led_index][4]))) / 100.0 - z0 + led_array_distance_z;

      yz = sqrt(y * y + z * z);
      xz = sqrt(x * x + z * z);
      Na_x = sin(atan(x / yz));
      Na_y = sin(atan(y / xz));

      LedArrayInterface::led_position_list_na[led_index][0] = Na_x;
      LedArrayInterface::led_position_list_na[led_index][1] = Na_y;
    }
    else
    {
      LedArrayInterface::led_position_list_na[led_index][0] = INVALID_NA; // invalid NA
      LedArrayInterface::led_position_list_na[led_index][1] = INVALID_NA; // invalid NA
    }
  }
  if (debug)
    Serial.printf(F("Finished updating led positions."));
}

/* A function to fill the LED array with the color specified by led_value */
void LedArray::fillArray()
{
  // Turn on all LEDs
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);
  }

  // Update array
  led_array_interface->update();

  if (debug)
    Serial.printf(F("Filled Array%s"), SERIAL_LINE_ENDING);
}

/* A function to clear the LED array */
void LedArray::clear()
{
  led_array_interface->clear();
}

/* A function to set the numerical aperture of the system*/
void LedArray::setNa(int argc, char ** argv)
{
  if (argc == 0)
    ; // do nothing, just display current na
  else if (argc == 1)
  {
    int new_na = atoi(argv[0]);
    if ((new_na > 0) && new_na < 100 * led_array_interface->max_na)
      objective_na = (float)new_na / 100.0;
    else
      Serial.printf(F("ERROR (LedArray::setNa): invalid NA. Make sure NA is 100*na%s"), SERIAL_LINE_ENDING);
  }
  else
    Serial.printf(F("ERROR (LedArray::setNa): wrong number of arguments.%s"), SERIAL_LINE_ENDING);

  // Print current NA
  clearOutputBuffers();
  sprintf(output_buffer_short, "NA.%d", (uint8_t)round(objective_na * 100));
  sprintf(output_buffer_long, "Current NA is 0.%d.", (uint8_t)round(objective_na * 100));
  print(output_buffer_short, output_buffer_long);
}

/* A function to draw a DPC navigator pattern */
void LedArray::drawNavDpc()
{
  notImplemented("Navigator DPC");
}

/* A function to draw a darkfield pattern */
void LedArray::drawDarkfield()
{
  if (auto_clear_flag)
    clear();

  //TODO: Add check for max current
  drawCircle(objective_na, 1.0);
  led_array_interface->update();
}

/* A function to draw a cDPC pattern */
void LedArray::drawCdpc(int argc, char * *argv)
{
  if (led_array_interface->color_channel_count != 3)
  {
    notImplemented("cDPC");
    return;
  }
  else
  {
    uint8_t illumination_intensity;

    if (argc == 0)
      illumination_intensity = 127;
    else if (argc == 1)
      illumination_intensity = (uint8_t)atoi(argv[0]);
    else
    {
      Serial.printf(F("ERROR (LedArray::drawCdpc): Invalid number of arguments %s"), SERIAL_LINE_ENDING);
      return;
    }

    // This mask defines the cDPC pattern
    int cdpc_mask[4][3] = {
      {1, 0, 0},
      {1, 0, 1},
      {0, 1, 1},
      {0, 1, 0}
    };

    // Clear array
    if (auto_clear_flag)
      clear();

    for (int quadrant_index = 0; quadrant_index < 4; quadrant_index++)
    {
      // Set all colors to zero (off)
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;
      for (int color_index = 0; color_index < 3; color_index++)
      {
        if (cdpc_mask[quadrant_index][color_index])
        {
          led_value[color_index] = illumination_intensity;
          drawQuadrant(quadrant_index, 0.0, objective_na, true);
        }
      }
    }
    led_array_interface->update();
  }
}

/* A function to draw a half annulus */
void LedArray::drawHalfAnnulus(int argc, char * *argv)
{


  float na_start, na_end;
  if (argc == 1)
  {
    na_start = objective_na;
    na_end = objective_na + 0.2;
  }
  else if (argc == 3)
  {
    na_start = atof(argv[1]) / 100.0;
    na_end = atof(argv[2]) / 100.0;
  }
  else
  {
    Serial.printf(F("ERROR (LedArray::drawHalfAnnulus) Invlaid number of arguments. %s"), SERIAL_LINE_ENDING);
    return;
  }

  if (debug >= 1)
  {
    Serial.print(F("Drawing half-annulus pattern with type: "));
    Serial.print(argv[0]);
    Serial.print(F(" from "));
    Serial.print(na_start);
    Serial.print(F("NA to "));
    Serial.print(na_end);
    Serial.printf(F("NA.%s"), SERIAL_LINE_ENDING);
  }

  if (auto_clear_flag)
    clear();

  int8_t half_annulus_type = 0;
  if ( (strcmp(argv[0], DPC_TOP1) == 0) || (strcmp(argv[0], DPC_TOP2) == 0))
    half_annulus_type = 0;
  else if ( (strcmp(argv[0], DPC_BOTTOM1) == 0) || (strcmp(argv[0], DPC_BOTTOM2) == 0))
    half_annulus_type = 1;
  else if ( (strcmp(argv[0], DPC_LEFT1) == 0) || (strcmp(argv[0], DPC_LEFT2) == 0))
    half_annulus_type = 2;
  else if ( (strcmp(argv[0], DPC_RIGHT1) == 0) || (strcmp(argv[0], DPC_RIGHT2) == 0))
    half_annulus_type = 3;
  else
    Serial.printf(F("ERROR - invalid half annulus circle type. Options are t, b, l, and r %s"), SERIAL_LINE_ENDING);

  if (half_annulus_type >= 0)
  {
    drawHalfCircle(half_annulus_type, na_start, na_end);
    led_array_interface->update();
  }
}

/* A function to draw a color darkfield pattern */
void LedArray::drawColorDarkfield(int argc, char * * argv)
{
  if (led_array_interface->color_channel_count != 3)
  {
    notImplemented("Color darkfield");
    return;
  }
  else
  {
    uint8_t illumination_intensity = 127;
    float start_na = objective_na;
    float end_na = objective_na + 0.2;

    if (argc == 0)
      ; // do nothing (use default values above)
    else if (argc >= 1)
      illumination_intensity = (uint8_t)atoi(argv[0]);
    else if (argc >= 2)
    {
      start_na = atof(argv[1]);
      end_na = min(start_na + 0.2, 1.0);
    }
    else if (argc >= 3)
    {
      start_na = atof(argv[1]);
      start_na = atof(argv[2]);
    }
    else
    {
      Serial.printf(F("ERROR (LedArray::drawColorDarkfield): Invalid number of arguments %s"), SERIAL_LINE_ENDING);
      return;
    }

    // This mask defines the cDPC pattern
    int cdf_mask[4][3] = {
      {1, 0, 0},
      {1, 0, 1},
      {0, 1, 1},
      {0, 1, 0}
    };

    // Clear array
    led_array_interface->clear();
    for (int quadrant_index = 0; quadrant_index < 4; quadrant_index++)
    {
      // Set all colors to zero (off)
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;
      for (int color_index = 0; color_index < 3; color_index++)
      {
        if (cdf_mask[quadrant_index][color_index])
        {
          led_value[color_index] = illumination_intensity;
          drawQuadrant(quadrant_index, start_na, end_na, true);
        }
      }
    }
    led_array_interface->update();
  }
}

/* A function to draw an annulus*/
void LedArray::drawAnnulus(int argc, char * * argv)
{
  float start_na, end_na;
  if (argc == 0)
  {
    start_na = objective_na;
    end_na = min(objective_na + 0.2, 1.0);
  }
  else if (argc == 2)
  {
    start_na = (float)atoi(argv[0]) / 100.0;
    end_na = (float)atoi(argv[1]) / 100.0;
  }
  else
  {
    Serial.printf(F("ERROR (LedArray::drawAnnulus): Invalid number of arguments! %s"), SERIAL_LINE_ENDING);
    return;
  }

  if (debug >= 1)
  {
    Serial.print(F("Drawing annulus from "));
    Serial.print(start_na);
    Serial.print(F("NA to "));
    Serial.print(end_na);
    Serial.printf(F("NA.%s"), SERIAL_LINE_ENDING);
  }

  if (auto_clear_flag)
    clear();

  // Draw circle
  drawCircle(start_na, end_na);
  led_array_interface->update();
}

/* A function to draw a spoecific LED channel as indexed in hardware */
void LedArray::drawChannel(int argc, char * *argv)
{
  if (argc != 1)
    Serial.printf(F("ERROR (LedArray::drawChannel): invalid argument count%s"), SERIAL_LINE_ENDING);
  else
  {
    if (auto_clear_flag)
      clear();

    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->setChannel(strtol(argv[0], NULL, 0), color_channel_index, led_value[color_channel_index]);

    led_array_interface->update();
  }
}

/* A function to set the pin order of a LED (for multi-color designs */
void LedArray::setPinOrder(int argc, char * *argv)
{
  if (argc == led_array_interface->color_channel_count)
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->setPinOrder(-1, color_channel_index, strtoul(argv[color_channel_index], NULL, 0));
  else if (argc == led_array_interface->color_channel_count + 1)
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->setPinOrder(strtoul(argv[0], NULL, 0) , color_channel_index, strtoul(argv[color_channel_index + 1], NULL, 0));
  else
    Serial.printf(F("ERROR (LedArray::setPinOrder): Wrong number of arguments %s"), SERIAL_LINE_ENDING);
}

/* Trigger setup function for setting the trigger pulse width and delay after sending */
void LedArray::triggerSetup(int argc, char ** argv)
{
  int trigger_index = 0;
  uint32_t trigger_pulse_width_us = 0;
  uint32_t trigger_start_delay_ms = 0;

  if ((argc == 2) || (argc == 3)) {
    trigger_index = atoi(argv[0]);
    trigger_pulse_width_us = strtoul(argv[1], NULL, 0);
    if (argc >= 3)
      trigger_start_delay_ms = strtoul(argv[2], NULL, 0);

    if (trigger_pulse_width_us >= 0)
      LedArray::trigger_pulse_width_list_us[trigger_index] = trigger_pulse_width_us;

    if (trigger_start_delay_ms >= 0)
      LedArray::trigger_start_delay_list_us[trigger_index] = trigger_start_delay_ms;

    if (debug > 1)
    {
      Serial.print("Trigger ");
      Serial.print(trigger_index);
      Serial.print(" now has a pulse width of ");
      Serial.print(LedArray::trigger_pulse_width_list_us[trigger_index] );
      Serial.print("us and a start delay of ");
      Serial.print(LedArray::trigger_start_delay_list_us[trigger_index]);
      Serial.printf(F("us. %s"), SERIAL_LINE_ENDING);
    }
  }
  else if (argc == 0)
  {

    //    // Print trigger settings
    //    // Input Pins
    //    for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
    //    {
    //      Serial.print("Trigger input pin index ");
    //      Serial.print(trigger_index);
    //      Serial.print(F(" uses Pin #"));
    //      Serial.print(LedArrayInterface::trigger_input_pin_list[trigger_index]);
    //      Serial.print(SERIAL_LINE_ENDING);
    //    }
    //
    //    // Output Pins
    //    for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
    //    {
    //      Serial.print(F("Trigger output pin index "));
    //      Serial.print(trigger_index);
    //      Serial.print(F(" uses Pin #"));
    //      Serial.print(LedArrayInterface::trigger_output_pin_list[trigger_index]);
    //      Serial.print(F(" with pulse width "));
    //      Serial.print(LedArray::trigger_pulse_width_list_us[trigger_index]);
    //      Serial.print(F("us. Start delay is "));
    //      Serial.print(LedArray::trigger_start_delay_list_us[trigger_index]);
    //      Serial.printf(F("us.%s"), SERIAL_LINE_ENDING);
    //    }

    Serial.printf(F("{%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);
    Serial.printf(F("    \"input\": [%s"), SERIAL_LINE_ENDING);
    for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
    {
      Serial.printf("        {\"channel\": %d, \"pin\": %d}",
                    trigger_index,
                    LedArrayInterface::trigger_input_pin_list[trigger_index]);

      if (trigger_index < (led_array_interface->trigger_input_count - 1))
        Serial.printf(",%s", SERIAL_LINE_ENDING);
      else
        Serial.printf("%s", SERIAL_LINE_ENDING);
    }
    Serial.printf(F("    ],%s    \"output\": [%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);
    for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
    {
      Serial.printf("        {\"channel\": %d, \"pin\": %d, \"pulse_width_us\": %d, \"start_delay_us\": %d}",
                    trigger_index,
                    LedArrayInterface::trigger_output_pin_list[trigger_index],
                    LedArray::trigger_pulse_width_list_us[trigger_index],
                    LedArray::trigger_start_delay_list_us[trigger_index]);

      if (trigger_index < (led_array_interface->trigger_input_count - 1))
        Serial.printf(",%s", SERIAL_LINE_ENDING);
      else
        Serial.printf("%s", SERIAL_LINE_ENDING);
    }
    Serial.printf(F("    ]%s}%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);
  }
  else
    Serial.printf(F("ERROR: Invalid number of arguments for setTriggerPulse! %s"), SERIAL_LINE_ENDING);
}

/* Send a trigger pulse */
void LedArray::sendTriggerPulse(int trigger_index, bool show_output)
{
  // TODO: store polarity and use it here
  if (debug >= 2)
    Serial.printf(F("Called sendTriggerPulse %s"), SERIAL_LINE_ENDING);

  // Send trigger pulse with pulse_width
  int status = led_array_interface->sendTriggerPulse(trigger_index, LedArray::trigger_pulse_width_list_us[trigger_index], false);

  if (status < 0)
    Serial.printf(F("ERROR - pin not configured! %s"), SERIAL_LINE_ENDING);
}

void LedArray::setTriggerState(int trigger_index, bool state, bool show_output)
{
  int status = led_array_interface->setTriggerState(trigger_index, state);
  if (status < 0)
    Serial.printf(F("ERROR - pin not configured! %s"), SERIAL_LINE_ENDING);
}

bool LedArray::getTriggerState(int trigger_index)
{
  return (digitalReadFast(led_array_interface->trigger_input_pin_list[trigger_index]));
}

/* Wait for a TTL trigger port to be in the given state */
bool LedArray::waitForTriggerState(int trigger_index, bool state)
{
  float delayed_ms = 0;
  while ((digitalReadFast(led_array_interface->trigger_input_pin_list[trigger_index]) != state))
  {
    delayMicroseconds(1);
    delayed_ms += 0.001;
    if (delayed_ms > MAX_TRIGGER_WAIT_TIME_S * 1000.0)
    {
      Serial.printf(F("WARNING (LedArray::waitForTriggerState): Exceeding max delay for trigger input %d (%.2f sec.) %s"), trigger_index, MAX_TRIGGER_WAIT_TIME_S, SERIAL_LINE_ENDING);
      return false;
    }
  }
  return true;
}
/* A trigger test function */
void LedArray::triggerInputTest(uint16_t channel)
{
  led_array_interface->setLed(-1, -1, (uint8_t)0);
  led_array_interface->update();
  Serial.print(LedArrayInterface::trigger_input_state[channel]); Serial.print(SERIAL_LINE_ENDING);
  Serial.print("Begin trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  bool result = waitForTriggerState(channel, !LedArrayInterface::trigger_input_state[channel]);
  if (result)
  {
    Serial.print("Passed trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  }
  else
  {
    Serial.print("Failed trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  }
  led_array_interface->setLed(-1, -1, (uint8_t)0);
  led_array_interface->setLed(0, -1, (uint8_t)255);
  led_array_interface->update();
}

/* Draw a LED list */
void LedArray::drawLedList(uint16_t argc, char ** argv)
{
  if (debug >= 2)
    Serial.printf(F("LedArray::drawLedList called %s"), SERIAL_LINE_ENDING);

  uint16_t led_number;
  if (auto_clear_flag)
    clear();

  for (uint16_t led_index = 0; led_index < argc; led_index++)
  {
    led_number = strtoul(argv[led_index], NULL, 0);
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->setLed(led_number, color_channel_index, led_value[color_channel_index]);
  }
  led_array_interface->update();
}

/* Scan brightfield LEDs */
void LedArray::scanBrightfieldLeds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;

  // Parse inputs
  if (argc == 1)
    delay_ms = strtoul(argv[0], NULL, 0);
  else if (argc <= led_array_interface->trigger_output_count)
  {
    for (int arg_index = 0; arg_index < argc; arg_index++)
      LedArray::trigger_output_mode_list[arg_index] = atoi(argv[arg_index]);
  }

  // Scan the LEDs
  scanLedRange(delay_ms, 0.0, objective_na, true);

  if (debug >= 1)
    Serial.printf(F("Finished brightfield LED scan %s"), SERIAL_LINE_ENDING);
}

/* Scan all LEDs */
void LedArray::scanAllLeds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;
  if (argc == 1)
    delay_ms = strtoul(argv[0], NULL, 0);

  if ((delay_ms >= 0) && (delay_ms < DELAY_MAX))
  {
    // Clear array initially
    clear();
    for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
    {
      if (LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
        sendTriggerPulse(trigger_index, false);
    }

    // Initiate LED scan
    scanLedRange(delay_ms, 0.0, 1.0, true);

    if (debug >= 1)
      Serial.printf(F("Finished full LED scan.%s"), SERIAL_LINE_ENDING);
  }
  else
    Serial.printf(F("ERROR - full scan delay too short/long %s"), SERIAL_LINE_ENDING);
}

void LedArray::setBrightness(int16_t argc, char ** argv)
{

  if (argc == 0)
    ; // pass
  else
  {
    if (strcmp(argv[0], "max") == 0)
    {
      led_brightness = UINT8_MAX;
    }
    else if (strcmp(argv[0], "min") == 0)
    {
      led_brightness = 1;
    }
    else if (strcmp(argv[0], "half") == 0)
    {
      led_brightness = (uint8_t) ((float)UINT8_MAX / 2.0);
    }
    else if (strcmp(argv[0], "quarter") == 0)
    {
      led_brightness = (uint8_t) ((float)UINT8_MAX / 4.0);
    }
    else
      led_brightness = (uint8_t) strtoul(argv[0], NULL, 0);
  }

  // Set LED vlaue based on color and brightness
  for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    led_value[color_channel_index] = (uint8_t) ceil((float) led_color[color_channel_index] / (float) UINT8_MAX * (float) led_brightness);

  // Print current brightness
  clearOutputBuffers();
  sprintf(output_buffer_short, "SB.%u", led_brightness);
  sprintf(output_buffer_long, "Current brightness value is %u.", led_brightness);
  print(output_buffer_short, output_buffer_long);
}

/* Allows setting of current color buffer, which is respected by most other commands */
void LedArray::setColor(int16_t argc, char ** argv)
{
  if (led_array_interface->color_channel_count > 1)
  {
    // TODO: check if argv is within valid range
    // TODO: respect bit_depth
    if (argc == 0)
      ; // Do nothing
    else if (argc == 1)
    {
      if (strcmp(argv[0], "red") == 0  && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = default_brightness;
        led_color[1] = 0;
        led_color[2] = 0;
      }
      else if (strcmp(argv[0], "green") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = default_brightness;
        led_color[2] = 0;
      }
      else if (strcmp(argv[0], "blue") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = 0;
        led_color[2] = default_brightness;
      }
      else if (strcmp(argv[0], "white") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = default_brightness;
        led_color[1] = default_brightness;
        led_color[2] = default_brightness;
      }
      else if (strcmp(argv[0], "redmax") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = UINT8_MAX;
        led_color[1] = 0;
        led_color[2] = 0;
      }
      else if (strcmp(argv[0], "greenmax") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = UINT8_MAX;
        led_color[2] = 0;
      }
      else if (strcmp(argv[0], "bluemax") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = 0;
        led_color[2] = UINT8_MAX;
      }
      else if (strcmp(argv[0], "whitemax") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = UINT8_MAX;
        led_color[1] = UINT8_MAX;
        led_color[2] = UINT8_MAX;
      }
      else if ((strcmp(argv[0], "all") == 0) || (strcmp(argv[0], "white") == 0))
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_color[color_channel_index] = default_brightness;

      }
      else if (strcmp(argv[0], "max") == 0)
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_color[color_channel_index] = UINT8_MAX;
      }
      else if (strcmp(argv[0], "half") == 0)
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_color[color_channel_index] = (uint8_t) ((float)UINT8_MAX / 2);
      }
      else if (strcmp(argv[0], "quarter") == 0)
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_color[color_channel_index] = (uint8_t) ((float)UINT8_MAX / 4);
      }
      else if (isdigit(argv[0][0]))
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_color[color_channel_index] = (uint8_t)atoi(argv[0]);
      }
      else
      {
        Serial.printf(F("ERROR (LedArray::setColor): Invalid color value %s"), SERIAL_LINE_ENDING);
        return;
      }
    }
    else if (argc == led_array_interface->color_channel_count && isdigit(argv[0][0]))
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_color[color_channel_index] = (uint8_t)atoi(argv[color_channel_index]);
    }
    else
    {
      Serial.printf(F("ERROR (LedArray::setColor): Invalid color value %s"), SERIAL_LINE_ENDING);
      return;
    }

    // Set LED vlaue based on color and brightness
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_value[color_channel_index] = (uint8_t) (((float) led_color[color_channel_index] / UINT8_MAX) * (float) led_brightness);


    // Print current color value
    clearOutputBuffers();

    if (led_array_interface->color_channel_count == 1)
    {
      sprintf(output_buffer_short, "CO.%d", led_color[0]);
      sprintf(output_buffer_long, "Current color value is %d.", led_color[0]);
      print(output_buffer_short, output_buffer_long);
    }
    else if (led_array_interface->color_channel_count == 3)
    {
      sprintf(output_buffer_short, "CO.%d.%d.%d", led_color[0], led_color[1], led_color[2]);
      sprintf(output_buffer_long, "Current color value is %d.%d.%d", led_color[0], led_color[1], led_color[2]);
      print(output_buffer_short, output_buffer_long);
    }
    else
      Serial.printf(F("ERROR (LedArray::setColor): Invalid color channel count (%d) %s"), led_array_interface->color_channel_count , SERIAL_LINE_ENDING);
  }
  else
  {
    Serial.printf(F("ERROR (LedArray::setColor): Current device does not support color illumination %s"), SERIAL_LINE_ENDING);
  }
}

/* Draws a single quadrant of LEDs using standard quadrant indexing (top left is 0, moving clockwise) */
void LedArray::drawQuadrant(int quadrant_number, float start_na, float end_na, bool include_center)
{
  if (debug >= 2)
  {
    Serial.print(F("Drawing Quadrant "));
    Serial.print(quadrant_number);
    Serial.print(SERIAL_LINE_ENDING);
  }

  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    float x = LedArrayInterface::led_position_list_na[led_index][0];
    float y = LedArrayInterface::led_position_list_na[led_index][1];
    float d = sqrt(x * x + y * y);

    if (!include_center)
    {
      if (  (quadrant_number == 0 && (x < 0) && (y > 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 1 && (x > 0) && (y > 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 2 && (x > 0) && (y < 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 3 && (x < 0) && (y < 0) && (d <= end_na) && (d >= start_na)))
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);
      }
    }
    else
    {
      if (  (quadrant_number == 0 && (x <= 0) && (y >= 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 1 && (x >= 0) && (y >= 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 2 && (x >= 0) && (y <= 0) && (d <= end_na) && (d >= start_na))
            || (quadrant_number == 3 && (x <= 0) && (y <= 0) && (d <= end_na) && (d >= start_na)))
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);
      }
    }
  }
}

/* Draws a single half-circle of LEDs using standard quadrant indexing (top left is 0, moving clockwise) */
void LedArray::drawHalfCircle(int8_t half_circle_type, float start_na, float end_na)
{
  if (debug >= 2)
  {
    Serial.print(F("Drawing Half Annulus:"));
    Serial.print(half_circle_type);
    Serial.print(SERIAL_LINE_ENDING);
  }

  float x, y, d;
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    x = LedArrayInterface::led_position_list_na[led_index][0];
    y = LedArrayInterface::led_position_list_na[led_index][1];
    d = sqrt(x * x + y * y);

    if (d > (start_na) && (d <= (end_na)))
    {
      if (  (half_circle_type == 0 && (y > 0))      // Top
            || (half_circle_type == 1 && (y < 0))   // Bottom
            || (half_circle_type == 2 && (x < 0))   // Left
            || (half_circle_type == 3 && (x > 0)))  // Right

        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);
    }
  }
}

/* Draws a circle or annulus of LEDs */
void LedArray::drawCircle(float start_na, float end_na)
{
  if (debug >= 2)
  {
    Serial.print(F("Drawing circle from "));
    Serial.print(start_na);
    Serial.print(F("NA to "));
    Serial.print(end_na);
    Serial.printf(F("NA %s"), SERIAL_LINE_ENDING);
  }

  // Clear array first (helps eleminate weird patterns)
  clear();

  float d;
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    d = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);
    if ((d >= start_na) && (d <= end_na))
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);
    }
  }
}

/* Scan brightfield LEDs */
void LedArray::scanLedRange(uint16_t delay_ms, float start_na, float end_na, bool print_indicies)
{
  float d;

  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
  {
    if (LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
      sendTriggerPulse(trigger_index, false);
  }

  if (print_indicies)
    Serial.print(F("scan_start:"));

  int16_t led_index = 0;
  while (led_index < (int16_t)led_array_interface->led_count && !Serial.available())
  {
    d = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);

    if (d >= start_na && d <= end_na)
    {
      // Clear all LEDs
      led_array_interface->clear();

      // Set LEDs
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_index, color_channel_index, led_value[color_channel_index]);

      if (print_indicies)
      {
        Serial.print(led_index);
        if (led_index < led_array_interface->led_count - 1)
          Serial.print(SERIAL_DELIMITER);
      }

      // Update LED Pattern
      led_array_interface->update();

      // Send trigger pulse
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION)
          sendTriggerPulse(trigger_index, false);
      }

      // Delay for desired wait period
      delay(delay_ms);
    }
    led_index++;
  }

  if (print_indicies)
    Serial.print(F(":scan_end"));

  Serial.print(SERIAL_LINE_ENDING);

  delay(delay_ms);
  led_array_interface->clear();
}

/* Command parser for DPC */
void LedArray::drawDpc(uint16_t argc, char ** argv)
{
  if (argc == 1)
  {
    if (debug >= 1)
    {
      Serial.print(F("Drew DPC pattern with type: "));
      Serial.print(argv[0]); Serial.print(SERIAL_LINE_ENDING);
    }

    if (auto_clear_flag)
      led_array_interface->clear();

    int8_t dpc_type = -1;
    if ( (strcmp(argv[0], DPC_TOP1) == 0) || (strcmp(argv[0], DPC_TOP2) == 0))
      dpc_type = 0;
    else if ( (strcmp(argv[0], DPC_BOTTOM1) == 0) || (strcmp(argv[0], DPC_BOTTOM2) == 0))
      dpc_type = 1;
    else if ( (strcmp(argv[0], DPC_LEFT1) == 0) || (strcmp(argv[0], DPC_LEFT2) == 0))
      dpc_type = 2;
    else if ( (strcmp(argv[0], DPC_RIGHT1) == 0) || (strcmp(argv[0], DPC_RIGHT2) == 0))
      dpc_type = 3;
    else
      Serial.printf(F("ERROR - invalid dpc circle type. Options are dpc.t, dpc.b, dpc.l, dpc.r%s"), SERIAL_LINE_ENDING);

    if (dpc_type >= 0)
    {
      drawHalfCircle(dpc_type, 0.0, objective_na);
      led_array_interface->update();
    }
  }
  else if (argc == 0)
  {
    // Draw the first DPC pattern
    drawHalfCircle(0, 0.0, objective_na);
    led_array_interface->update();
  }

  else
    Serial.printf(F("ERROR (LedArray::drawDpc) Invlaid number of arguments.%s"), SERIAL_LINE_ENDING);
}

/* Draw brightfield pattern */
void LedArray::drawBrightfield(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.printf(F("Drawing brightfield pattern%s"), SERIAL_LINE_ENDING);

  if (auto_clear_flag)
    clear();

  // Draw circle
  drawCircle(0.0, objective_na);
  led_array_interface->update();
}

/* Set sequence length */
void LedArray::setSequenceLength(uint16_t new_seq_length, bool quiet)
{
  // Reset old sequence
  LedArray::led_sequence.deallocate();

  // Initalize new sequence
  LedArray::led_sequence.allocate(new_seq_length);

  if (!quiet)
  {
    Serial.print(F("New sequence length is: "));
    Serial.print(new_seq_length);
    Serial.print(SERIAL_LINE_ENDING);
  }
}

void LedArray::setSequenceBitDepth(uint8_t bit_depth, bool quiet)
{
  // Set the sequence bit depth
  LedArray::led_sequence.setBitDepth(bit_depth);

  // Print bit depth
  if (!quiet)
  {
    // Print current NA
    clearOutputBuffers();
    sprintf(output_buffer_short, "BD.%d", LedArray::led_sequence.bit_depth);
    sprintf(output_buffer_long, "Sequence bit depth is now: %d.", LedArray::led_sequence.bit_depth);
    print(output_buffer_short, output_buffer_long);
  }
}

int LedArray::getSequenceBitDepth()
{
  return LedArray::led_sequence.bit_depth;
}

void LedArray::setSequenceZeros(uint16_t argc, char ** argv)
{
  if (argc != 1)
  {
    Serial.printf(F("ERROR (LedArray::setSequenceZeros): invalid number of arguments! %s"), SERIAL_LINE_ENDING);
    return;
  }
  else
  {
    uint16_t zero_count = strtoul(argv[0], NULL, 0);
    if (zero_count + LedArray::led_sequence.number_of_patterns_assigned <= LedArray::led_sequence.length)
    {
      for (uint16_t value_index = 0; value_index < zero_count; value_index++)
        LedArray::led_sequence.incriment(0);
    }
    else
    {
      Serial.printf(F("ERROR (LedArray::setSequenceZeros): number of zeros exceeds pattern length! %s"), SERIAL_LINE_ENDING);
      return;
    }
  }
}

/* Set sequence value */
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

  // Assign LED indicies and values
  uint8_t * values = ((uint8_t *) led_values);

  // Debug led values and numbers
  if (debug >= 2)
  {
    for (uint8_t led_number_index = 0; led_number_index < led_numbers[0]; led_number_index++)
    {
      Serial.print(F("Found LED # "));
      Serial.print(led_numbers[led_number_index + 1]);
      Serial.printf(F(" with value: %d %s"), values[led_number_index], SERIAL_LINE_ENDING);
    }
  }

  // Check number of arguments
  if (led_argc > 0 && (argc == (led_argc))) // Color (or white if one channel) led_array_interface->color_channel_count
  {
    // Switch to new led pattern
    if (LedArray::led_sequence.incriment(pattern_led_count) && (pattern_led_count > 0))
    {

      // Loop over argument indicies
      for (int led_argument_index = 0; led_argument_index < led_argc; led_argument_index++)
      {
        // If the led number is -1, append all LEDs to the sequence
        if (led_numbers[led_argument_index + 1] == -1)
        {
          for (int led_number = 0; led_number < led_array_interface->led_count; led_number++)
          {
            if (LedArray::led_sequence.bit_depth == 8)
              LedArray::led_sequence.append(led_number, values[led_argument_index * led_array_interface->color_channel_count]);
            else if (LedArray::led_sequence.bit_depth == 1)
              LedArray::led_sequence.append(led_number, true);
          }
        }
        else // Normal LED value
        {
          // Append this LED to the sequence
          if (LedArray::led_sequence.bit_depth == 8)
            LedArray::led_sequence.append(led_numbers[led_argument_index + 1], values[led_argument_index]);
          else if (LedArray::led_sequence.bit_depth == 1)
            LedArray::led_sequence.append(led_numbers[led_argument_index + 1], true);
        }
      }

      if (debug >= 0)
        LedArray::led_sequence.print(LedArray::led_sequence.number_of_patterns_assigned - 1);
    }
  }
  else if (pattern_led_count == 0)
  {
    // Add blank pattern
    LedArray::led_sequence.incriment(0);
  }
  else
  {
    Serial.print(F("Error (LedArray::setSequenceValue) - invalid number of arguments (should be divisible by ")); Serial.print(led_array_interface->color_channel_count); Serial.print(SERIAL_LINE_ENDING);
  }
}

void LedArray::printSequence()
{
  Serial.print(F("Sequence has ")); Serial.print(LedArray::led_sequence.length); Serial.print("x "); Serial.print(LedArray::led_sequence.bit_depth); Serial.printf(F(" bit patterns:%s"), SERIAL_LINE_ENDING);
  LedArray::led_sequence.print();
}

void LedArray::printSequenceLength()
{
  // Print current NA
  clearOutputBuffers();
  sprintf(output_buffer_short, "SL.%d", LedArray::led_sequence.length);
  sprintf(output_buffer_long, "Sequence length is: %d.", LedArray::led_sequence.length);
  print(output_buffer_short, output_buffer_long);
}

/* Reset stored sequence */
void LedArray::resetSequence()
{
  LedArray::pattern_index = 0;
}

void LedArray::runSequence(uint16_t argc, char ** argv)
{
  if (debug)
    print("START", "Starting Sequence...");

  /* Format for argv:
     0: delay between acquisitions, us/ms
     1: number of times to repeat pattern
     2: trigger output 1 setting
     3: trigger output 2 setting
     4: trigger input 1 setting
     5: trigger input 2 setting
  */

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;
  LedArray::trigger_input_mode_list[0] = 0;
  LedArray::trigger_input_mode_list[1] = 0;

  uint16_t delay_ms = 10;
  uint16_t acquisition_count = 1;

  // Print Argument syntax if no arguments are provided
  if (argc == 0)
    error(ERROR_CODE_ARGS_RUN_SEQUENCE, "LedArray::runSequence");

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      delay_ms  = strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      acquisition_count  = strtoul(argv[1], NULL, 0);
    else if (argc_index >= 2 && argc_index < 4)
      LedArray::trigger_output_mode_list[argc_index - 2] = atoi(argv[argc_index]);
    else if (argc_index >= 4 && argc_index < 6)
      LedArray::trigger_input_mode_list[argc_index - 4] = atoi(argv[argc_index]);
    else
      Serial.printf("WARNING:  Ignoring additional argument in runSequence%s", SERIAL_LINE_ENDING);
  }

  if (debug)
  {
    Serial.printf("OPTIONS:%s", SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  acquisition_count: ");
    Serial.print(acquisition_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 1: ");
    Serial.print(LedArray::trigger_output_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 1: ");
    Serial.print(LedArray::trigger_input_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return;
  }

  // Clear LED Array
  led_array_interface->clear();
  led_array_interface->update();

  // Initialize variables
  uint16_t led_number;
  bool result = true;

  elapsedMicros elapsed_us_outer;

  for (uint16_t frame_index = 0; frame_index < acquisition_count; frame_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < LedArray::led_sequence.number_of_patterns_assigned; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
        return;

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->clear();

      // Define pattern
      for (uint16_t led_idx = 0; led_idx < LedArray::led_sequence.led_counts[pattern_index]; led_idx++)
      {
        led_number = LedArray::led_sequence.led_list[pattern_index][led_idx];
        if (LedArray::led_sequence.bit_depth == 1)
          for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
            led_array_interface->setLed(led_number, color_channel_index, led_value[color_channel_index]);
        else
          for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
            led_array_interface->setLed(led_number, color_channel_index, uint8_t(round(float(led_value[color_channel_index]) * float(LedArray::led_sequence.values[pattern_index][led_idx]) / 255.0)));
      }

      // Check if led_count is zero - if so, clear the array
      if (LedArray::led_sequence.led_counts[pattern_index] == 0)
        led_array_interface->clear();

      // Update pattern
      led_array_interface->update();

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms))
      {
        Serial.printf(F("Error - delay too short!%s"), SERIAL_LINE_ENDING);
        return;
      }

      // Sent output trigger pulses
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && pattern_index == 0)))
        {
          sendTriggerPulse(trigger_index, false);
          if (LedArray::trigger_start_delay_list_us[trigger_index] > 0)
            delayMicroseconds(LedArray::trigger_start_delay_list_us[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured)
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
        {
          result = waitForTriggerState(trigger_index, true);
          if (!result)
          {
            return;
          }
        }
      }

      // Wait for the defined mininum amount of time (delay_ms) before checking trigger input state
      while ((float)elapsed_us_inner < (1000 * (float)delay_ms)) {} // Wait until this is true

      // Wait for all devices to stop acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
          result = waitForTriggerState(trigger_index, false);
        if (!result)
        {
          return;
        }
      }

      if (debug)
      {
        Serial.print(F("Elapsed time: "));
        Serial.print((float)elapsed_us_outer);
        Serial.printf(F("us %s"), SERIAL_LINE_ENDING);
      }
    }
  }

  led_array_interface->clear();
  led_array_interface->update();

  // Let user know we're done
  Serial.printf("Finished sending sequence.%s", SERIAL_LINE_ENDING);
}

void LedArray::patternIncrementFast()
{
  noInterrupts();

  // Display pattern
  if (LedArray::pattern_index <  LedArray::led_sequence.number_of_patterns_assigned && LedArray::led_sequence.led_counts[LedArray::pattern_index] > 0)
  {
    // Clear array for all LED pins
    digitalWriteFast(5, HIGH);
    digitalWriteFast(6, HIGH);
    digitalWriteFast(9, HIGH);
    digitalWriteFast(10, HIGH);
  }
  else
  {
    // Clear array for all LED pins
    digitalWriteFast(5, LOW);
    digitalWriteFast(6, LOW);
    digitalWriteFast(9, LOW);
    digitalWriteFast(10, LOW);
  }

  // Increment pattern
  LedArray::pattern_index++;

  // Enable interrupts
  interrupts();
}

void LedArray::runSequenceFast(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.printf(F("Starting fast Sequence %s"), SERIAL_LINE_ENDING);

  /* Format for argv:
     0: delay between acquisitions, us
     1: delay between frames us
     2: number of times to repeat pattern
     3: trigger output 0 setting
     4: trigger output 1 setting
     5: trigger input 0 setting
     6: trigger input 1 setting
     7: trigger min dt 0
     7: trigger min dt 1
  */

  float frame_delay_us = 0;
  float pattern_delay_us = 0;
  uint16_t acquisition_count = 1;

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;
  LedArray::trigger_input_mode_list[0] = 0;
  LedArray::trigger_input_mode_list[1] = 0;

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      pattern_delay_us = (float)strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      frame_delay_us = (float)strtoul(argv[1], NULL, 0);
    else if (argc_index == 2)
      acquisition_count  = strtoul(argv[2], NULL, 0);
    else if (argc_index >= 3 && argc_index < 5)
      LedArray::trigger_output_mode_list[argc_index - 3] = atoi(argv[argc_index]);
    else if (argc_index >= 5 && argc_index < 7)
      LedArray::trigger_input_mode_list[argc_index - 5] = atoi(argv[argc_index]);
    else
      Serial.printf("WARNING:  Ignoring additional argument in runSequence %s", SERIAL_LINE_ENDING);
  }

  if (debug >= 2)
  {
    Serial.printf("OPTIONS: %s", SERIAL_LINE_ENDING);
    Serial.printf(" Pattern delay (us): %f %s", pattern_delay_us, SERIAL_LINE_ENDING);
    Serial.printf(" Frame delay (us): %f %s", frame_delay_us, SERIAL_LINE_ENDING);
    Serial.printf(" Acquisition count: %f %s", acquisition_count, SERIAL_LINE_ENDING);
    Serial.printf(" Trigger %d: Output mode: %d, Input mode: %d %s", 0, LedArray::trigger_output_mode_list[0], LedArray::trigger_input_mode_list[0], SERIAL_LINE_ENDING);
    Serial.printf(" Trigger %d: Output mode: %d, Input mode: %d %s", 1, LedArray::trigger_output_mode_list[1], LedArray::trigger_input_mode_list[1], SERIAL_LINE_ENDING);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((pattern_delay_us < (float)MIN_SEQUENCE_DELAY_FAST))
  {
    Serial.print("ERROR: Pattern delay (");
    Serial.print(pattern_delay_us);
    Serial.print("us) was shorter than MIN_SEQUENCE_DELAY_FAST (");
    Serial.print(MIN_SEQUENCE_DELAY_FAST);
    Serial.print("us).");
    Serial.print(SERIAL_LINE_ENDING);
    return;
  }

  // Determine number of trigger pulses and create an array for storing trigger timing information
  uint16_t trigger_count = 0;
  for (uint8_t trigger_pin_index = 0; trigger_pin_index < led_array_interface->trigger_output_count; trigger_pin_index++)
  {
    if (LedArray::trigger_output_mode_list[trigger_pin_index] == TRIG_MODE_ITERATION)
      trigger_count += acquisition_count;
    else if (LedArray::trigger_output_mode_list[trigger_pin_index] == TRIG_MODE_START)
      trigger_count += 1;
    else
      trigger_count += ceil(acquisition_count * LedArray::led_sequence.number_of_patterns_assigned / LedArray::trigger_output_mode_list[trigger_pin_index]);
  }

  if (debug >= 2)
    Serial.printf(F("Sending %d trigger pulses this acquisition %s"), trigger_count, SERIAL_LINE_ENDING);

  // Clear LED Array
  led_array_interface->clear();

  // Initialize variables
  elapsedMicros elapsed_us_outer;
  bool triggered[2] = {false, false};
  float trigger_spin_up_delay_list[led_array_interface->trigger_output_count];
  int trigger_finished_waiting_count = 0;
  uint32_t max_start_delay_us = 0;
  bool triggers_used_this_pattern;

  // Clear LED Array
  led_array_interface->clear();

  // Set sequence and pattern indicies to zero
  LedArray::pattern_index = 0;
  LedArray::frame_index = 0;

  // Create interrupt-based intervaltimer object for precise illumination timing
  IntervalTimer itimer;

  // Store initial time
  float elapsed_us_start = (float) elapsed_us_outer;
  float elapsed_us_trigger;
  bool result = true;

  while (LedArray::frame_index < acquisition_count)
  {
    // Wait until we exceed timing for this frame
    if (((float) elapsed_us_outer - elapsed_us_start) >= (float)LedArray::frame_index * frame_delay_us)
    {
      // Determine whether trigger pulses are sent this pattern
      triggers_used_this_pattern = false;
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (LedArray::pattern_index  % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index  == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && LedArray::pattern_index  == 0)))
        {
          triggers_used_this_pattern = true;
        }
      }

      if (triggers_used_this_pattern)
      {
        // Clear array
        led_array_interface->clear();

        // Wait for all devices to be ready to acquire (if input triggers are configured
        max_start_delay_us = 0;
        for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
        {
          if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index  % LedArray::trigger_input_mode_list[trigger_index] == 0))
              || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index  == 0))
              || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && LedArray::pattern_index  == 0)))
          {
            result = waitForTriggerState(trigger_index, false);
            if (!result)
            {
              return;
            }
          }

          // Set the wait list to inactive state
          trigger_spin_up_delay_list[trigger_index] = -0.1;
        }

        // Set trigger state to true by default
        for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
          triggered[trigger_index] = true;

        // Determine max start delay using trigger output settings and wait for defined amount before continuing
        for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
        {
          if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (LedArray::pattern_index  % LedArray::trigger_output_mode_list[trigger_index] == 0))
              || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index  == 0))
              || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && LedArray::pattern_index  == 0)))
          {
            // Indicate that trigger pulses ARE used this pattern
            triggers_used_this_pattern = true;

            // Set triggered flag to indicate that we need to send this trigger pulse this iteration
            triggered[trigger_index] = false;

            // Store the max start delay, which is the maximum time before t0 to start sending triggers (to allow for spin-up, such as acceleration for a motion stage)
            if (max_start_delay_us < (LedArray::trigger_start_delay_list_us[trigger_index] + LedArray::trigger_pulse_width_list_us[trigger_index]))
              max_start_delay_us = LedArray::trigger_start_delay_list_us[trigger_index] + LedArray::trigger_pulse_width_list_us[trigger_index];
          }
        }

        // Send output trigger pulses before illuminating
        elapsed_us_trigger = (float) elapsed_us_outer;
        while (((float)elapsed_us_outer - elapsed_us_trigger) < (float)max_start_delay_us)
        {
          for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
          {
            if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (LedArray::pattern_index  % LedArray::trigger_output_mode_list[trigger_index] == 0))
                || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index  == 0))
                || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && LedArray::pattern_index  == 0)))
            {
              // Check that we are waiting the correct amount of time
              if (!triggered[trigger_index])
              {
                // Start triggers LedArray::trigger_start_delay_list_us BEFORE t0, which occurs at the end of this loop
                if (((float)elapsed_us_outer - elapsed_us_trigger) > (float)(max_start_delay_us - LedArray::trigger_start_delay_list_us[trigger_index] - LedArray::trigger_pulse_width_list_us[trigger_index])) // check if we're ready to trigger
                {
                  led_array_interface->sendTriggerPulse(trigger_index, LedArray::trigger_pulse_width_list_us[trigger_index], false);
                  triggered[trigger_index] = true;
                }
              }
            }
          }
          if (((float)elapsed_us_outer - elapsed_us_trigger) > 5000000)
          {
            Serial.print(F("ERROR (LedArray::runSequenceFast): Dropping out output trigger loop. "));
            for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
              Serial.printf(" | Trigger %d has triggered state %d", trigger_index, (int)triggered[trigger_index]);
            Serial.printf("%s", SERIAL_LINE_ENDING);
            return;
          }
        }

        // Wait for all devices to be in an acquiring state (if input triggers are configured)
        elapsed_us_trigger = (float) elapsed_us_outer;
        trigger_finished_waiting_count = 0;

        while (trigger_finished_waiting_count < led_array_interface->trigger_input_count)
        {
          for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
          {
            if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index  % LedArray::trigger_input_mode_list[trigger_index] == 0))
                || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index  == 0))
                || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && LedArray::pattern_index  == 0)))
            {
              if (trigger_spin_up_delay_list[trigger_index] < 0)
              {
                if (getTriggerState(trigger_index))
                {
                  trigger_spin_up_delay_list[trigger_index] = max((float)elapsed_us_outer - elapsed_us_trigger, 0.0);
                  trigger_finished_waiting_count += 1;
                }
              }
              else
              {
                trigger_spin_up_delay_list[trigger_index] = 0.0;
                trigger_finished_waiting_count += 1;
              }

            }
            else
            {
              if (trigger_spin_up_delay_list[trigger_index] < 0)
              {
                trigger_spin_up_delay_list[trigger_index] = 0.0;
                trigger_finished_waiting_count += 1;
              }
            }
          }
          if (((float)elapsed_us_outer - elapsed_us_trigger) > 5000000)
          {
            Serial.print(F("ERROR (LedArray::runSequenceFast): Dropping out of acquiring state wait loop. "));
            for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
              Serial.printf(" | Trigger %d has input triggered state %d", trigger_index, (int)triggered[trigger_index]);
            Serial.printf("%s", SERIAL_LINE_ENDING);
            return;
          }
        }
      }

      // If this is the first frame, start the timing counter from when the first trigger pulses are sent.
      // This eleminates all issues with the first trigger pulse being longer (e.g. from motion state spin-up)
      if (LedArray::frame_index == 0)
        elapsed_us_start = (float) elapsed_us_outer;

      // Run sequence
      itimer.priority(0);
      itimer.begin(patternIncrementFast, pattern_delay_us);
      itimer.priority(0);

      if (LedArray::pattern_index > LedArray::led_sequence.number_of_patterns_assigned)
        return;
      else
        while (LedArray::pattern_index <= LedArray::led_sequence.number_of_patterns_assigned) {}

      // Stop sequence
      itimer.end();

      // Reset pattern
      LedArray::pattern_index = 0;

      // Check to ensure we haven't exceeded the time between frames. If we have, return which will cause errors downstream.
      if (((float) elapsed_us_outer - elapsed_us_start) >= (float)(LedArray::frame_index + 1) * frame_delay_us)
      {
        Serial.printf(F("ERROR (LedArray::runSequenceFast) Trigger process time (%f) exceeded frame delay (%f) for frame %d %s"),
                      ((float) elapsed_us_outer - elapsed_us_start), (float)(LedArray::frame_index + 1) * frame_delay_us, LedArray::frame_index, SERIAL_LINE_ENDING);
        return;
      }

      // Increment frame index
      LedArray::frame_index++;
    }
  }
  Serial.printf(F("Finished fast Sequence %s"), SERIAL_LINE_ENDING);
}

void LedArray::stepSequence(uint16_t argc, char ** argv)
{
  Serial.printf(F("Stepping sequence %s"), SERIAL_LINE_ENDING);

  /* Format for argv:
     0: trigger output 1 setting
     1: trigger output 2 setting
     2: trigger input 1 setting
     3: trigger input 2 setting
  */

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;
  LedArray::trigger_input_mode_list[0] = 0;
  LedArray::trigger_input_mode_list[1] = 0;

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index >= 0 && argc_index < 2)
      LedArray::trigger_output_mode_list[argc_index] = atoi(argv[argc_index]);
    else if (argc_index >= 2 && argc_index < 4)
      LedArray::trigger_input_mode_list[argc_index - 2] = atoi(argv[argc_index]);
    else
      Serial.printf("WARNING:  Ignoring additional argument in stepSequence%s", SERIAL_LINE_ENDING);
  }

  if (debug)
  {
    Serial.printf("OPTIONS: %s", SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 1: ");
    Serial.print(LedArray::trigger_output_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 1: ");
    Serial.print(LedArray::trigger_input_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
  }

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;

  // Loop sequence counter if it's at the end
  if (LedArray::pattern_index >= LedArray::led_sequence.number_of_patterns_assigned)
    LedArray::pattern_index = 0;

  uint16_t led_number;
  bool result = true;

  // Sent output trigger pulses before illuminating
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
  {
    if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
        || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index == 0))
        || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (LedArray::pattern_index == 0)))
    {
      sendTriggerPulse(trigger_index, false);

      if (LedArray::trigger_start_delay_list_us[trigger_index] > 0)
        delayMicroseconds(LedArray::trigger_start_delay_list_us[trigger_index]);
    }
  }

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
  {
    if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
        || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index == 0))
        || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (LedArray::pattern_index == 0)))
    {
      result = waitForTriggerState(trigger_index, true);
      if (!result)
      {
        return;
      }
    }
  }

  elapsedMicros elapsed_us_inner;

  // Clear the array
  led_array_interface->clear();

  // Send LEDs
  for (uint16_t led_idx = 0; led_idx < LedArray::led_sequence.led_counts[LedArray::pattern_index]; led_idx++)
  {
    led_number = LedArray::led_sequence.led_list[LedArray::pattern_index][led_idx];
    
    if (LedArray::led_sequence.bit_depth == 1)
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_number, color_channel_index, led_value[color_channel_index]);
    else
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(led_number, color_channel_index, LedArray::led_sequence.values[LedArray::pattern_index][led_idx]);
  }

  // Update pattern
  led_array_interface->update();

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
  {
    result = true;
    if ((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
      result = waitForTriggerState(trigger_index, true);
    else if ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index == 0))
      result = waitForTriggerState(trigger_index, true);
    else if ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (LedArray::pattern_index == 0))
      result = waitForTriggerState(trigger_index, true);

    if (!result)
    {
      return;
    }
  }

  // Incriment counter
  LedArray::pattern_index++;

  // Print user feedback
  Serial.print(F("Displayed pattern # "));
  Serial.print(LedArray::pattern_index);
  Serial.print(F(" of "));
  Serial.print( LedArray::led_sequence.number_of_patterns_assigned);
  Serial.print(SERIAL_LINE_ENDING);
}

void LedArray::runSequenceDpc(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.printf(F("Starting sequence.%s"), SERIAL_LINE_ENDING);

  /* Format for argv:
     0: delay between acquisitions, us/ms
     1: number of times to repeat pattern
     2: trigger output 1 setting
     3: trigger output 2 setting
     4: trigger input 1 setting
     5: trigger input 2 setting
  */

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;
  LedArray::trigger_input_mode_list[0] = 0;
  LedArray::trigger_input_mode_list[1] = 0;

  uint16_t delay_ms = 500;
  uint16_t acquisition_count = 1;

  // Print Argument syntax if no arguments are provided
  if (argc == 0)
    Serial.printf(F("ERROR (LedArray::runSequence): Wrong number of arguments. Syntax: rseq.[frame dt,ms],[# acquisitions],[trigger output mode 0], [trigger input mode 0], ...%s"), SERIAL_LINE_ENDING);

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      delay_ms  = strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      acquisition_count  = strtoul(argv[1], NULL, 0);
    else if (argc_index >= 2 && argc_index < 4)
      LedArray::trigger_output_mode_list[argc_index - 2] = atoi(argv[argc_index]);
    else if (argc_index >= 4 && argc_index < 6)
      LedArray::trigger_input_mode_list[argc_index - 4] = atoi(argv[argc_index]);
    else
      Serial.printf("WARNING:  Ignoring additional argument in runDpcSequence%s", SERIAL_LINE_ENDING);
  }

  if (debug)
  {
    Serial.printf("OPTIONS:%s", SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  acquisition_count: ");
    Serial.print(acquisition_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 1: ");
    Serial.print(LedArray::trigger_output_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 1: ");
    Serial.print(LedArray::trigger_input_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return;
  }

  // Clear LED Array
  led_array_interface->clear();
  led_array_interface->update();

  elapsedMicros elapsed_us_outer;
  bool result = true;

  for (uint16_t frame_index = 0; frame_index < acquisition_count; frame_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < 4; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
        return;

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->clear();

      // Draw half circle
      drawHalfCircle(pattern_index, 0.0, objective_na);

      // Update pattern
      led_array_interface->update();

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms))
      {
        Serial.printf(F("Error - delay too short!%s"), SERIAL_LINE_ENDING);
        return;
      }

      // Sent output trigger pulses
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && pattern_index == 0)))
        {
          sendTriggerPulse(trigger_index, false);
          if (LedArray::trigger_start_delay_list_us[trigger_index] > 0)
            delayMicroseconds(LedArray::trigger_start_delay_list_us[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured)
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
        {
          result = waitForTriggerState(trigger_index, true);
          if (!result)
          {
            return;
          }
        }
      }

      // Wait for the defined mininum amount of time (delay_ms) before checking trigger input state
      while ((float)elapsed_us_inner < (1000 * (float)delay_ms)) {} // Wait until this is true

      // Wait for all devices to stop acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
          result = waitForTriggerState(trigger_index, false);
        if (!result)
        {
          return;
        }
      }

      if (debug)
      {
        Serial.print(F("Elapsed time: "));
        Serial.print((float)elapsed_us_outer);
        Serial.printf(F("us %s"), SERIAL_LINE_ENDING);
      }
    }
  }

  led_array_interface->clear();
  led_array_interface->update();

  // Let user know we're done
  Serial.printf("Finished sending sequence.%s", SERIAL_LINE_ENDING);
}

void LedArray::runSequenceFpm(uint16_t argc, char ** argv)
{
  if (debug)
    Serial.printf(F("Starting sequence.%s"), SERIAL_LINE_ENDING);

  /* Format for argv:
     0: delay between acquisitions, us/ms
     1: number of times to repeat pattern
     2: maximum na
     3: trigger output 1 setting
     4: trigger output 2 setting
     5: trigger input 1 setting
     6: trigger input 2 setting
  */

  // Reset Trigger parameters
  LedArray::trigger_output_mode_list[0] = 0;
  LedArray::trigger_output_mode_list[1] = 0;
  LedArray::trigger_input_mode_list[0] = 0;
  LedArray::trigger_input_mode_list[1] = 0;

  uint16_t delay_ms = 40;
  uint16_t acquisition_count = 1;
  float maximum_na = 1.0;

  // Print Argument syntax if no arguments are provided
  if (argc == 0)
    Serial.printf(F("ERROR (LedArray::runSequence): Wrong number of arguments. Syntax: rseq.[frame dt,ms],[# acquisitions],[trigger output mode 0], [trigger input mode 0], ...%s"), SERIAL_LINE_ENDING);

  for (int argc_index = 0; argc_index < argc; argc_index++)
  {
    if (argc_index == 0)
      delay_ms  = strtoul(argv[0], NULL, 0);
    else if (argc_index == 1)
      acquisition_count  = strtoul(argv[1], NULL, 0);
    else if (argc_index == 2)
      maximum_na = atof(argv[2]) / 100.0;
    else if (argc_index >= 3 && argc_index < 5)
      LedArray::trigger_output_mode_list[argc_index - 3] = atoi(argv[argc_index]);
    else if (argc_index >= 5 && argc_index < 7)
      LedArray::trigger_input_mode_list[argc_index - 5] = atoi(argv[argc_index]);
    else
      Serial.printf("WARNING:  Ignoring additional argument in runDpcSequence%s", SERIAL_LINE_ENDING);
  }

  if (debug)
  {
    Serial.printf("OPTIONS:%s", SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  acquisition_count: ");
    Serial.print(acquisition_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  maximum na: ");
    Serial.print(maximum_na);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 1: ");
    Serial.print(LedArray::trigger_output_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 1: ");
    Serial.print(LedArray::trigger_input_mode_list[1]);
    Serial.print(SERIAL_LINE_ENDING);
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return;
  }

  // Clear LED Array
  led_array_interface->clear();
  led_array_interface->update();

  // Determine maximum LED number to use
  uint16_t max_led_index = 0;
  float d;
  for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    d = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);

    if (d <= maximum_na)
      max_led_index = led_index;
  }

  elapsedMicros elapsed_us_outer;
  bool result = true;

  for (uint16_t frame_index = 0; frame_index < acquisition_count; frame_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < max_led_index; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
        return;

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->clear();

      // Turn on one LED
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_array_interface->setLed(pattern_index, color_channel_index, led_value[color_channel_index]);

      // Update pattern
      led_array_interface->update();

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms))
      {
        Serial.printf(F("Error - delay too short!%s"), SERIAL_LINE_ENDING);
        return;
      }

      // Sent output trigger pulses
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == 0 && pattern_index == 0)))
        {
          sendTriggerPulse(trigger_index, false);
          if (LedArray::trigger_start_delay_list_us[trigger_index] > 0)
            delayMicroseconds(LedArray::trigger_start_delay_list_us[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured)
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
        {
          result = waitForTriggerState(trigger_index, true);
          if (!result)
          {
            return;
          }
        }
      }

      // Wait for the defined mininum amount of time (delay_ms) before checking trigger input state
      while ((float)elapsed_us_inner < (1000 * (float)delay_ms)) {} // Wait until this is true

      // Wait for all devices to stop acquiring (if input triggers are configured
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (frame_index == acquisition_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
          result = waitForTriggerState(trigger_index, false);
        if (!result)
        {
          return;
        }
      }

      if (debug)
      {
        Serial.print(F("Elapsed time: "));
        Serial.print((float)elapsed_us_outer);
        Serial.printf(F("us %s"), SERIAL_LINE_ENDING);
      }
    }
  }

  led_array_interface->clear();
  led_array_interface->update();

  // Let user know we're done
  Serial.printf("Finished sending sequence.%s", SERIAL_LINE_ENDING);
}

/* A function to set the distance from the sample to the LED array. Used for calculating the NA of each LED.*/
void LedArray::setArrayDistance(int argc, char ** argv)
{
  if (argc == 0)
    ; // do nothing, just display current z-distance
  else if (argc == 1)
  {
    uint32_t new_z = strtoul(argv[0], NULL, 0);
    if (new_z > 0)
    {
      led_array_distance_z = (float)new_z;
      buildNaList(led_array_distance_z);
    }
    else
      Serial.printf(F("ERROR (LedArray::setDistanceZ): invalid z-distance.%s"), SERIAL_LINE_ENDING);
  }
  else
    Serial.printf(F("ERROR (LedArray::setDistanceZ): wrong number of arguments.%s"), SERIAL_LINE_ENDING);

  // Print current Array Distance
  clearOutputBuffers();
  sprintf(output_buffer_short, "DZ.%d", (uint8_t)round(led_array_distance_z));
  sprintf(output_buffer_long, "Current array distance from sample is %dmm.", (uint8_t)round(led_array_distance_z));
  print(output_buffer_short, output_buffer_long);
}

void LedArray::toggleAutoClear(uint16_t argc, char ** argv)
{
  if (argc == 0)
    auto_clear_flag = !auto_clear_flag;
  else
    auto_clear_flag = (bool)atoi(argv[0]);

  if (auto_clear_flag)
    print("AC.1", "Auto clear bit is now 1 (The LED array will clear before and after each new command)");
  else
    print("AC.0", "Auto clear bit is now 0 (The LED array will NOT clear before and after each new command)");
}

void LedArray::setDebug(uint16_t new_debug_level)
{
  if (new_debug_level > 10)
  {
    // Set debug level for this file
    debug = (int) ((new_debug_level % 100 - new_debug_level % 10) / 10.0);
  }
  else
    debug = new_debug_level;

  // User feedback
  Serial.printf(F("(LedArray::setDebug): Set debug level to %d \n"), debug);

  print("AC.1", "Auto clear bit is now 1 (The LED array will clear before and after each new command)");

  // Set debug level for interface
  led_array_interface->setDebug((int) (new_debug_level % 10));
}

void LedArray::setInterface(LedArrayInterface * interface)
{
  led_array_interface = interface;
}

void LedArray::setup()
{
  // If setup has been run before, deallocate previous arrays to avoid memory leaks
  if (!initial_setup)
  {
    delete[] LedArray::trigger_pulse_width_list_us;
    delete[] LedArray::trigger_start_delay_list_us;
    delete[] LedArray::trigger_output_mode_list;
    delete[] LedArray::trigger_input_mode_list;
    delete[] led_value;
    delete[] led_color;
  }


  // Read device mac address once
  read_mac();

  // Initialize led array
  led_array_interface->deviceSetup();

  // Set z-distance to device default
  led_array_distance_z = led_array_interface->led_array_distance_z_default;

  // Initialize output trigger settings
  LedArray::trigger_pulse_width_list_us = new uint32_t [led_array_interface->trigger_output_count];
  LedArray::trigger_start_delay_list_us = new uint32_t [led_array_interface->trigger_output_count];
  LedArray::trigger_output_mode_list = new int [led_array_interface->trigger_output_count];
  for (uint16_t trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
  {
    LedArray::trigger_pulse_width_list_us[trigger_index] = TRIGGER_PULSE_WIDTH_DEFAULT;
    LedArray::trigger_start_delay_list_us[trigger_index] = TRIGGER_DELAY_DEFAULT;
    LedArray::trigger_output_mode_list[trigger_index] = 0;
  }

  // Set up trigger pins
  LedArray::trigger_input_mode_list = new int [led_array_interface->trigger_input_count];
  for (int trig_input_pin = 0; trig_input_pin < led_array_interface->trigger_input_count; trig_input_pin++)
    pinMode(led_array_interface->trigger_input_pin_list[trig_input_pin], INPUT);

  // Define led_value and led_color
  led_brightness = LED_BRIGHTNESS_DEFAULT;
  led_value = new uint8_t[led_array_interface->color_channel_count];
  led_color = new uint8_t[led_array_interface->color_channel_count];

  // Populate led_color and led_value
  for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    led_color[color_channel_index] = (uint8_t)(round((float)UINT8_MAX / led_array_interface->color_channel_count)) ; // TODO: make this respect bit depth

  // Reset sequence
  LedArray::led_sequence.deallocate();

  // Initialize sequences at every bit depth so these are defined
  LedArray::led_sequence.allocate(7);
  LedArray::led_sequence.incriment(1);
  LedArray::led_sequence.append(0, 127);
  LedArray::led_sequence.incriment(0);
  LedArray::led_sequence.incriment(1);
  LedArray::led_sequence.append(1, 127);
  LedArray::led_sequence.incriment(0);
  LedArray::led_sequence.incriment(1);
  LedArray::led_sequence.append(2, 127);
  LedArray::led_sequence.incriment(0);
  LedArray::led_sequence.incriment(1);
  LedArray::led_sequence.append(3, 127);

  // Build list of LED NA coordinates
  buildNaList(led_array_distance_z);

  // Define default NA
  objective_na = led_array_interface->default_na;

  // Indicate that setup has run
  initial_setup = false;

  // Run demo mode if EEPROM indicates we should
  if (getDemoMode())
    demo();

}

void LedArray::demo()
{
  // Set demo mode flag
  setDemoMode(true);

  // Run until key received
  while (getDemoMode())
  {

    // Demo Brightfield patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;

      led_value[color_channel_index_outer] = 64;
      led_array_interface->clear();
      drawCircle(0, objective_na);
      led_array_interface->update();
      delay(250);

      // Break loop if key is pressed
      if (Serial.available())
      {
        setDemoMode(false);
        clear();
        return;
      }
    }

    // Demo Annulus patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;

      led_value[color_channel_index_outer] = 64;
      led_array_interface->clear();
      drawCircle(objective_na, objective_na + 0.2);
      led_array_interface->update();
      delay(250);

      // Break loop if key is pressed
      if (Serial.available())
      {
        setDemoMode(false);
        clear();
        return;
      }
    }

    // Demo DPC Patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int dpc_index = 0; dpc_index < 4; dpc_index++)
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_value[color_channel_index] = 0;

        led_value[color_channel_index_outer] = 127;
        led_array_interface->clear();
        drawHalfCircle(dpc_index, 0, objective_na);
        led_array_interface->update();
        delay(250);

        // Break loop if key is pressed
        if (Serial.available())
        {
          setDemoMode(false);
          clear();
          return;
        }

      }
    }

    for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      led_array_interface->setLed(-1, -1, (uint8_t)0);
      led_array_interface->setLed(led_index, -1, (uint8_t)127);
      led_array_interface->update();
      delay(1);

      // Break loop if key is pressed
      if (Serial.available())
      {
        setDemoMode(false);
        clear();
        return;
      }
    }


    for ( int16_t led_index = led_array_interface->led_count - 1; led_index >= 0; led_index--)
    {
      led_array_interface->setLed(-1, -1, (uint8_t)0);
      led_array_interface->setLed(led_index, -1, (uint8_t)127);
      led_array_interface->update();
      delay(1);

      // Break loop if key is pressed
      if (Serial.available())
      {
        setDemoMode(false);
        clear();
        return;
      }
    }
    delay(100);
  }
}

void LedArray::setBaudRate(uint16_t argc, char ** argv)
{
  if (argc > 0)
  {
    uint32_t new_baud_rate = (strtoul((char *) argv[0], NULL, 0));
    led_array_interface->setBaudRate(new_baud_rate);
  }
  Serial.printf(F("Current serial baud rate is %.3f MHz %s"), (double)led_array_interface->getBaudRate() / 1000000.0, SERIAL_LINE_ENDING);
}

void LedArray::setGsclkFreq(uint16_t argc, char ** argv)
{
  if (argc > 0)
  {
    uint32_t new_gsclk_frequency = (strtoul((char *) argv[0], NULL, 0));
    led_array_interface->setGsclkFreq(new_gsclk_frequency);
  }

  Serial.printf(F("Current grayscale clock frequency is %.3f MHz %s"), (double)led_array_interface->getGsclkFreq() / 1000000.0, SERIAL_LINE_ENDING);
}

void LedArray::setCommandMode(const char * mode)
{
  if (!strcmp(mode, "short") || !strcmp(mode, "machine"))
  {
    command_mode = COMMAND_MODE_SHORT;
    print("Command output set to machine-readable", "Command output set to machine-readable");
  }
  else
  {
    command_mode = COMMAND_MODE_LONG;
    print("Command output set to human-readable", "Command output set to human-readable");
  }
}

void LedArray::notImplemented(const char * command_name)
{
  // Long form
  char buf_long[100];
  sprintf(buf_long, "ERROR: Command %s is not implemented for this device.", command_name);

  // Short form
  char buf_short[100];
  sprintf(buf_short, "ERR: %d", ERROR_CODE_NOT_IMPLEMENTED);

  // Print
  print(buf_short, buf_long);
}

int LedArray::getColorChannelCount()
{
  return led_array_interface->color_channel_count;
}

void LedArray::print(const char * short_output, const char * long_output)
{
  if (command_mode == COMMAND_MODE_SHORT)
    Serial.printf("%s%s", short_output, SERIAL_LINE_ENDING);
  else
    Serial.printf("%s%s", long_output, SERIAL_LINE_ENDING);
}

void LedArray::clearOutputBuffers()
{
  memset(&output_buffer_short, ' ', sizeof(output_buffer_short));
  memset(&output_buffer_long, ' ', sizeof(output_buffer_long));
}

void LedArray::error(int16_t error_code, const char * calling_function)
{
  if (error_code < ERROR_CODE_COUNT)
  {
    if (command_mode == COMMAND_MODE_SHORT)
      Serial.printf("ERROR.%s%s", calling_function, error_code_list[error_code][0], SERIAL_LINE_ENDING);
    else
      Serial.printf("ERROR (%s): %s%s", calling_function, error_code_list[error_code][1], SERIAL_LINE_ENDING);
  }
}

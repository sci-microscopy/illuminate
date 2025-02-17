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
      Neither the name of the UC Berkley nor thei
      derived from this software without specific prior written permission.demo

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
#include "illuminate.h"

volatile uint16_t LedArray::pattern_index = 0;

volatile float LedArray::trigger_input_timeout = 60; // Seconds
volatile uint32_t * LedArray::trigger_output_pulse_width_list_us;
volatile uint32_t * LedArray::trigger_output_start_delay_list_us;
volatile int * LedArray::trigger_input_mode_list;
volatile int * LedArray::trigger_output_mode_list;
volatile bool * LedArray::trigger_input_polarity_list;
volatile bool * LedArray::trigger_output_polarity_list;
LedSequence LedArray::led_sequence;

uint8_t LedArray::get_device_command_count()
{
  return led_array_interface->get_device_command_count();
}

const char * LedArray::get_device_command_name_short(int device_command_index)
{
  return led_array_interface->get_device_command_name_short(device_command_index);
}
const char * LedArray::get_device_command_name_long(int device_command_index)
{
  return led_array_interface->get_device_command_name_long(device_command_index);
}

int LedArray::device_command(int device_command_index, uint16_t argc, char * *argv)
{
  // Get pattern sizes (stored as a 32-bit integer with first 16 as leds per pattern pattern and second as pattern count
  uint32_t concatenated = led_array_interface->get_device_command_led_list_size(device_command_index);
  uint16_t pattern_count  = (uint16_t)(concatenated >> 16);
  uint16_t leds_per_pattern = (uint16_t)concatenated;

  // Get arguments
  int16_t pattern_number = 0;
  if (argc == 1)
    pattern_number = -1;
  else if (argc == 2)
    pattern_number = strtoul(argv[1], NULL, 0);
  else
  {
    Serial.printf("ERROR (LedArray::device_command) Invalid number of arguments (%d) %s", argc, SERIAL_LINE_ENDING);
    return ERROR_ARGUMENT_COUNT;
  }

  if (auto_clear_flag)
    led_array_interface->clear();

  if (pattern_number < 0)
  {
    for (int16_t pattern_index = 0; pattern_index < pattern_count; pattern_index++)
    {
      for (int16_t led_index = 0; led_index < leds_per_pattern; led_index++)
      {
        if (debug_level >= 3)
          Serial.printf("Pattern %d contains led %d %s", pattern_index, led_array_interface->get_device_command_led_list_element(device_command_index, pattern_index, led_index), SERIAL_LINE_ENDING);

        for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
          set_led(led_array_interface->get_device_command_led_list_element(device_command_index, pattern_index, led_index), color_channel_index, led_value[color_channel_index]);
      }
    }
  }
  else
  {
    for (int16_t led_index = 0; led_index < leds_per_pattern; led_index++)
    {
      if (debug_level >= 3)
        Serial.printf("Pattern %d contains led %d %s", pattern_index, led_array_interface->get_device_command_led_list_element(device_command_index, pattern_index, led_index), SERIAL_LINE_ENDING);

      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        set_led(led_array_interface->get_device_command_led_list_element(device_command_index, pattern_number, led_index), color_channel_index, led_value[color_channel_index]);
    }
  }

  // Update pattern
  led_array_interface->update();

  return NO_ERROR;
}

/* A function to print current LED positions (xyz) */
int LedArray::print_led_positions(uint16_t argc, char * *argv, bool print_na)
{
  // Parse arguments
  uint16_t start_index = 0;
  uint16_t end_index = led_array_interface->led_count;
  if (argc == 2)
    start_index = strtoul(argv[1], NULL, 0);
  else if (argc == 3)
  {
    start_index = strtoul(argv[1], NULL, 0);
    end_index = strtoul(argv[2], NULL, 0);
  }

  // Initialize working variables
  int16_t led_number;
  float na_x, na_y, x, y, z;

  if (print_na)
  {
    build_na_list(led_array_distance_z);
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
  Serial.printf(F("    }%s}%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);

  return NO_ERROR;
}

/* A function to print current LED values */
int LedArray::print_led_values(uint16_t argc, char * *argv)
{
  // Parse arguments
  uint16_t start_index = 0;
  uint16_t end_index = led_array_interface->led_count;
  if (argc == 2)
    start_index = strtoul(argv[1], NULL, 0);
  else if (argc == 3)
  {
    start_index = strtoul(argv[1], NULL, 0);
    end_index = strtoul(argv[2], NULL, 0);
  }

  int16_t led_number;
  Serial.printf(F("{\n    \"led_values\" : {%s"), SERIAL_LINE_ENDING);
  for (uint16_t led_index = start_index; led_index < end_index; led_index++)
  {
    led_number = (int16_t)pgm_read_word(&(LedArrayInterface::led_positions[led_index][0]));

    Serial.printf(F("        \"%d\" : ["), led_number);
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    {
      Serial.printf(F("%u"), led_array_interface->get_led_value(led_number, color_channel_index));

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
  return NO_ERROR;
}

/* A function to the version of this device */
int LedArray::print_version(uint16_t argc, char * *argv)
{
  Serial.print(VERSION);
  Serial.print(SERIAL_LINE_ENDING);
  return NO_ERROR;
}

/* A function to print a human-readable about page */
int LedArray::print_about(uint16_t argc, char * *argv)
{
  Serial.printf("====================================================================================================%s", SERIAL_LINE_ENDING);
  Serial.print("  ");
  Serial.print(led_array_interface->device_name);
  Serial.printf(F(" LED Array Controller %s"), SERIAL_LINE_ENDING);
  Serial.print(F("  Illuminate r"));
  Serial.print(VERSION);
  Serial.print(F(" | Serial Number: "));
  Serial.printf("%04d", get_serial_number());
  Serial.print(F(" | Part Number: "));
  Serial.printf("%04d", get_part_number());
  Serial.print(F(" | Teensy MAC address: "));
  print_mac_address();
  Serial.printf(F("\n  For help, type ? %s"), SERIAL_LINE_ENDING);
  Serial.printf("====================================================================================================%s", SERIAL_LINE_ENDING);

  return NO_ERROR;
}

/* A function to print a json-formatted file which contains relevant system parameters */
int LedArray::print_system_parameters(uint16_t argc, char * *argv)
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
  Serial.print(F(",\n    \"color_channel_center_wavelengths_nm\" : {"));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(", "));
    Serial.print('\"');
    Serial.print(LedArrayInterface::color_channel_names[channel_index]);
    Serial.print('\"');
    Serial.printf(" : %.3f", LedArrayInterface::color_channel_center_wavelengths_nm[channel_index]);
  }
  Serial.print(F("},\n    \"color_channel_fwhm_wavelengths_nm\" : {"));
  for (int channel_index = 0; channel_index < led_array_interface->color_channel_count; channel_index++)
  {
    if (channel_index > 0)
      Serial.print(F(", "));
    Serial.print('\"');
    Serial.print(LedArrayInterface::color_channel_names[channel_index]);
    Serial.print('\"');
    Serial.printf(" : %.3f", LedArrayInterface::color_channel_fwhm_wavelengths_nm[channel_index]);
  }
  Serial.print(F("},\n    \"trigger_input_count\" : "));
  Serial.print(led_array_interface->trigger_input_count);
  Serial.print(F(",\n    \"trigger_output_count\" : "));
  Serial.print(led_array_interface->trigger_output_count);
  Serial.print(F(",\n    \"bit_depth\" : "));
  Serial.print(led_array_interface->bit_depth);
  Serial.print(F(",\n    \"serial_number\" : "));
  Serial.print(get_serial_number());
  Serial.print(F(",\n    \"color_channel_count\" : "));
  Serial.print(led_array_interface->color_channel_count);
  Serial.print(F(",\n    \"part_number\" : "));
  Serial.print(get_part_number());
  Serial.print(F(",\n    \"mac_address\" : \""));
  print_mac_address();
  Serial.print(F("\""));
  Serial.print(F(",\n    \"interface_version\" : "));
  Serial.print(VERSION);

  // Terminate JSON
  Serial.printf("\n}", SERIAL_LINE_ENDING);

  return NO_ERROR;
}

int LedArray::set_max_current_limit(uint16_t argc, char ** argv)
{
  if (argc == 2)
    led_array_interface->set_max_current_limit(atof(argv[1]));
  else if (argc == 1)
    ;
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "smc.%.2f", led_array_interface->get_max_current_limit());
  sprintf(output_buffer_long, "Maximum current limit (Amps): %.2f", led_array_interface->get_max_current_limit());
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_max_current_enforcement(uint16_t argc, char ** argv)
{
  if (argc == 2)
    led_array_interface->set_max_current_enforcement(atoi(argv[1]) > 0);
  else if (argc == 1)
    ;
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "smce.%d", led_array_interface->get_max_current_enforcement());
  sprintf(output_buffer_long, "Maximum current enforced: %d", led_array_interface->get_max_current_enforcement());
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::print_mac_address()
{
  uint8_t mac[6];

  static char teensyMac[23];

#if defined HW_OCOTP_MAC1 && defined HW_OCOTP_MAC0
  //    Serial.println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
  for (uint8_t by = 0; by < 2; by++) mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++) mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;

#define MAC_OK

#else

  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;

  uint32_t SN = 0;
  __disable_irq();

#if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
  //      Serial.println("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");

  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  FTFL_FCCOB0 = 0x41;
  FTFL_FCCOB1 = 15;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
  SN = *(uint32_t *)&FTFL_FCCOB7;

#define MAC_OK

#elif defined(HAS_KINETIS_FLASH_FTFE)
  //      Serial.println("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");

  kinetis_hsrun_disable();
  FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
  *(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
  FTFL_FSTAT = FTFL_FSTAT_CCIF;
  while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
  SN = *(uint32_t *)&FTFL_FCCOBB;
  kinetis_hsrun_enable();

#define MAC_OK

#endif

  __enable_irq();

  for (uint8_t by = 0; by < 3; by++) mac[by + 3] = (SN >> ((2 - by) * 8)) & 0xFF;

#endif

#ifdef MAC_OK
  sprintf(teensyMac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(teensyMac);
#else
  return ERROR_MAC_ADDRESS
#endif

  return NO_ERROR;
}


int LedArray::set_demo_mode(uint16_t argc, char ** argv)
{
  if (argc == 1)
    set_demo_mode(true);
  else if (argc == 2)
    set_demo_mode(atoi(argv[1]));
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "DEMO.%05d", get_demo_mode());
  sprintf(output_buffer_long, "Demo mode is: %05d", get_demo_mode());
  print(output_buffer_short, output_buffer_long);

  // If demo mode is true, go ahead and run the demo
  if (get_demo_mode())
    run_demo();

  return NO_ERROR;
}

int LedArray::print_serial_number(uint16_t argc, char ** argv)
{

  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "SN.%05d", get_serial_number());
  sprintf(output_buffer_long, "Serial number is: %05d", get_serial_number());
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::print_part_number(uint16_t argc, char ** argv)
{

  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "PN.%05d", get_part_number());
  sprintf(output_buffer_long, "Part number is: %05d", get_part_number());
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

/* A function to reset the device to power-on state */
int LedArray::reset(uint16_t argc, char ** argv)
{
  // Print current SN
  clear_output_buffers();
  sprintf(output_buffer_short, "RESET");
  sprintf(output_buffer_long, "Resetting LED Array...");
  print(output_buffer_short, output_buffer_long);

  return led_array_interface->device_reset();
}

/* A function to draw a random "disco" pattern. For parties, mostly. */
int LedArray::disco()
{
  // Determine number of LEDs to illuminate at once
  int led_on_count = (int)round(led_array_interface->led_count / 4.0);

  // Clear the array
  clear();

  // Clear Serial buffer
  while (Serial.available())
    Serial.read();

  // Party time
  while (Serial.available() == 0)
  {
    led_array_interface->clear();

    for (uint16_t led_index = 0; led_index < led_on_count; led_index++)
    {
      led_index = random(0, led_array_interface->led_count);
      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        set_led(led_index, color_channel_index, (uint8_t)random(0, 255));
    }
    led_array_interface->update();
    delay(10);
  }

  // Clear the array
  clear();

  return NO_ERROR;
}

/* A function to draw a water drop (radial sine pattern)*/
int LedArray::water_drop()
{
  // Clear the array
  clear();

  float na_period = LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][0] * LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][0];
  na_period += LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][1] * LedArrayInterface::led_position_list_na[led_array_interface->led_count - 1][1];
  na_period = sqrt(na_period) / 4.0;

  uint8_t value;
  float na;
  uint8_t max_led_value = 16;
  uint8_t phase_counter = 0;

  // Clear Serial buffer
  while (Serial.available())
    Serial.read();

  while (Serial.available() == 0)
  {
    // Clear array
    set_led(-1, -1, false);
    for (uint16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      na = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);
      value = (uint8_t)round(0.5 * (1.0 + sin(((na / na_period) + ((float)phase_counter / 100.0)) * 2.0 * 3.14)) * max_led_value);
      for (int color_channel_index = 0; color_channel_index <  led_array_interface->color_channel_count; color_channel_index++)
        set_led(led_index, color_channel_index, value);
    }
    led_array_interface->update();
    delay(1);
    phase_counter++;
    if (phase_counter == 100)
      phase_counter = 0;
  }

  // Clear the array
  clear();

  return NO_ERROR;
}


/* A function to calculate the NA of each LED given the XYZ position and an offset */
void LedArray::build_na_list(float new_board_distance)
{
  float Na_x, Na_y, yz, xz, x, y, z, z0;

  if (new_board_distance > 0)
    led_array_distance_z = new_board_distance;

  max_na = 0;
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

      // Calculate max NA
      max_na = max(max_na, sqrt(Na_x * Na_x + Na_y * Na_y));
    }
    else
    {
      LedArrayInterface::led_position_list_na[led_index][0] = INVALID_NA; // invalid NA
      LedArrayInterface::led_position_list_na[led_index][1] = INVALID_NA; // invalid NA
    }
  }
  if (debug_level)
    Serial.printf(F("Finished updating led positions."));
}

/* A function to fill the LED array with the color specified by led_value */
int LedArray::fill_array()
{
  // Turn on all LEDs
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      set_led(led_index, color_channel_index, led_value[color_channel_index]);
  }

  // Update array
  led_array_interface->update();

  if (debug_level)
    Serial.printf(F("Filled Array%s"), SERIAL_LINE_ENDING);

  return NO_ERROR;
}

/* A function to clear the LED array */
int LedArray::clear()
{
  led_array_interface->clear();
  led_array_interface->update();
  return NO_ERROR;
}

/* A function to set the numerical aperture of the system*/
int LedArray::set_na(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ; // do nothing, just display current na
  else if (argc == 2)
  {
    float new_objective_na = (float)atoi(argv[1]);
    new_objective_na = (float) new_objective_na / 100.0;
    if (strlen(argv[1]) == 1)
      new_objective_na = new_objective_na * 10.0;

    if ((new_objective_na > 0) && new_objective_na <= 1.0)
      objective_na = new_objective_na;
    else
      return ERROR_ARGUMENT_RANGE;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current NA
  clear_output_buffers();
  sprintf(output_buffer_short, "NA.%02d", (uint8_t) round(objective_na * 100));
  sprintf(output_buffer_long, "Current NA is %.02f", objective_na);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

/* A function to set the numerical aperture of the system*/
int LedArray::set_inner_na(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ; // do nothing, just display current na
  else if (argc == 2)
  {
    float new_inner_na = (float)atoi(argv[1]);
    new_inner_na = (float) new_inner_na / 100.0;
    if (strlen(argv[1]) == 1)
      new_inner_na = new_inner_na * 10.0;

    if ((new_inner_na > 0) && new_inner_na < 1.0)
      inner_na = new_inner_na;
    else
      return ERROR_ARGUMENT_RANGE;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current NA
  clear_output_buffers();
  sprintf(output_buffer_short, "NAI.%02d", (uint8_t) round(inner_na * 100));
  sprintf(output_buffer_long, "Current Inner NA is %.02f", inner_na);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_cosine_factor(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ; // do nothing, just display current na
  else if (argc == 2)
    cosine_factor = (uint8_t)atoi(argv[1]);
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Cosine Factor
  clear_output_buffers();
  sprintf(output_buffer_short, "COS.%d", cosine_factor);
  sprintf(output_buffer_long, "Current Cosine Factor is %d.", cosine_factor);
  print(output_buffer_short, output_buffer_long);
  return NO_ERROR;
}

/* A function to draw a darkfield pattern */
int LedArray::draw_darkfield(uint16_t argc, char * *argv)
{
  if (auto_clear_flag)
    clear();

  draw_primative_circle(objective_na, 1.0);
  led_array_interface->update();

  return NO_ERROR;
}

/* A function to draw a cDPC pattern */
int LedArray::draw_cdpc(uint16_t argc, char * *argv)
{
  if (led_array_interface->color_channel_count != 3)
  {
    return ERROR_NOT_SUPPORTED_BY_DEVICE;
  }
  else
  {
    uint8_t illumination_intensity;

    if (argc == 1)
      illumination_intensity = 127;
    else if (argc == 2)
      illumination_intensity = (uint8_t)atoi(argv[1]);
    else
    {
      return ERROR_ARGUMENT_COUNT;
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
          draw_primative_quadrant(quadrant_index, inner_na, objective_na, true);
        }
      }
    }
    led_array_interface->update();
  }
  return NO_ERROR;
}

/* A function to draw a half annulus */
int LedArray::draw_half_annulus(uint16_t argc, char * *argv)
{
  float na_start = objective_na;
  float na_end = objective_na + 0.2;
  int8_t pattern_index = -1;
  float angle_deg = 0.0;
  if (argc == 1)
  {
    pattern_index = 0;
  }
  else if ((argc == 2) || (argc == 4))
  {
    if ( (strcmp(argv[1], DPC_TOP1) == 0) || (strcmp(argv[1], DPC_TOP2) == 0))
      pattern_index = 0;
    else if ( (strcmp(argv[1], DPC_BOTTOM1) == 0) || (strcmp(argv[1], DPC_BOTTOM2) == 0))
      pattern_index = 1;
    else if ( (strcmp(argv[1], DPC_LEFT1) == 0) || (strcmp(argv[1], DPC_LEFT2) == 0))
      pattern_index = 2;
    else if ( (strcmp(argv[1], DPC_RIGHT1) == 0) || (strcmp(argv[1], DPC_RIGHT2) == 0))
      pattern_index = 3;
    else
    {
      angle_deg = (float) strtol(argv[1], NULL, 0);
    }
  }
  else
    return ERROR_ARGUMENT_COUNT;

  if (argc == 4)
  {
    na_start = atof(argv[2]) / 100.0;
    na_end = atof(argv[3]) / 100.0;
  }

  if (debug_level >= 1)
  {
    Serial.print(F("Drawing half-annulus pattern with type: "));
    Serial.print(argv[1]);
    Serial.print(F(" from "));
    Serial.print(na_start);
    Serial.print(F("NA to "));
    Serial.print(na_end);
    Serial.printf(F("NA.%s"), SERIAL_LINE_ENDING);
  }

  if (auto_clear_flag)
    clear();

  if (pattern_index >= 0)
  {
    draw_primative_half_circle(dpc_pattern_angles[pattern_index], na_start, na_end);
    led_array_interface->update();
  }
  else
  {
    draw_primative_half_circle(angle_deg, na_start, na_end);
    led_array_interface->update();
  }

  return NO_ERROR;
}

/* A function to draw a color darkfield pattern */
int LedArray::draw_color_darkfield(uint16_t argc, char * * argv)
{
  if (led_array_interface->color_channel_count != 3)
  {
    return ERROR_NOT_IMPLEMENTED;
  }
  else
  {
    uint8_t illumination_intensity = 127;
    float start_na = objective_na;
    float end_na = objective_na + 0.2;

    if (argc == 1)
      ; // do nothing (use default values above)
    else if (argc >= 1)
      illumination_intensity = (uint8_t)atoi(argv[1]);
    else if (argc >= 2)
    {
      start_na = atof(argv[2]);
      end_na = min(start_na + 0.2, 1.0);
    }
    else if (argc >= 3)
    {
      start_na = atof(argv[2]);
      start_na = atof(argv[3]);
    }
    else
    {
      return ERROR_ARGUMENT_COUNT;
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
          draw_primative_quadrant(quadrant_index, start_na, end_na, true);
        }
      }
    }
    led_array_interface->update();
  }

  return NO_ERROR;
}

/* A function to draw an annulus*/
int LedArray::draw_annulus(uint16_t argc, char * * argv)
{
  float start_na, end_na;
  if (argc == 1)
  {
    start_na = objective_na;
    end_na = min(objective_na + 0.2, 1.0);
  }
  else if (argc == 3)
  {
    start_na = (float)atoi(argv[1]) / 100.0;
    end_na = (float)atoi(argv[2]) / 100.0;
  }
  else
  {
    return ERROR_ARGUMENT_COUNT;
  }

  if (debug_level >= 1)
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
  draw_primative_circle(start_na, end_na);
  led_array_interface->update();

  return NO_ERROR;
}

/* A function to draw a spoecific LED channel as indexed in hardware */
int LedArray::draw_channel(uint16_t argc, char * *argv)
{
  if (argc == 2)
  {
    if (auto_clear_flag)
      clear();

    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->set_channel(strtol(argv[1], NULL, 0), color_channel_index, led_value[color_channel_index]);

    led_array_interface->update();
  }
  else
    return ERROR_ARGUMENT_COUNT;
  return NO_ERROR;
}

/* A function to set the pin order of a LED (for multi-color designs */
int LedArray::set_pin_order(uint16_t argc, char * *argv)
{
  if (argc == led_array_interface->color_channel_count)
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->set_pin_order(-1, color_channel_index, strtoul(argv[color_channel_index], NULL, 0));
  else if (argc == led_array_interface->color_channel_count + 1)
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_array_interface->set_pin_order(strtoul(argv[1], NULL, 0) , color_channel_index, strtoul(argv[color_channel_index + 1], NULL, 0));
  else
    return ERROR_ARGUMENT_COUNT;

  return NO_ERROR;
}

int LedArray::set_global_shutter_state(uint16_t argc, char ** argv)
{
  if (argc == 2)
  {
    led_array_interface->set_global_shutter_state(bool(atoi(argv[1])));
  }
  else if (argc == 1)
  {
    ;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current shutter state
  clear_output_buffers();
  sprintf(output_buffer_short, "GS.%i", led_array_interface->get_global_shutter_state());
  sprintf(output_buffer_long, "Current global shutter state: %s", led_array_interface->get_global_shutter_state() ? "open" : "closed");
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_trigger_input_timeout(uint16_t argc, char ** argv)
{
  if (argc == 2)
  {
    uint16_t new_trigger_timeout_seconds = strtoul(argv[1], NULL, 0);
    if (new_trigger_timeout_seconds > 0)
      LedArray::trigger_input_timeout = (float)new_trigger_timeout_seconds;
  }
  else if (argc > 1)
    return ERROR_ARGUMENT_COUNT;

  // Print current brightness
  clear_output_buffers();
  sprintf(output_buffer_short, "TRINPUTTIMEOUT.%f", LedArray::trigger_input_timeout);
  sprintf(output_buffer_long, "Current trigger input timeout value is %f seconds.", LedArray::trigger_input_timeout);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_trigger_output_pulse_width(uint16_t argc, char ** argv)
{

  if ((argc == 2) && (led_array_interface->trigger_output_count == 1))
  {
    uint16_t new_trigger_pulse_width_us = strtoul(argv[1], NULL, 0);
    if (new_trigger_pulse_width_us > 0)
      LedArray::trigger_output_pulse_width_list_us[0] = new_trigger_pulse_width_us;
  }
  else if (argc == 3)
  {
    uint8_t trigger_index = atoi(argv[1]);
    uint16_t new_trigger_pulse_width_us = strtoul(argv[2], NULL, 0);
    if (new_trigger_pulse_width_us > 0)
      LedArray::trigger_output_pulse_width_list_us[trigger_index] = new_trigger_pulse_width_us;
  }
  else if (argc > 2)
    return ERROR_ARGUMENT_COUNT;

  // Print current pulse width
  if (led_array_interface->trigger_output_count == 1)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTPULSEWIDTH.%lu", LedArray::trigger_output_pulse_width_list_us[0]);
    sprintf(output_buffer_long, "Current trigger output pulse width is %lu microseconds.", LedArray::trigger_output_pulse_width_list_us[0]);
    print(output_buffer_short, output_buffer_long);
  }
  else if (led_array_interface->trigger_output_count == 2)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTPULSEWIDTH.%lu.%lu", LedArray::trigger_output_pulse_width_list_us[0], LedArray::trigger_output_pulse_width_list_us[1]);
    sprintf(output_buffer_long, "Current trigger output pulse widths are (%lu, %lu) microseconds.", LedArray::trigger_output_pulse_width_list_us[0], LedArray::trigger_output_pulse_width_list_us[1]);
    print(output_buffer_short, output_buffer_long);
  }
  else
    return ERROR_NOT_IMPLEMENTED;

  return NO_ERROR;
}

int LedArray::set_trigger_input_polarity(uint16_t argc, char ** argv)
{
  if ((argc == 2) && (led_array_interface->trigger_input_count == 1))
  {
    LedArray::trigger_input_polarity_list[0] = (bool)atoi(argv[1]);
  }
  else if (argc == 3)
  {
    uint8_t trigger_index = atoi(argv[1]);
    LedArray::trigger_input_polarity_list[trigger_index] = (bool)atoi(argv[2]);
  }
  else if (argc > 2)
    return ERROR_ARGUMENT_COUNT;

  // Print current polarity
  if (led_array_interface->trigger_input_count == 1)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TRINPUTPOLARITY.%d", LedArray::trigger_input_polarity_list[0]);
    sprintf(output_buffer_long, "Current trigger input polarity is %d.", LedArray::trigger_input_polarity_list[0]);
    print(output_buffer_short, output_buffer_long);
  }
  else if (led_array_interface->trigger_input_count == 2)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TRINPUTPOLARITY.%d.%d", LedArray::trigger_input_polarity_list[0], LedArray::trigger_input_polarity_list[1]);
    sprintf(output_buffer_long, "Current trigger input polarity is (%d, %d).", LedArray::trigger_input_polarity_list[0], LedArray::trigger_input_polarity_list[1]);
    print(output_buffer_short, output_buffer_long);
  }
  else
    return ERROR_NOT_IMPLEMENTED;

  return NO_ERROR;

}

int LedArray::set_trigger_output_polarity(uint16_t argc, char ** argv)
{
  if ((argc == 2) && (led_array_interface->trigger_output_count == 1))
  {
    LedArray::trigger_output_polarity_list[0] = (bool)atoi(argv[1]);
  }
  else if (argc == 3)
  {
    uint8_t trigger_index = atoi(argv[1]);
    LedArray::trigger_output_polarity_list[trigger_index] = (bool)atoi(argv[2]);
  }
  else if (argc > 2)
    return ERROR_ARGUMENT_COUNT;

  // Print current polarity
  if (led_array_interface->trigger_output_count == 1)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTPOLARITY.%d", LedArray::trigger_output_polarity_list[0]);
    sprintf(output_buffer_long, "Current trigger output polarity is %d.", LedArray::trigger_output_polarity_list[0]);
    print(output_buffer_short, output_buffer_long);
  }
  else if (led_array_interface->trigger_output_count == 2)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTPOLARITY.%d.%d", LedArray::trigger_output_polarity_list[0], LedArray::trigger_output_polarity_list[1]);
    sprintf(output_buffer_long, "Current trigger output polarity is (%d, %d).", LedArray::trigger_output_polarity_list[0], LedArray::trigger_output_polarity_list[1]);
    print(output_buffer_short, output_buffer_long);
  }
  else
    return ERROR_NOT_IMPLEMENTED;

  return NO_ERROR;
}

int LedArray::set_trigger_output_delay(uint16_t argc, char ** argv)
{
  if ((argc == 2) && (led_array_interface->trigger_output_count == 1))
  {
    uint16_t new_trigger_delay_us = strtoul(argv[1], NULL, 0);
    if (new_trigger_delay_us >= 0)
      LedArray::trigger_output_start_delay_list_us[0] = new_trigger_delay_us;
  }
  else if (argc == 3)
  {
    uint8_t trigger_index = atoi(argv[1]);
    uint16_t new_trigger_delay_us = strtoul(argv[2], NULL, 0);
    if (new_trigger_delay_us >= 0)
      LedArray::trigger_output_start_delay_list_us[trigger_index] = new_trigger_delay_us;
  }
  else if (argc > 2)
    return ERROR_ARGUMENT_COUNT;

  // Print current polarity
  if (led_array_interface->trigger_output_count == 1)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTDELAY.%lu", LedArray::trigger_output_start_delay_list_us[0]);
    sprintf(output_buffer_long, "Current trigger output delay is %lu.", LedArray::trigger_output_start_delay_list_us[0]);
    print(output_buffer_short, output_buffer_long);
  }
  else if (led_array_interface->trigger_output_count == 2)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "TROUTPUTDELAY.%lu.%lu", LedArray::trigger_output_start_delay_list_us[0], LedArray::trigger_output_start_delay_list_us[1]);
    sprintf(output_buffer_long, "Current trigger output delay is (%lu, %lu).", LedArray::trigger_output_start_delay_list_us[0], LedArray::trigger_output_start_delay_list_us[1]);
    print(output_buffer_short, output_buffer_long);
  }
  else
    return ERROR_NOT_IMPLEMENTED;

  return NO_ERROR;
}

int LedArray::get_trigger_input_pins(uint16_t argc, char ** argv)
{
  if (argc == 1)
  {
    if (led_array_interface->trigger_input_count == 0)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TRINPUTPIN.NONE");
      sprintf(output_buffer_long, "No trigger input pins on this device.");
      print(output_buffer_short, output_buffer_long);
    }
    else if (led_array_interface->trigger_input_count == 1)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TRINPUTPIN.%d", led_array_interface->trigger_input_pin_list[0]);
      sprintf(output_buffer_long, "Trigger input pin is %d", led_array_interface->trigger_input_pin_list[0]);
      print(output_buffer_short, output_buffer_long);
    }
    else if (led_array_interface->trigger_input_count == 2)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TRINPUTPIN.%d.%d", led_array_interface->trigger_input_pin_list[0], led_array_interface->trigger_input_pin_list[1]);
      sprintf(output_buffer_long, "Trigger input pins are (%d, %d)", led_array_interface->trigger_input_pin_list[0], led_array_interface->trigger_input_pin_list[1]);
      print(output_buffer_short, output_buffer_long);
    }
    else
      return ERROR_NOT_IMPLEMENTED;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  return NO_ERROR;
}

int LedArray::get_trigger_output_pins(uint16_t argc, char ** argv)
{
  if (argc == 1)
  {
    if (led_array_interface->trigger_output_count == 0)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TROUTPUTPIN.NONE");
      sprintf(output_buffer_long, "No trigger output pins on this device.");
      print(output_buffer_short, output_buffer_long);
    }
    else if (led_array_interface->trigger_output_count == 1)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TROUTPUTPIN.%d", led_array_interface->trigger_output_pin_list[0]);
      sprintf(output_buffer_long, "Trigger output pin is %d", led_array_interface->trigger_output_pin_list[0]);
      print(output_buffer_short, output_buffer_long);
    }
    else if (led_array_interface->trigger_output_count == 2)
    {
      clear_output_buffers();
      sprintf(output_buffer_short, "TROUTPUTPIN.%d.%d", led_array_interface->trigger_output_pin_list[0], led_array_interface->trigger_output_pin_list[1]);
      sprintf(output_buffer_long, "Trigger output pins are (%d, %d)", led_array_interface->trigger_output_pin_list[0], led_array_interface->trigger_output_pin_list[1]);
      print(output_buffer_short, output_buffer_long);
    }
    else
      return ERROR_NOT_IMPLEMENTED;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  return NO_ERROR;
}

/* Trigger setup function for setting the trigger pulse width and delay after sending */
int LedArray::trigger_setup(uint16_t argc, char ** argv)
{
  int trigger_index = 0;
  uint32_t trigger_pulse_width_us = 0;
  uint32_t trigger_start_delay_ms = 0;

  if ((argc == 3) || (argc == 4)) {
    trigger_index = atoi(argv[1]);
    trigger_pulse_width_us = strtoul(argv[2], NULL, 0);
    if (argc >= 3)
      trigger_start_delay_ms = strtoul(argv[3], NULL, 0);

    if (trigger_pulse_width_us >= 0)
      LedArray::trigger_output_pulse_width_list_us[trigger_index] = trigger_pulse_width_us;

    if (trigger_start_delay_ms >= 0)
      LedArray::trigger_output_start_delay_list_us[trigger_index] = trigger_start_delay_ms;

    if (debug_level > 1)
    {
      Serial.print("Trigger ");
      Serial.print(trigger_index);
      Serial.print(" now has a pulse width of ");
      Serial.print(LedArray::trigger_output_pulse_width_list_us[trigger_index] );
      Serial.print("us and a start delay of ");
      Serial.print(LedArray::trigger_output_start_delay_list_us[trigger_index]);
      Serial.printf(F("us. %s"), SERIAL_LINE_ENDING);
    }
  }
  else if (argc == 1)
  {

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
                    LedArray::trigger_output_pulse_width_list_us[trigger_index],
                    LedArray::trigger_output_start_delay_list_us[trigger_index]);

      if (trigger_index < (led_array_interface->trigger_input_count - 1))
        Serial.printf(",%s", SERIAL_LINE_ENDING);
      else
        Serial.printf("%s", SERIAL_LINE_ENDING);
    }
    Serial.printf(F("    ]%s}%s"), SERIAL_LINE_ENDING, SERIAL_LINE_ENDING);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  return NO_ERROR;
}

/* Send a trigger pulse */
int LedArray::send_trigger_pulse(int trigger_index, bool show_output)
{
  if (debug_level >= 2)
    Serial.printf(F("Called send_trigger_pulse %s"), SERIAL_LINE_ENDING);

  if ((trigger_index < 0) || (trigger_index >= led_array_interface->trigger_output_count))
    return ERROR_INVALID_ARGUMENT;

  int status = led_array_interface->send_trigger_pulse(trigger_index, LedArray::trigger_output_pulse_width_list_us[trigger_index], false);

  if (status < 0)
    return ERROR_TRIGGER_CONFIG;

  return NO_ERROR;
}

/* Wait for a TTL trigger port to be in the given state */
bool LedArray::wait_for_trigger_state(int trigger_index, bool state)
{
  float delayed_us = 0;

  // Clear Serial buffer
  while (Serial.available())
    Serial.read();

  bool done = false;
  while (!done)
  {

    // Break the loop if we've received a serial command
    if (Serial.available())
    {
      while (Serial.available())
        Serial.read();
      Serial.printf(F("WARNING (LedArray::wait_for_trigger_state): Cancelling on pin %d due to serial interrupt %s"), trigger_index, SERIAL_LINE_ENDING);
      clear();
      return false;
    }

    // Break the loop if there's a timeout
    if (delayed_us > LedArray::trigger_input_timeout * 1000000.0)
    {
      Serial.printf(F("WARNING (LedArray::wait_for_trigger_state): Exceeding max delay for trigger input %d (%.2f sec.) %s"), trigger_index, LedArray::trigger_input_timeout, SERIAL_LINE_ENDING);
      return false;
    }

    // Do basic debouncing
    if (led_array_interface->trigger_input_state[trigger_index] == state)
    {
      done = true;
    }
    else
    {
      delayMicroseconds(INPUT_TRIGGER_WAIT_PERIOD_US);
      delayed_us += INPUT_TRIGGER_WAIT_PERIOD_US;
    }
  }
  return true;
}

int LedArray::trigger_input_test(uint16_t channel)
{
  set_led(-1, -1, (uint8_t)0);
  led_array_interface->update();
  Serial.print(LedArrayInterface::trigger_input_state[channel]); Serial.print(SERIAL_LINE_ENDING);
  Serial.print("Begin trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  bool result = wait_for_trigger_state(channel, !LedArrayInterface::trigger_input_state[channel]);
  if (result)
  {
    Serial.print("Passed trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  }
  else
  {
    Serial.print("Failed trigger input test for channel "); Serial.print(channel); Serial.print(SERIAL_LINE_ENDING);
  }
  set_led(-1, -1, (uint8_t)0);
  set_led(0, -1, (uint8_t)255);
  led_array_interface->update();
  return NO_ERROR;
}

int LedArray::draw_led_list(uint16_t argc, char ** argv)
{
  // Clear if desired
  if (auto_clear_flag)
    clear();

  // Parse inputs
  if (argc == 1)
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      set_led(0, color_channel_index, led_value[color_channel_index]);
  else
  {
    for (int arg_index = 1; arg_index < argc; arg_index++)
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        set_led(strtoul(argv[arg_index], NULL, 0), color_channel_index, led_value[color_channel_index]);
    led_array_interface->update();
  }

  return NO_ERROR;
}

int LedArray::run_sequence_individual_darkfield_leds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;
  uint16_t sequence_run_count = 1;
  if ((argc != 1) && (argc != 2) && (argc != 3))
    return ERROR_INVALID_ARGUMENT;
  if (argc >= 2)
    delay_ms = strtoul(argv[1], NULL, 0);
  if (argc == 3)
    sequence_run_count = strtoul(argv[2], NULL, 0);

  // Scan the LEDs
  return scan_led_range(delay_ms, objective_na, 1.0, true, sequence_run_count);
}

int LedArray::run_sequence_individual_brightfield_leds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;
  uint16_t sequence_run_count = 1;
  if ((argc != 1) && (argc != 2) && (argc != 3))
    return ERROR_INVALID_ARGUMENT;
  if (argc >= 2)
    delay_ms = strtoul(argv[1], NULL, 0);
  if (argc == 3)
    sequence_run_count = strtoul(argv[2], NULL, 0);

  return scan_led_range(delay_ms, inner_na, objective_na, true, sequence_run_count);
}

/* Scan all LEDs */
int LedArray::run_sequence_individual_leds(uint16_t argc, char ** argv)
{
  uint16_t delay_ms = 0;
  uint16_t sequence_run_count = 1;
  if ((argc != 1) && (argc != 2) && (argc != 3))
    return ERROR_INVALID_ARGUMENT;
  if (argc >= 2)
    delay_ms = strtoul(argv[1], NULL, 0);
  if (argc == 3)
    sequence_run_count = strtoul(argv[2], NULL, 0);

  return scan_led_range(delay_ms, 0.0, 1.0, true, sequence_run_count);
}

int LedArray::set_brightness(int16_t argc, char ** argv)
{

  if (argc == 1)
    ; // pass
  else
  {
    if (strcmp(argv[1], "max") == 0)
    {
      led_brightness = UINT8_MAX;
    }
    else if (strcmp(argv[1], "min") == 0)
    {
      led_brightness = 1;
    }
    else if (strcmp(argv[1], "half") == 0)
    {
      led_brightness = (uint8_t) ((float)UINT8_MAX / 2.0);
    }
    else if (strcmp(argv[1], "quarter") == 0)
    {
      led_brightness = (uint8_t) ((float)UINT8_MAX / 4.0);
    }
    else
      led_brightness = (uint8_t) strtoul(argv[1], NULL, 0);
  }

  // Set LED value based on color and brightness
  for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    led_value[color_channel_index] = (uint8_t) ceil((float) led_color[color_channel_index] / (float) UINT8_MAX * (float) led_brightness);

  // Print current brightness
  clear_output_buffers();
  sprintf(output_buffer_short, "SB.%u", led_brightness);
  sprintf(output_buffer_long, "Current brightness value is %u.", led_brightness);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

/* Allows setting of current color buffer, which is respected by most other commands */
int LedArray::set_single_color(int16_t argc, char ** argv)
{
  if (argc == 1)
    ; // Do nothing, print result
  else if (argc == 3)
  {
    uint8_t color_channel = atoi(argv[1]);
    uint8_t value = atoi(argv[2]);

    if ((color_channel >= 0) & (color_channel < led_array_interface->color_channel_count))
      led_value[color_channel] = value;
    else
      return ERROR_ARGUMENT_RANGE;

  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current color value
  clear_output_buffers();

  if (led_array_interface->color_channel_count == 3)
  {
    sprintf(output_buffer_short, "CO.%d.%d.%d", led_value[0], led_value[1], led_value[2]);
    sprintf(output_buffer_long, "Current color balance values are %d.%d.%d", led_value[0], led_value[1], led_value[2]);
    print(output_buffer_short, output_buffer_long);
  }
  else if (led_array_interface->color_channel_count == 1)
  {
    // Print single color
    sprintf(output_buffer_short, "CO.%d", led_value[0]);
    sprintf(output_buffer_long, "Current color balance values are %d.", led_value[0]);
    print(output_buffer_short, output_buffer_long);
  }

  return NO_ERROR;
}

/* Allows setting of current color buffer, which is respected by most other commands */
int LedArray::set_color(int16_t argc, char ** argv)
{
  if (led_array_interface->color_channel_count == 3)
  {
    if (argc == 1)
      ; // Do nothing
    else if (argc == 2)
    {
      if (strcmp(argv[1], "red") == 0  && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = default_brightness;
        led_color[1] = 0;
        led_color[2] = 0;
      }
      else if (strcmp(argv[1], "green") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = default_brightness;
        led_color[2] = 0;
      }
      else if (strcmp(argv[1], "blue") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = 0;
        led_color[1] = 0;
        led_color[2] = default_brightness;
      }
      else if (strcmp(argv[1], "white") == 0 && led_array_interface->color_channel_count == 3)
      {
        led_color[0] = default_brightness;
        led_color[1] = default_brightness;
        led_color[2] = default_brightness;
      }
      else
        return ERROR_INVALID_ARGUMENT;
    }
    else if (argc == (1 + led_array_interface->color_channel_count) && isdigit(argv[1][0]))
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      {
        led_color[color_channel_index] = (uint8_t)atoi(argv[1 + color_channel_index]);
      }
    }
    else
      return ERROR_INVALID_ARGUMENT;

    // Normalize color
    uint8_t max_value = max(max(led_color[0], led_color[1]), led_color[2]);
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_color[color_channel_index]  = (uint8_t) round(UINT8_MAX * ((float) led_color[color_channel_index]) / ((float) max_value));

    // Set LED value based on color and brightness
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      led_value[color_channel_index] = (uint8_t) (((float) led_color[color_channel_index] / UINT8_MAX) * (float) led_brightness);

    // Print current color value
    clear_output_buffers();

    sprintf(output_buffer_short, "CO.%d.%d.%d", led_color[0], led_color[1], led_color[2]);
    sprintf(output_buffer_long, "Current color balance values are %d.%d.%d", led_color[0], led_color[1], led_color[2]);
    print(output_buffer_short, output_buffer_long);
  }
  else
  {
    return ERROR_NOT_SUPPORTED_BY_DEVICE;
  }

  return NO_ERROR;
}

/* Draws a single quadrant of LEDs using standard quadrant indexing (top left is 0, moving clockwise) */
void LedArray::draw_primative_quadrant(int quadrant_number, float start_na, float end_na, bool include_center)
{
  if (debug_level >= 2)
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
          set_led(led_index, color_channel_index, led_value[color_channel_index]);
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
          set_led(led_index, color_channel_index, led_value[color_channel_index]);
      }
    }
  }
}

/* Draws a single half-circle of LEDs using standard quadrant indexing (top left is 0, moving clockwise) */
void LedArray::draw_primative_half_circle(float angle_deg, float start_na, float end_na)
{
  if (debug_level >= 2)
  {
    Serial.print(F("Drawing Half Annulus:"));
    Serial.print(angle_deg);
    Serial.print(SERIAL_LINE_ENDING);
  }

  float angle_rad = angle_deg / 180.0 * 3.14;

  float x, y, d, x_rotated, y_rotated;
  for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
  {
    // Get raw coordinates
    x = LedArrayInterface::led_position_list_na[led_index][0];
    y = LedArrayInterface::led_position_list_na[led_index][1];

    // Rotate
    x_rotated = round((cos(angle_rad) * x - sin(angle_rad) * y) * 100.0) / 100.0;
    y_rotated = round((sin(angle_rad) * x + cos(angle_rad) * y) * 100.0) / 100.0;
    d = sqrt(x_rotated * x_rotated + y_rotated * y_rotated);

    // Filter rotated coordinates
    if (d > (start_na) && (d <= (end_na)) && (y_rotated > 0.0))
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        set_led(led_index, color_channel_index, led_value[color_channel_index]);
    }
  }
}

/* Draws a circle or annulus of LEDs */
void LedArray::draw_primative_circle(float start_na, float end_na)
{
  if (debug_level >= 2)
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
        set_led(led_index, color_channel_index, led_value[color_channel_index]);
    }
  }
}

/* Scan brightfield LEDs */
int LedArray::scan_led_range(uint16_t delay_ms, float start_na, float end_na, bool print_indicies, uint16_t sequence_run_count)
{

  // Debug setting print
  if (debug_level)
  {
    Serial.printf(F("Starting sequence with settings:%s"), SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  sequence_run_count: ");
    Serial.print(sequence_run_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0 mode: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0 mode: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    if (led_array_interface->trigger_output_count > 1)
    {
      Serial.print("  trigger out 1 mode: ");
      Serial.print(LedArray::trigger_output_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
    if (led_array_interface->trigger_input_count > 1)
    {
      Serial.print("  trigger in 1 mode: ");
      Serial.print(LedArray::trigger_input_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY) && (delay_ms > 0))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }
  else if (delay_ms > MAX_SEQUENCE_DELAY)
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was greater than MAX_SEQUENCE_DELAY (");
    Serial.print(MAX_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }

  float d;

  // Clear array initially
  clear();

  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
  {
    if (LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START)
      send_trigger_pulse(trigger_index, false);
  }

  // Clear serial buffer
  while (Serial.available())
    Serial.read();

  if (print_indicies)
    Serial.print(F("scan_start:"));

  for (uint16_t sequence_index = 0; sequence_index < sequence_run_count; sequence_index++)
  {
    for (int16_t led_index = 0; led_index < (int16_t)led_array_interface->led_count; led_index++)
    {
      d = sqrt(LedArrayInterface::led_position_list_na[led_index][0] * LedArrayInterface::led_position_list_na[led_index][0] + LedArrayInterface::led_position_list_na[led_index][1] * LedArrayInterface::led_position_list_na[led_index][1]);
      if (d >= start_na && d <= end_na)
      {
        // Clear all LEDs
        led_array_interface->clear();

        // Set LEDs
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          set_led(led_index, color_channel_index, led_value[color_channel_index]);

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
            send_trigger_pulse(trigger_index, false);
        }

        // Delay for desired wait period
        delay(delay_ms);
      }
      if (Serial.available())
        break;
    }
  }

  if (print_indicies)
    Serial.print(F(":scan_end"));

  Serial.print(SERIAL_LINE_ENDING);

  delay(delay_ms);
  clear();

  return NO_ERROR;
}

/* Command parser for DPC */
int LedArray::draw_dpc(uint16_t argc, char ** argv)
{
  int8_t pattern_index = -1;
  float angle_deg = 0.0;
  if (argc == 1)
  {
    pattern_index = 0;
  }
  else if (argc == 2)
  {
    if ( (strcmp(argv[1], DPC_TOP1) == 0) || (strcmp(argv[1], DPC_TOP2) == 0))
      pattern_index = 0;
    else if ( (strcmp(argv[1], DPC_BOTTOM1) == 0) || (strcmp(argv[1], DPC_BOTTOM2) == 0))
      pattern_index = 1;
    else if ( (strcmp(argv[1], DPC_LEFT1) == 0) || (strcmp(argv[1], DPC_LEFT2) == 0))
      pattern_index = 2;
    else if ( (strcmp(argv[1], DPC_RIGHT1) == 0) || (strcmp(argv[1], DPC_RIGHT2) == 0))
      pattern_index = 3;
    else
    {
      angle_deg = (float) strtol(argv[1], NULL, 0);
    }
  }
  else
    return ERROR_ARGUMENT_COUNT;

  if (debug_level >= 1)
  {
    Serial.print(F("Drew DPC pattern with type: "));
    Serial.print(argv[1]); Serial.print(SERIAL_LINE_ENDING);
  }

  if (auto_clear_flag)
    led_array_interface->clear();


  if (pattern_index >= 0)
  {
    draw_primative_half_circle(dpc_pattern_angles[pattern_index], inner_na, objective_na);
    led_array_interface->update();
  }
  else
  {
    draw_primative_half_circle(angle_deg, inner_na, objective_na);
    led_array_interface->update();
  }

  return NO_ERROR;
}

/* Draw brightfield pattern */
int LedArray::draw_brightfield(uint16_t argc, char ** argv)
{
  if (debug_level)
    Serial.printf(F("Drawing brightfield pattern.%s"), SERIAL_LINE_ENDING);

  if (auto_clear_flag)
    clear();

  // Draw circle
  draw_primative_circle(inner_na, objective_na);
  led_array_interface->update();

  return NO_ERROR;
}

/* Draw quadrant pattern */
int LedArray::draw_quadrant(uint16_t argc, char ** argv)
{
  if (debug_level)
    Serial.printf(F("Drawing single quadrant pattern.%s"), SERIAL_LINE_ENDING);

  if (auto_clear_flag)
    clear();

  int quadrant_index = 0;
  if (argc == 1)
    ;
  else if (argc == 2)
    quadrant_index = atoi(argv[1]);
  else
    return ERROR_ARGUMENT_COUNT;

  // Draw circle
  draw_primative_quadrant(quadrant_index, inner_na, objective_na, true);
  led_array_interface->update();

  return NO_ERROR;
}

/* Set sequence length */
int LedArray::set_custom_sequence_length(uint16_t argc, char ** argv)
{

  // Check arguments
  if (argc == 1)
    ; // do nothing
  if (argc == 2)
  {
    // Reset old sequence
    LedArray::led_sequence.deallocate();

    // Initalize new sequence
    LedArray::led_sequence.allocate(strtoul(argv[1], NULL, 0));
  }
  else
    return ERROR_ARGUMENT_COUNT;

  clear_output_buffers();
  sprintf(output_buffer_short, "SEQ_LEN.%d", LedArray::led_sequence.length);
  sprintf(output_buffer_long, "Sequence length is now: %d.", LedArray::led_sequence.length);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

/* Set sequence value */
int LedArray::set_custom_sequence_value(uint16_t argc, char ** argv)
{

  if (argc < 2)
    return ERROR_ARGUMENT_COUNT;
  else
  {
    if (!strcmp(argv[1], "range"))
    {
      uint16_t range_start = strtoul(argv[2], NULL, 0);
      uint16_t range_end = strtoul(argv[3], NULL, 0);
      if (LedArray::led_sequence.increment(range_end - range_start))
        for (uint16_t index = range_start; index < range_end; index++)
          LedArray::led_sequence.append(index);
      else
        return ERROR_SEQUENCE_FULL;
    }
    else if (!strcmp(argv[1], "na_range"))
    {
      float na_range_start = (float)atoi(argv[2]) / 100.0;
      float na_range_end = (float)atoi(argv[3]) / 100.0;

      // Determine LED Count
      uint16_t led_count = 0;
      float led_na;
      for (uint16_t index = 0; index < led_array_interface->led_count; index++)
      {
        led_na = sqrt(led_array_interface->led_position_list_na[index][0] * led_array_interface->led_position_list_na[index][0] \
                      + led_array_interface->led_position_list_na[index][1] * led_array_interface->led_position_list_na[index][1]);
        if ((led_na > na_range_start) && (led_na < na_range_end))
          led_count++;
      }

      // Increment pattern and add LEDs
      if (LedArray::led_sequence.increment(led_count))
      {
        for (uint16_t index = 0; index < led_array_interface->led_count; index++)
        {
          led_na = sqrt(led_array_interface->led_position_list_na[index][0] * led_array_interface->led_position_list_na[index][0]
                        + led_array_interface->led_position_list_na[index][1] * led_array_interface->led_position_list_na[index][1]);
          if ((led_na > na_range_start) && (led_na < na_range_end))
            LedArray::led_sequence.append(index);
        }
      }
      else
        return ERROR_SEQUENCE_FULL;
    }
    else if (!strcmp(argv[1], "all"))
    {
      if (LedArray::led_sequence.increment(led_array_interface->led_count))
        for (uint16_t index = 0; index < led_array_interface->led_count; index++)
          LedArray::led_sequence.append(index);
      else
        return ERROR_SEQUENCE_FULL;
    }
    else if (!strcmp(argv[1], "none"))
    {
      if (!LedArray::led_sequence.increment(0))
        return ERROR_SEQUENCE_FULL;
    }
    else
    {
      if (LedArray::led_sequence.increment(argc - 1))
      {
        for (uint16_t index = 1; index < argc; index++)
          LedArray::led_sequence.append(strtoul(argv[index], NULL, 0));
      }
      else
        return ERROR_SEQUENCE_FULL;
    }
  }

  // Print current sequence length
  LedArray::led_sequence.print(LedArray::led_sequence.number_of_patterns_assigned - 1, command_mode);
  return NO_ERROR;
}

int LedArray::print_custom_sequence(uint16_t argc, char ** argv)
{

  // Print sequence
  LedArray::led_sequence.print(command_mode);

  return NO_ERROR;
}

/* Restart stored sequence */
int LedArray::restart_custom_sequence(uint16_t argc, char ** argv)
{
  // Set pattern index to zero
  LedArray::pattern_index = 0;

  // Clear array
  clear();

  return NO_ERROR;
}

int LedArray::run_custom_sequence(uint16_t argc, char ** argv)
{
  // Parse Arguments
  uint16_t delay_ms = 500;
  uint16_t sequence_run_count = 1;
  if ((argc != 1) && (argc != 2) && (argc != 3))
    return ERROR_INVALID_ARGUMENT;
  if (argc >= 2)
    delay_ms = strtoul(argv[1], NULL, 0);
  if (argc == 3)
    sequence_run_count = strtoul(argv[2], NULL, 0);

  // Debug setting print
  if (debug_level)
  {
    Serial.printf(F("Starting sequence with settings:%s"), SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  sequence_run_count: ");
    Serial.print(sequence_run_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0 mode: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0 mode: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    if (led_array_interface->trigger_output_count > 1)
    {
      Serial.print("  trigger out 1 mode: ");
      Serial.print(LedArray::trigger_output_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
    if (led_array_interface->trigger_input_count > 1)
    {
      Serial.print("  trigger in 1 mode: ");
      Serial.print(LedArray::trigger_input_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY) && (delay_ms > 0))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }
  else if (delay_ms > MAX_SEQUENCE_DELAY)
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was greater than MAX_SEQUENCE_DELAY (");
    Serial.print(MAX_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }

  // Clear LED Array
  clear();

  // Initialize variables
  uint16_t led_number;
  bool result = true;

  // Clear serial buffer
  while (Serial.available())
    Serial.read();

  elapsedMicros elapsed_us_outer;

  for (uint16_t sequence_index = 0; sequence_index < sequence_run_count; sequence_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < LedArray::led_sequence.number_of_patterns_assigned; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
      {
        led_array_interface->clear();
        return ERROR_COMMAND_INTERRUPTED;
      }

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      led_array_interface->clear();

      // Define pattern
      for (uint16_t led_idx = 0; led_idx < LedArray::led_sequence.led_counts[pattern_index]; led_idx++)
      {
        led_number = LedArray::led_sequence.led_list[pattern_index][led_idx];
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          set_led(led_number, color_channel_index, led_value[color_channel_index]);
      }

      // Check if led_count is zero - if so, clear the array
      if (LedArray::led_sequence.led_counts[pattern_index] == 0)
        led_array_interface->clear();

      // Update pattern
      led_array_interface->update();

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms) && (delay_ms > 0))
      {
        Serial.printf(F("Error - delay too short!%s"), SERIAL_LINE_ENDING);
        clear();
        return ERROR_SEQUENCE_DELAY;
      }

      // Sent output trigger pulses
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == 0 && pattern_index == 0)))
        {
          send_trigger_pulse(trigger_index, false);
          if (LedArray::trigger_output_start_delay_list_us[trigger_index] > 0)
            delayMicroseconds(LedArray::trigger_output_start_delay_list_us[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured)
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == sequence_run_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
        {
          result = wait_for_trigger_state(trigger_index, true);
          if (!result)
          {
            clear();
            return ERROR_TRIGGER_TIMEOUT;
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
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == sequence_run_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
          result = wait_for_trigger_state(trigger_index, false);
        if (!result)
        {
          clear();
          return ERROR_TRIGGER_TIMEOUT;
        }
      }

      if (debug_level)
      {
        Serial.print(F("Elapsed time: "));
        Serial.print((float)elapsed_us_outer);
        Serial.printf(F("us %s"), SERIAL_LINE_ENDING);
      }
    }
  }

  clear();

  // Let user know we're done
  if (debug_level)
    Serial.printf("Finished sending sequence.%s", SERIAL_LINE_ENDING);
  else
    Serial.print(SERIAL_LINE_ENDING);

  return NO_ERROR;
}


int LedArray::step_custom_sequence(uint16_t argc, char ** argv)
{
  Serial.printf(F("Stepping sequence %s"), SERIAL_LINE_ENDING);


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
      send_trigger_pulse(trigger_index, false);

      if (LedArray::trigger_output_start_delay_list_us[trigger_index] > 0)
        delayMicroseconds(LedArray::trigger_output_start_delay_list_us[trigger_index]);
    }
  }

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
  {
    if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
        || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index == 0))
        || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (LedArray::pattern_index == 0)))
    {
      result = wait_for_trigger_state(trigger_index, true);
      if (!result)
      {
        return ERROR_TRIGGER_TIMEOUT;
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
    for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
      set_led(led_number, color_channel_index, led_value[color_channel_index]);
  }

  // Update pattern
  led_array_interface->update();

  // Wait for all devices to start acquiring (if input triggers are configured
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
  {
    result = true;
    if ((LedArray::trigger_input_mode_list[trigger_index] > 0) && (LedArray::pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
      result = wait_for_trigger_state(trigger_index, true);
    else if ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (LedArray::pattern_index == 0))
      result = wait_for_trigger_state(trigger_index, true);
    else if ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (LedArray::pattern_index == 0))
      result = wait_for_trigger_state(trigger_index, true);

    if (!result)
    {
      return ERROR_TRIGGER_TIMEOUT;
    }
  }

  // increment counter
  LedArray::pattern_index++;

  // Print user feedback
  Serial.print(F("Displayed pattern # "));
  Serial.print(LedArray::pattern_index);
  Serial.print(F(" of "));
  Serial.print( LedArray::led_sequence.number_of_patterns_assigned);
  Serial.print(SERIAL_LINE_ENDING);

  return NO_ERROR;
}

int LedArray::run_sequence_dpc(uint16_t argc, char ** argv)
{
  // Parse Arguments
  uint16_t delay_ms = 500;
  uint16_t sequence_run_count = 1;
  if ((argc != 1) && (argc != 2) && (argc != 3))
    return ERROR_INVALID_ARGUMENT;
  if (argc >= 2)
    delay_ms = strtoul(argv[1], NULL, 0);
  if (argc == 3)
    sequence_run_count = strtoul(argv[2], NULL, 0);

  // Debug setting print
  if (debug_level)
  {
    Serial.printf(F("Starting sequence with settings:%s"), SERIAL_LINE_ENDING);
    Serial.print("  delay: ");
    Serial.print(delay_ms);
    Serial.print("ms\n  sequence_run_count: ");
    Serial.print(sequence_run_count);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger out 0 mode: ");
    Serial.print(LedArray::trigger_output_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print("  trigger in 0 mode: ");
    Serial.print(LedArray::trigger_input_mode_list[0]);
    Serial.print(SERIAL_LINE_ENDING);
    if (led_array_interface->trigger_output_count > 1)
    {
      Serial.print("  trigger out 1 mode: ");
      Serial.print(LedArray::trigger_output_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
    if (led_array_interface->trigger_input_count > 1)
    {
      Serial.print("  trigger in 1 mode: ");
      Serial.print(LedArray::trigger_input_mode_list[1]);
      Serial.print(SERIAL_LINE_ENDING);
    }
  }

  // Check to be sure we're not trying to go faster than the hardware will allow
  if ((delay_ms < MIN_SEQUENCE_DELAY) && (delay_ms > 0))
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was shorter than MIN_SEQUENCE_DELAY (");
    Serial.print(MIN_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }
  else if (delay_ms > MAX_SEQUENCE_DELAY)
  {
    Serial.print("ERROR: Sequance delay (");
    Serial.print(delay_ms);
    Serial.print("ms) was greater than MAX_SEQUENCE_DELAY (");
    Serial.print(MAX_SEQUENCE_DELAY);
    Serial.print("ms).");
    Serial.print(SERIAL_LINE_ENDING);
    return ERROR_SEQUENCE_DELAY;
  }

  // Clear LED Array
  clear();

  elapsedMicros elapsed_us_outer;
  bool result = true;

  // Clear serial buffer
  while (Serial.available())
    Serial.read();

  for (uint16_t sequence_index = 0; sequence_index < sequence_run_count; sequence_index++)
  {
    for (uint16_t pattern_index = 0; pattern_index < 4; pattern_index++)
    {
      // Return if we send any command to interrupt.
      if (Serial.available())
        return ERROR_COMMAND_INTERRUPTED;

      elapsedMicros elapsed_us_inner;

      // Set all LEDs to zero
      clear();

      // Draw half circle
      draw_primative_half_circle(dpc_pattern_angles[pattern_index], inner_na, objective_na);

      // Update pattern
      led_array_interface->update();

      // Ensure that we haven't set too short of a delay
      if ((float)elapsed_us_inner > (1000 * (float)delay_ms) && (delay_ms > 0))
      {
        Serial.printf(F("Error - delay too short!%s"), SERIAL_LINE_ENDING);
        return ERROR_SEQUENCE_DELAY;
      }

      // Sent output trigger pulses
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
      {
        if (((LedArray::trigger_output_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_output_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == 0))
            || ((LedArray::trigger_output_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == 0 && pattern_index == 0)))
        {
          send_trigger_pulse(trigger_index, false);
          if (LedArray::trigger_output_start_delay_list_us[trigger_index] > 0)
            delayMicroseconds(LedArray::trigger_output_start_delay_list_us[trigger_index]);
        }
      }

      // Wait for all devices to start acquiring (if input triggers are configured)
      for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
      {
        if (((LedArray::trigger_input_mode_list[trigger_index] > 0) && (pattern_index % LedArray::trigger_input_mode_list[trigger_index] == 0))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_ITERATION) && (pattern_index == LedArray::led_sequence.number_of_patterns_assigned))
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == sequence_run_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
        {
          result = wait_for_trigger_state(trigger_index, true);
          if (!result)
          {
            return ERROR_TRIGGER_TIMEOUT;
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
            || ((LedArray::trigger_input_mode_list[trigger_index] == TRIG_MODE_START) && (sequence_index == sequence_run_count && pattern_index == LedArray::led_sequence.number_of_patterns_assigned)))
          result = wait_for_trigger_state(trigger_index, false);
        if (!result)
        {
          return ERROR_TRIGGER_TIMEOUT;
        }
      }

      if (debug_level)
      {
        Serial.print(F("Elapsed time: "));
        Serial.print((float)elapsed_us_outer);
        Serial.printf(F("us %s"), SERIAL_LINE_ENDING);
      }
    }
  }

  clear();

  // Let user know we're done
  if (debug_level)
    Serial.printf("Finished sending sequence.%s", SERIAL_LINE_ENDING);
  else
    Serial.print(SERIAL_LINE_ENDING);
  return NO_ERROR;
}

/* A function to set the distance from the sample to the LED array. Used for calculating the NA of each LED.*/
int LedArray::set_array_distance(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ; // do nothing, just display current z-distance
  else if (argc == 2)
  {
    uint32_t new_z = strtoul(argv[1], NULL, 0);
    if (new_z > 0)
    {
      led_array_distance_z = (float)new_z;
      build_na_list(led_array_distance_z);
    }
    else
      return ERROR_ARGUMENT_RANGE;
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Array Distance
  clear_output_buffers();
  sprintf(output_buffer_short, "DZ.%d", (uint8_t)round(led_array_distance_z));
  sprintf(output_buffer_long, "Current array distance from sample is %dmm", (uint8_t)round(led_array_distance_z));
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_auto_clear(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ;
  else if (argc == 2)
    auto_clear_flag = (bool)atoi(argv[1]);
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Array Distance
  clear_output_buffers();
  sprintf(output_buffer_short, "AC.%d", auto_clear_flag);
  sprintf(output_buffer_long, "Auto clear mode: %d", auto_clear_flag);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_debug(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ; //
  else if (argc == 2)
  {
    debug_level = atoi(argv[1]);
    led_array_interface->set_debug(debug_level);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Array Distance
  clear_output_buffers();
  sprintf(output_buffer_short, "DBG.%d", debug_level);
  sprintf(output_buffer_long, "Debug level: %d", debug_level);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

void LedArray::set_interface(LedArrayInterface * interface)
{
  led_array_interface = interface;
}

int LedArray::print_power_supply_plugged(uint16_t argc, char ** argv)
{
  if (led_array_interface->is_power_source_plugged_in() == 1)
    print("PS.1", "Power Source is plugged in and functioning correctly.");
  else if (led_array_interface->is_power_source_plugged_in() == 0)
    print("PS.0", "Power Source is not plugged in. Device will not illuminate.");
  else
    print("PS.-1", "Power source sensing is not enabled on this device.");

  return NO_ERROR;
}

int LedArray::set_power_supply_sensing(uint16_t argc, char ** argv)
{

  if (!(led_array_interface->get_device_power_sensing_capability() == PSU_SENSING_AND_MONITORING))
    return ERROR_POWER_SENSING_NOT_SUPPORTED;

  if (argc == 1)
    ; //
  else if (argc == 2)
  {
    led_array_interface->set_power_source_monitoring_state(atoi(argv[1]) > 0);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current source voltage
  clear_output_buffers();
  sprintf(output_buffer_short, "PSENS.%d", led_array_interface->get_power_source_monitoring_state());
  sprintf(output_buffer_long, "Current power source sensing state: %d.", led_array_interface->get_power_source_monitoring_state());
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::print_power_supply_voltage(uint16_t argc, char ** argv)
{
  if (led_array_interface->get_device_power_sensing_capability() >= PSU_SENSING_ONLY)
  {
    // Print current source voltage
    clear_output_buffers();
    sprintf(output_buffer_short, "PSV.%.3fV", led_array_interface->get_power_source_voltage());
    sprintf(output_buffer_long, "Current power source voltage is: %.3f Volts.", led_array_interface->get_power_source_voltage());
    print(output_buffer_short, output_buffer_long);
  }
  else
    return ERROR_POWER_SENSING_NOT_SUPPORTED;

  return NO_ERROR;
}

void LedArray::setup()
{
  // If setup has been run before, deallocate previous arrays to avoid memory leaks
  if (!initial_setup)
  {
    delete[] LedArray::trigger_output_pulse_width_list_us;
    delete[] LedArray::trigger_output_start_delay_list_us;
    delete[] LedArray::trigger_output_mode_list;
    delete[] LedArray::trigger_input_mode_list;
    delete[] LedArray::trigger_output_polarity_list;
    delete[] LedArray::trigger_input_polarity_list;
    delete[] led_value;
    delete[] led_color;
  }

  // Sleep 0.1 seconds to allow chips to power up
  delay(100);

  // Initialize led array
  led_array_interface->device_setup();

  // Set z-distance to device default
  led_array_distance_z = led_array_distance_z_default;

  // Initialize output trigger settings
  LedArray::trigger_output_pulse_width_list_us = new uint32_t [led_array_interface->trigger_output_count];
  LedArray::trigger_output_start_delay_list_us = new uint32_t [led_array_interface->trigger_output_count];
  LedArray::trigger_output_mode_list = new int [led_array_interface->trigger_output_count];
  LedArray::trigger_input_polarity_list = new bool [led_array_interface->trigger_output_count];
  LedArray::trigger_output_polarity_list = new bool [led_array_interface->trigger_output_count];
  for (uint16_t trigger_index = 0; trigger_index < led_array_interface->trigger_output_count; trigger_index++)
  {
    LedArray::trigger_output_pulse_width_list_us[trigger_index] = TRIGGER_OUTPUT_PULSE_WIDTH_DEFAULT;
    LedArray::trigger_output_start_delay_list_us[trigger_index] = TRIGGER_OUTPUT_DELAY_DEFAULT;
    LedArray::trigger_output_mode_list[trigger_index] = 0;
    LedArray::trigger_output_polarity_list[trigger_index] = TRIGGER_OUTPUT_POLARITY_DEFAULT;
    pinMode(led_array_interface->trigger_output_pin_list[trigger_index], OUTPUT);
  }

  // Initialize output trigger settings
  LedArray::trigger_input_mode_list = new int [led_array_interface->trigger_input_count];
  for (int trigger_index = 0; trigger_index < led_array_interface->trigger_input_count; trigger_index++)
  {
    pinMode(led_array_interface->trigger_input_pin_list[trigger_index], INPUT);
    LedArray::trigger_input_polarity_list[trigger_index] = TRIGGER_INPUT_POLARITY_DEFAULT;
    LedArray::trigger_input_mode_list[trigger_index] = 0;
  }

  // Define led_value and led_color
  led_brightness = led_brightness_default;
  led_value = new uint8_t[led_array_interface->color_channel_count];
  led_color = new uint8_t[led_array_interface->color_channel_count];

  // Populate led_color and led_value
  for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    led_color[color_channel_index] = (uint8_t)(round((float)UINT8_MAX / led_array_interface->color_channel_count)) ; // TODO: make this respect bit depth

  // Reset sequence
  LedArray::led_sequence.deallocate();

  // Initialize sequences at every bit depth so these are defined
  LedArray::led_sequence.allocate(7);
  LedArray::led_sequence.increment(1);
  LedArray::led_sequence.append(0);
  LedArray::led_sequence.increment(0);
  LedArray::led_sequence.increment(1);
  LedArray::led_sequence.append(1);
  LedArray::led_sequence.increment(0);
  LedArray::led_sequence.increment(1);
  LedArray::led_sequence.append(2);
  LedArray::led_sequence.increment(0);
  LedArray::led_sequence.increment(1);
  LedArray::led_sequence.append(3);

  // Build list of LED NA coordinates
  build_na_list(led_array_distance_z);

  // Define default NA
  objective_na = na_default;

  // Define default inner na
  inner_na = inner_na_default;

  // Set default cosine factor
  cosine_factor = cosine_factor_default;

  // Load stored parameters
  if (led_array_interface->get_register(STORED_AUTOLOAD_LAST_STATE) != 0)
    recall_parameters(true);

  // Set LED value based on color and brightness
  for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
    led_value[color_channel_index] = (uint8_t) ceil((float) led_color[color_channel_index] / (float) UINT8_MAX * (float) led_brightness);

  // Update LED Pattern
  led_array_interface->update();

  // Run demo mode if EEPROM indicates we should
  if (get_demo_mode())
    run_demo();

  // Indicate that setup has run
  initial_setup = false;
}

int LedArray::run_demo()
{
  // Set demo mode flag
  set_demo_mode(true);

  // Clear Serial buffer
  while (Serial.available())
    Serial.read();


  // Run until key received
  while (get_demo_mode())
  {

    // Demo Brightfield patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;

      led_value[color_channel_index_outer] = 16;
      led_array_interface->clear();
      draw_primative_circle(0, objective_na);
      led_array_interface->update();
      delay(250);

      // Break loop if key is pressed
      if (Serial.available())
      {
        set_demo_mode(false);
        clear();
        return NO_ERROR;
      }
    }

    // Demo Annulus patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
        led_value[color_channel_index] = 0;

      led_value[color_channel_index_outer] = 16;
      led_array_interface->clear();
      draw_primative_circle(objective_na, objective_na + 0.2);
      led_array_interface->update();
      delay(250);

      // Break loop if key is pressed
      if (Serial.available())
      {
        set_demo_mode(false);
        clear();
        return NO_ERROR;
      }
    }

    // Demo DPC Patterns
    for (int color_channel_index_outer = 0; color_channel_index_outer < led_array_interface->color_channel_count; color_channel_index_outer++)
    {
      for (int pattern_index = 0; pattern_index < 4; pattern_index++)
      {
        for (int color_channel_index = 0; color_channel_index < led_array_interface->color_channel_count; color_channel_index++)
          led_value[color_channel_index] = 0;

        led_value[color_channel_index_outer] = 16;
        led_array_interface->clear();
        draw_primative_half_circle(dpc_pattern_angles[pattern_index], 0, objective_na);
        led_array_interface->update();
        delay(250);

        // Break loop if key is pressed
        if (Serial.available())
        {
          set_demo_mode(false);
          clear();
          return NO_ERROR;
        }

      }
    }

    for ( int16_t led_index = 0; led_index < led_array_interface->led_count; led_index++)
    {
      set_led(-1, -1, (uint8_t)0);
      set_led(led_index, -1, (uint8_t)127);
      led_array_interface->update();
      delay(10);

      // Break loop if key is pressed
      if (Serial.available())
      {
        set_demo_mode(false);
        clear();
        return NO_ERROR;
      }
    }


    for ( int16_t led_index = led_array_interface->led_count - 1; led_index >= 0; led_index--)
    {
      set_led(-1, -1, (uint8_t)0);
      set_led(led_index, -1, (uint8_t)127);
      led_array_interface->update();
      delay(10);

      // Break loop if key is pressed
      if (Serial.available())
      {
        set_demo_mode(false);
        clear();
        return NO_ERROR;
      }
    }
    delay(100);
  }


  return NO_ERROR;
}

int LedArray::set_sclk_baud_rate(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ;
  else if (argc == 2)
  {
    uint32_t new_baud_rate = (strtoul((char *) argv[1], NULL, 0));
    led_array_interface->set_sclk_baud_rate(new_baud_rate);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Array Distance
  clear_output_buffers();
  sprintf(output_buffer_short, "SBD.%g", led_array_interface->get_sclk_baud_rate() / 1000000.0);
  sprintf(output_buffer_long, "Serial baud rate is: %g MHz", led_array_interface->get_sclk_baud_rate() / 1000000.0);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_gsclk_frequency(uint16_t argc, char ** argv)
{
  if (argc == 1)
    ;
  else if (argc == 2)
  {
    uint32_t new_gsclk_frequency = (strtoul((char *) argv[1], NULL, 0));
    led_array_interface->set_gsclk_frequency(new_gsclk_frequency);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current Array Distance
  clear_output_buffers();
  sprintf(output_buffer_short, "GSF.%g", (double)led_array_interface->get_gsclk_frequency() / 1000000.0);
  sprintf(output_buffer_long, "Grayscale clock frequency is: %g MHz", (double)led_array_interface->get_gsclk_frequency() / 1000000.0);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int LedArray::set_command_mode(const char * mode)
{
  if (!strcmp(mode, "short") || !strcmp(mode, "machine"))
  {
    command_mode = COMMAND_MODE_SHORT;
    print("Command output set to machine-readable", "Command output set to machine-readable");
  }
  else if (!strcmp(mode, "long") || !strcmp(mode, "human"))
  {
    command_mode = COMMAND_MODE_LONG;
    print("Command output set to human-readable", "Command output set to human-readable");
  }
  else
    return ERROR_INVALID_ARGUMENT;

  return NO_ERROR;
}

void LedArray::print(const char * short_output, const char * long_output)
{
  if (command_mode == COMMAND_MODE_SHORT)
    Serial.printf("%s%s", short_output, SERIAL_LINE_ENDING);
  else
    Serial.printf("%s%s", long_output, SERIAL_LINE_ENDING);
}

void LedArray::clear_output_buffers()
{
  memset(&output_buffer_short, ' ', sizeof(output_buffer_short));
  memset(&output_buffer_long, ' ', sizeof(output_buffer_long));
}

// Note that passing a -1 for led_number or color_channel_index sets all LEDs or all color channels respectively
int LedArray::set_led(int16_t led_number, int16_t color_channel_index, uint16_t value)
{

  // Apply cosine weighting
  if (cosine_factor != 0)
    value = (uint16_t) (((float) value) * pow(1.0 / cos(asin(sqrt(LedArrayInterface::led_position_list_na[led_number][0] * LedArrayInterface::led_position_list_na[led_number][0] + LedArrayInterface::led_position_list_na[led_number][1] * LedArrayInterface::led_position_list_na[led_number][1]))), cosine_factor));

  led_array_interface->set_led(led_number, color_channel_index, value);

  return NO_ERROR;
}

int LedArray::set_led(int16_t led_number, int16_t color_channel_index, uint8_t value)
{

  // Apply cosine weighting
  if (cosine_factor != 0)
    value = (uint8_t) (((float) value) * pow(1.0 / cos(asin(sqrt(LedArrayInterface::led_position_list_na[led_number][0] * LedArrayInterface::led_position_list_na[led_number][0] + LedArrayInterface::led_position_list_na[led_number][1] * LedArrayInterface::led_position_list_na[led_number][1]))), cosine_factor));

  led_array_interface->set_led(led_number, color_channel_index, value);

  return NO_ERROR;
}

int LedArray::set_led(int16_t led_number, int16_t color_channel_index, bool value)
{
  // Cosine factors don't make sense for boolean arrays
  if (cosine_factor != 0)
    return ERROR_ARGUMENT_RANGE;

  led_array_interface->set_led(led_number, color_channel_index, value);

  return NO_ERROR;
}

int8_t LedArray::get_demo_mode()
{
  return led_array_interface->get_register(DEMO_MODE_ADDRESS);
}

int8_t LedArray::set_demo_mode(int8_t demo_mode)
{
  return led_array_interface->set_register(DEMO_MODE_ADDRESS, demo_mode);
}

int8_t LedArray::store_parameters()
{
  led_array_interface->set_register(STORED_NA_ADDRESS, (int8_t) round(objective_na * (float)INT8_MAX));
  led_array_interface->set_register(STORED_DISTANCE_ADDRESS, (uint8_t) led_array_distance_z);
  led_array_interface->set_register(STORED_BRIGHTNESS_ADDRESS, (uint8_t) led_brightness);

  led_array_interface->set_register(STORED_COLOR_R_ADDRESS, led_color[0]);
  if (led_array_interface->color_channel_count == 3)
  {
    led_array_interface->set_register(STORED_COLOR_G_ADDRESS, led_color[1]);
    led_array_interface->set_register(STORED_COLOR_B_ADDRESS, led_color[2]);
  }
  led_array_interface->set_register(STORED_GSCLK_ADDRESS, (uint8_t)((float)led_array_interface->get_gsclk_frequency() / 1000000 / 20 * UINT8_MAX));
  led_array_interface->set_register(STORED_SCLK_ADDRESS, (uint8_t)((float)led_array_interface->get_sclk_baud_rate() / 1000000 / 20 * UINT8_MAX));

  // Recall machine/human output state
  led_array_interface->set_register(STORED_COMMAND_MODE_ADDRESS, command_mode);

  // Print confirmation
  clear_output_buffers();
  sprintf(output_buffer_short, "STORE.OK");
  sprintf(output_buffer_long, "Stored parameters:\n   objective_na: %.2f\n   led_array_distance_z: %.2f\n   led_brightness: %d", objective_na, led_array_distance_z, led_brightness);
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

int8_t LedArray::recall_parameters(bool quiet)
{
  objective_na = (((float)led_array_interface->get_register(STORED_NA_ADDRESS)) / (float)INT8_MAX);
  led_array_distance_z = (float)led_array_interface->get_register(STORED_DISTANCE_ADDRESS);
  led_brightness = (uint8_t)led_array_interface->get_register(STORED_BRIGHTNESS_ADDRESS);

  // Set colors
  led_color[0] = (uint8_t)led_array_interface->get_register(STORED_COLOR_R_ADDRESS);
  if (led_array_interface->color_channel_count == 3)
  {
    led_color[1] = (uint8_t)led_array_interface->get_register(STORED_COLOR_G_ADDRESS);
    led_color[2] = (uint8_t)led_array_interface->get_register(STORED_COLOR_B_ADDRESS);
  }
  led_array_interface->set_gsclk_frequency((uint32_t) ((float)led_array_interface->get_register(STORED_GSCLK_ADDRESS) * 1000000 * 20 / UINT8_MAX));
  led_array_interface->set_sclk_baud_rate((uint32_t) ((float)led_array_interface->get_register(STORED_SCLK_ADDRESS) * 1000000 * 20 / UINT8_MAX));

  // Recall machine/human output state
  command_mode = led_array_interface->get_register(STORED_COMMAND_MODE_ADDRESS);

  // Print confirmation
  if (!quiet)
  {
    clear_output_buffers();
    sprintf(output_buffer_short, "READ.OK");
    sprintf(output_buffer_long, "Read parameters:\n   objective_na: %.2f\n   led_array_distance_z: %.2f\n   led_brightness: %d", objective_na, led_array_distance_z, led_brightness);
    print(output_buffer_short, output_buffer_long);
  }

  return NO_ERROR;
}

int8_t LedArray::set_autoload_on_reboot(uint16_t argc, char ** argv)
{
  bool new_state;
  if (argc == 1)
    ;
  else if (argc == 2)
  {
    new_state = (bool)atoi(argv[1]);
    led_array_interface->set_register(STORED_AUTOLOAD_LAST_STATE, new_state);
  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Print current setting
  clear_output_buffers();
  sprintf(output_buffer_short, "AUTOLOAD.%d", led_array_interface->get_register(STORED_AUTOLOAD_LAST_STATE));
  sprintf(output_buffer_long, "Autoload is %d", led_array_interface->get_register(STORED_AUTOLOAD_LAST_STATE));
  print(output_buffer_short, output_buffer_long);

  return NO_ERROR;
}

uint16_t LedArray::get_serial_number()
{
  return ((uint8_t)led_array_interface->get_register(SN_ADDRESS + 1) << 8) | (uint8_t)led_array_interface->get_register(SN_ADDRESS);
}

void LedArray::set_serial_number(uint16_t serial_number)
{
  byte lower_8bits_sn = serial_number & 0xff;
  byte upper_8bits_sn = (serial_number >> 8) & 0xff;
  led_array_interface->set_register(SN_ADDRESS, lower_8bits_sn);
  led_array_interface->set_register(SN_ADDRESS + 1, upper_8bits_sn);
}

uint16_t LedArray::get_part_number()
{
  return ((uint8_t)led_array_interface->get_register(PN_ADDRESS + 1) << 8) | (uint8_t)led_array_interface->get_register(PN_ADDRESS);
}

void LedArray::set_part_number(uint16_t part_number)
{
  byte lower_8bits_pn = part_number & 0xff;
  byte upper_8bits_pn = (part_number >> 8) & 0xff;
  led_array_interface->set_register(PN_ADDRESS, lower_8bits_pn);
  led_array_interface->set_register(PN_ADDRESS + 1, upper_8bits_pn);
}

int8_t LedArray::initialize_hardware(uint16_t argc, char ** argv)
{
  // Parse serial number
  if (argc == 3)
  {
    set_part_number(strtoul(argv[1], NULL, 0));
    set_serial_number(strtoul(argv[2], NULL, 0));

  }
  else
    return ERROR_ARGUMENT_COUNT;

  // Turn off autoload so default parameters are to be used
  led_array_interface->set_register(STORED_AUTOLOAD_LAST_STATE, false);

  // Re-initialize device using default values
  setup();

  // Set up grayscale clock and serial clock
  led_array_interface->set_sclk_baud_rate(SCLK_BAUD_RATE);
  led_array_interface->set_gsclk_frequency(GSCLK_FREQUENCY);

  // Store parameters
  store_parameters();

  // Turn on autoload so default parameters are to be used
  led_array_interface->set_register(STORED_AUTOLOAD_LAST_STATE, true);

  return NO_ERROR;
}

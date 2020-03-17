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


#ifndef COMMAND_ROUTING_H
#define COMMAND_ROUTING_H

#include <SPI.h>
#include <mk20dx128.h>

#define MAX_ARGUMENT_ELEMENT_LENGTH 10
#define MAX_COMMAND_LENGTH 20
#define MAX_ARGUMENT_COUNT_CHAR 1500

#include "commandconstants.h"
#include "ledarray.h"

class CommandRouter {
  public:
    int getArgumentBitDepth(char * command_header);
    void route(char * command_header, int16_t argc, void ** argv, int16_t * argument_led_number_list);
    void processSerialStream();
    int getArgumentLedNumberPitch(char * command_header);
    void printHelp();
    void setLedArray(LedArray *  new_led_array);
    void printTerminator();
    void setDebug(int16_t argc, char * * argv);

  private:
    // Standard element variables
    int debug = 0;

    // Serial command holders
    LedArray * led_array;
    bool send_termination_char = true;
    char command [MAX_COMMAND_LENGTH + 1]; // Allow for terminating null byte
    char * * argv;
    char current_argument[MAX_ARGUMENT_ELEMENT_LENGTH + 1];

    char * * argument_list = NULL;
    bool * argument_list_bool = NULL;
    uint8_t * argument_list_uint8 = NULL;
    uint16_t * argument_list_uint16 = NULL;
    int16_t * argument_led_number_list = NULL;
};

void CommandRouter::printHelp()
{
  led_array->printAbout();
  Serial.printf(F("-----------------------------------%s"), SERIAL_LINE_ENDING);
  Serial.printf(F("Command List: %s"), SERIAL_LINE_ENDING);
  Serial.printf(F("-----------------------------------%s"), SERIAL_LINE_ENDING);
  for (int16_t cIdx = 0; cIdx < COMMAND_COUNT; cIdx++)
  {
    Serial.printf(F("COMMAND: %s"), SERIAL_LINE_ENDING);
    Serial.print(command_list[cIdx][0]);
    Serial.print(" / ");
    Serial.print(command_list[cIdx][1]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.print(F("SYNTAX:"));
    Serial.print(command_list[cIdx][3]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.printf(F("DESCRIPTION:%s"), SERIAL_LINE_ENDING);
    Serial.print(command_list[cIdx][2]);
    Serial.print(SERIAL_LINE_ENDING);
    Serial.printf(F("-----------------------------------%s"), SERIAL_LINE_ENDING);
  }
}

void CommandRouter::setLedArray(LedArray* new_led_array)
{
  led_array = new_led_array;
}

void CommandRouter::setDebug(int16_t argc, char * * argv)
{
  int interface_debug = 0;
  if (argc == 1)
  {
    uint16_t debug_value = strtoul(argv[0], NULL, 0);
    if (debug_value > 10)
    {
      debug = (int) ((debug_value % 1000 - debug_value % 100) / 100.0);
      interface_debug = debug_value - debug * 100;
    }
    else
    {
      debug = debug_value;
      interface_debug = debug_value;
    }
  }
  else if (argc == 0)
  {
    debug = (debug == 0) * 1;
    interface_debug = debug;
  }
  else if (argc == 3)
  {
    debug = (uint8_t)atoi(argv[0]);
    interface_debug = 10 * (uint8_t)atoi(argv[1]) + (uint8_t)atoi(argv[2]);
  }
  else
    Serial.printf(F("ERROR (CommandRouter::setDebug): Invalud argument count.%s"), SERIAL_LINE_ENDING);

  // User feedback
  Serial.printf(F("(CommandRouter::setDebug): Debug level is %d \n"), debug);

  // Set lower-level debug
  led_array->setDebug(interface_debug); // Set all to the same vaue
}

int CommandRouter::getArgumentBitDepth(char * command_header)
{
  if ((strcmp(command_header, command_list[CMD_SET_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SEQ_IDX][1]) == 0))
    return (led_array->getSequenceBitDepth());
  else
    return (-1);
}

/* This function is used to dictate the pitch of led numbers in a command stream.
  Every value at this pitch will be stored as a uint16_t instead of the default datatype */
int CommandRouter::getArgumentLedNumberPitch(char * command_header)
{
  if ((strcmp(command_header, command_list[CMD_SET_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SEQ_IDX][1]) == 0))
    //    return (led_array->getColorChannelCount() + 1);
    return (2);
  else
    return (-1);
}

void CommandRouter::route(char * command_header, int16_t argc, void ** argv, int16_t * argument_led_number_list)
{
  if ((strcmp(command_header, command_list[CMD_HELP_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_HELP_IDX][1]) == 0))
    printHelp();
  else if ((strcmp(command_header, command_list[CMD_ABOUT_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_ABOUT_IDX][1]) == 0))
    led_array->printAbout();
  else if ((strcmp(command_header, command_list[CMD_REBOOT_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_REBOOT_IDX][1]) == 0))
    led_array->reset();
  else if ((strcmp(command_header, command_list[CMD_SHOW_VERSION][0]) == 0) || (strcmp(command_header, command_list[CMD_SHOW_VERSION][1]) == 0))
    led_array->printVersion();

  else if ((strcmp(command_header, command_list[CMD_AUTOCLEAR_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_AUTOCLEAR_IDX][1]) == 0))
    led_array->toggleAutoClear(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_NA_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_NA_IDX][1]) == 0))
    led_array->setNa(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SET_COLOR_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_COLOR_IDX][1]) == 0))
    led_array->setColor(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SET_BRIGHTNESS][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_BRIGHTNESS][1]) == 0))
    led_array->setBrightness(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SET_ARRAY_DIST][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_ARRAY_DIST][1]) == 0))
    led_array->setArrayDistance(argc, (char * *) argv);

  else if ((strcmp(command_header, command_list[CMD_LED_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_LED_IDX][1]) == 0))
    led_array->drawLedList(argc, (char * *)argv);

  else if ((strcmp(command_header, command_list[CMD_CLEAR_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_CLEAR_IDX][1]) == 0))
    led_array->clear();
  else if ((strcmp(command_header, command_list[CMD_FILL_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_FILL_IDX][1]) == 0))
    led_array->fillArray();
  else if ((strcmp(command_header, command_list[CMD_BF_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_BF_IDX][1]) == 0))
    led_array->drawBrightfield(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_DF_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_DF_IDX][1]) == 0))
    led_array->drawDarkfield();
  else if ((strcmp(command_header, command_list[CMD_DPC_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_DPC_IDX][1]) == 0))
    led_array->drawDpc(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_CDPC_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_CDPC_IDX][1]) == 0))
    led_array->drawCdpc(argc, (char * *)argv);
  else if ((strcmp(command_header, command_list[CMD_AN_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_AN_IDX][1]) == 0))
    led_array->drawAnnulus(argc, (char * *)argv);
  else if ((strcmp(command_header, command_list[CMD_HALF_ANNULUS][0]) == 0) || (strcmp(command_header, command_list[CMD_HALF_ANNULUS][1]) == 0))
    led_array->drawHalfAnnulus(argc, (char * *)argv);
  else if ((strcmp(command_header, command_list[CMD_CDF_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_CDF_IDX][1]) == 0))
    led_array->drawColorDarkfield(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_NAV_DPC_IDX][0]) == 0)  || (strcmp(command_header, command_list[CMD_NAV_DPC_IDX][1]) == 0))
    led_array->drawNavDpc();

  else if ((strcmp(command_header, command_list[CMD_SCF_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SCB_IDX][1]) == 0))
    led_array->scanAllLeds(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SCB_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SCB_IDX][1]) == 0))
    led_array->scanBrightfieldLeds(argc, (char * *) argv);

  else if ((strcmp(command_header, command_list[CMD_LEN_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_LEN_SEQ_IDX][1]) == 0))
    led_array->setSequenceLength(strtoul((char *) argv[0], NULL, 0), false);
  else if ((strcmp(command_header, command_list[CMD_SET_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SEQ_IDX][1]) == 0))
    led_array->setSequenceValue(argc, argv, argument_led_number_list);
  else if ((strcmp(command_header, command_list[CMD_RUN_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_RUN_SEQ_IDX][1]) == 0))
    led_array->runSequence(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_RUN_SEQ_DPC_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_RUN_SEQ_DPC_IDX][1]) == 0))
    led_array->runSequenceDpc(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_RUN_SEQ_FPM_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_RUN_SEQ_FPM_IDX][1]) == 0))
    led_array->runSequenceFpm(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_RUN_SEQ_FAST_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_RUN_SEQ_FAST_IDX][1]) == 0))
    led_array->runSequenceFast(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_PRINT_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_SEQ_IDX][1]) == 0))
    led_array->printSequence();
  else if ((strcmp(command_header, command_list[CMD_PRINT_SEQ_LENGTH_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_SEQ_LENGTH_IDX][1]) == 0))
    led_array->printSequenceLength();
  else if ((strcmp(command_header, command_list[CMD_STEP_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_STEP_SEQ_IDX][1]) == 0))
    led_array->stepSequence(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_RESET_SEQ_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_RESET_SEQ_IDX][1]) == 0))
    led_array->resetSequence();
  else if ((strcmp(command_header, command_list[CMD_SET_SEQ_BIT_DEPTH][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SEQ_BIT_DEPTH][1]) == 0))
    led_array->setSequenceBitDepth(atoi((char *) argv[0]), false); // second arg is quiet
  else if ((strcmp(command_header, command_list[CMD_SET_SEQ_ZEROS][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SEQ_ZEROS][1]) == 0))
    led_array->setSequenceZeros(argc, (char * *) argv);

  else if ((strcmp(command_header, command_list[CMD_TRIG_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_TRIG_IDX][1]) == 0))
  {
    if (argc == 0)
      led_array->sendTriggerPulse(0, true);
    else if (argc == 1)
      led_array->sendTriggerPulse(atoi((char *) argv[0]), true);
  }
  else if ((strcmp(command_header, command_list[CMD_TRIG_SETUP_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_TRIG_SETUP_IDX][1]) == 0))
    led_array->triggerSetup(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_TRIG_TEST_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_TRIG_TEST_IDX][1]) == 0))
    led_array->triggerInputTest(strtoul((char *) argv[0], NULL, 0));

  else if ((strcmp(command_header, command_list[CMD_PRINT_VALS_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_VALS_IDX][1]) == 0))
    led_array->printCurrentLedValues(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_CHANNEL_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_CHANNEL_IDX][1]) == 0))
    led_array->drawChannel(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_TOGGLE_DEBUG_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_TOGGLE_DEBUG_IDX][1]) == 0))
    setDebug(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_PIN_ORDER_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_PIN_ORDER_IDX][1]) == 0))
    led_array->setPinOrder(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_PRINT_LED_POSITIONS][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_LED_POSITIONS][1]) == 0))
    led_array->printLedPositions(argc, (char * *) argv, false);
  else if ((strcmp(command_header, command_list[CMD_PRINT_LED_POSITIONS_NA][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_LED_POSITIONS_NA][1]) == 0))
    led_array->printLedPositions(argc, (char * *) argv, true);
  else if ((strcmp(command_header, command_list[CMD_DELAY][0]) == 0) || (strcmp(command_header, command_list[CMD_DELAY][1]) == 0))
    delay(strtoul((char *) argv[0], NULL, 0));
  else if ((strcmp(command_header, command_list[CMD_SET_MAX_CURRENT][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_MAX_CURRENT][1]) == 0))
    led_array->setMaxCurrentLimit(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SET_MAX_CURRENT_ENFORCEMENT][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_MAX_CURRENT_ENFORCEMENT][1]) == 0))
    led_array->setMaxCurrentEnforcement(argc, (char * *) argv);

  else if ((strcmp(command_header, command_list[CMD_DISCO_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_DISCO_IDX][1]) == 0))
    led_array->drawDiscoPattern();
  else if ((strcmp(command_header, command_list[CMD_PRINT_PARAMS][0]) == 0) || (strcmp(command_header, command_list[CMD_PRINT_PARAMS][1]) == 0))
    led_array->printSystemParams();
  else if ((strcmp(command_header, command_list[CMD_DEMO_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_DEMO_IDX][1]) == 0))
    led_array->demo();
  else if ((strcmp(command_header, command_list[CMD_WATER_IDX][0]) == 0) || (strcmp(command_header, command_list[CMD_WATER_IDX][1]) == 0))
    led_array->waterDrop();

  else if ((strcmp(command_header, command_list[CMD_SET_SN][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_SN][1]) == 0))
    led_array->setPartNumber(strtoul((char *) argv[0], NULL, 0));
  else if ((strcmp(command_header, command_list[CMD_SET_PN][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_PN][1]) == 0))
    led_array->setSerialNumber(strtoul((char *) argv[0], NULL, 0));

  else if ((strcmp(command_header, command_list[CMD_SET_BAUD_RATE][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_BAUD_RATE][1]) == 0))
    led_array->setBaudRate(argc, (char * *) argv);
  else if ((strcmp(command_header, command_list[CMD_SET_GSCLK_FREQ][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_GSCLK_FREQ][1]) == 0))
    led_array->setGsclkFreq(argc, (char * *) argv);

  else if ((strcmp(command_header, command_list[CMD_SET_HUMAN][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_HUMAN][1]) == 0))
    led_array->setCommandMode("human");
  else if ((strcmp(command_header, command_list[CMD_SET_MACHINE][0]) == 0) || (strcmp(command_header, command_list[CMD_SET_MACHINE][1]) == 0))
    led_array->setCommandMode("machine");

  else
  {
    // Check if the command is equal to any device-specific commands
    bool command_found = false;
    for (uint16_t command_index = 0; command_index < led_array->getDeviceCommandCount(); command_index++)
    {
      if ((strcmp(command_header, led_array->getDeviceCommandNameShort(command_index)) == 0) || (strcmp(command_header, led_array->getDeviceCommandNameLong(command_index)) == 0))
      {
        led_array->deviceCommand(command_index, argc, (char * *) argv);
        command_found = true;
      }
    }

    if (!command_found)
    {
      Serial.print(F("Command ["));
      Serial.print(command_header);
      Serial.print(F("] is not implemented yet."));
    }
  }
}

void CommandRouter::processSerialStream()
{
  // Initialize command string
  memset(command, 0, sizeof(command));

  // Initialize empty argument element
  memset(current_argument, 0, sizeof(current_argument));

  // Initialize indexing variables used locally by this function
  uint16_t command_position = 0;
  uint16_t argument_element_position = 0;
  uint16_t argument_count = 0;
  uint16_t argument_led_count = 0;
  uint16_t argument_total_count = 0;
  uint16_t argument_max_led_count = 0;
  bool argument_flag = false;
  int argument_bit_depth = -1;
  int argument_led_number_pitch = -1;

  while (Serial.available() > 0)
  {
    const byte new_byte = Serial.read();
    switch (new_byte)
    {
      case '\n':   // end of text
        {
          command[command_position] = 0;     // terminating null byte
          if (argument_flag)
          {
            if (debug > 0) {
              Serial.print(F("Copying new argument inside newline with index "));
              Serial.print(argument_total_count);
              Serial.print(F(" with bit depth "));
              Serial.print(argument_bit_depth);
              Serial.print(SERIAL_LINE_ENDING);
            }

            // Character argument (standard)
            if (argument_bit_depth == -1)
            {
              argument_list[argument_count] = new char[argument_element_position + 1]; // Allow for null terminating byte
              memcpy(argument_list[argument_count], current_argument, sizeof(char) * argument_element_position + 1); // Also copy null terminating byte
            }
            else
            {
              if ((argument_bit_depth > 0) && (argument_total_count == 1))
              {
                if (argument_bit_depth == 1) // numerical argument (standard)
                  argument_list_bool = new bool[1];
                else if (argument_bit_depth == 8)
                  argument_list_uint8 = new uint8_t[1];
                else
                  argument_list_uint16 = new uint16_t[1];

                argument_led_number_list = new int16_t[1];
                argument_led_number_list[0] = 0;
              }
              if (argument_bit_depth == 1) // numerical argument (standard)
                argument_list_bool[argument_count]  = atoi(current_argument) > 0;
              else if (argument_bit_depth == 8)
                argument_list_uint8[argument_count]  = (uint8_t)atoi(current_argument);
              else
                argument_list_uint16[argument_count]  = strtoul(current_argument, NULL, 0);
            }

            if (debug > 0)
            {
              Serial.print("Copied new argument with value: ");
              Serial.print(current_argument);
              Serial.print(" and desired bit depth ");
              Serial.print(argument_bit_depth);
              Serial.print(" to new variable which is now ");
              if (argument_bit_depth == -1)
              {
                Serial.print(argument_list[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 1)
              {
                Serial.print(argument_list_bool[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 8)
              {
                Serial.print(argument_list_uint8[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 16)
              {
                Serial.print(argument_list_uint16[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
            }

            // Increment number of optional arguments
            argument_count++;
            argument_total_count++;
          }

          if (debug > 0)
          {
            Serial.print("Command: ");
            Serial.print(command);
            Serial.print(SERIAL_LINE_ENDING);
            if (argument_flag)
            {
              for (uint16_t arg_index = 0; arg_index < argument_count; arg_index++)
              {
                Serial.print(" Argument ");
                Serial.print(arg_index);
                Serial.print(": ");
                if (argument_bit_depth == -1)
                  Serial.print(argument_list[arg_index]);
                else
                {
                  if (argument_bit_depth == 1)
                    Serial.print(argument_list_bool[arg_index]);
                  else if (argument_bit_depth == 8)
                    Serial.print(argument_list_uint8[arg_index]);
                  else if (argument_bit_depth == 16)
                    Serial.print(argument_list_uint16[arg_index]);
                }
                Serial.print(SERIAL_LINE_ENDING);
              }
            }
          }
          if (argument_flag)
            if (argument_led_count > 0)
              argument_led_number_list[0] = argument_led_count;

          // Parse command and arguments based on bit depth
          if (argument_bit_depth == -1)
            route(command, argument_count, (void **) argument_list, argument_led_number_list);
          else if (argument_bit_depth == 1)
            route(command, argument_count, (void **) argument_list_bool, argument_led_number_list);
          else if (argument_bit_depth == 8)
            route(command, argument_count, (void **) argument_list_uint8, argument_led_number_list);
          else if (argument_bit_depth == 16)
            route(command, argument_count, (void **) argument_list_uint16, argument_led_number_list);

          // Clear serial buffer so we don't act on any serial input received during command processing.
          while (Serial.available())
            Serial.read();

          if (argument_count > 0)
          {
            // Delete argument list elements
            if (argument_bit_depth == -1)
            {
              for (uint16_t argument_index = 0; argument_index < argument_count; argument_index++)
              {
                delete[] argument_list[argument_index];
                if (debug > 1)
                {
                  Serial.print(" Deallocated argument ");
                  Serial.print(argument_index);
                  Serial.print(SERIAL_LINE_ENDING);
                }
              }
              delete[] argument_list;
            }
            else if (argument_bit_depth == 1)
              delete[] argument_list_bool;
            else if (argument_bit_depth == 8)
              delete[] argument_list_uint8;
            else if (argument_bit_depth == 16)
              delete[] argument_list_uint16;

            if (argument_led_number_pitch >= 0)
              delete[] argument_led_number_list;
          }

          if (send_termination_char)
          {
            Serial.print(SERIAL_COMMAND_TERMINATOR);
            Serial.print(SERIAL_LINE_ENDING);
          }
          break;
        }
      case '.':   // dot SERIAL_DELIMITER
        {
          if (!argument_flag) { // This is the case where we've just finished a command and need to initialize argument parameters
            argument_flag = true;
            argument_count = 0;
            command[command_position] = 0; // Add null terminating byte

            // Get argument bit depth from command header
            argument_bit_depth = getArgumentBitDepth(command);

            // Get LED number Pitch from command header
            argument_led_number_pitch = getArgumentLedNumberPitch(command);

            // Initialize charater argument list if we're using
            if (argument_bit_depth <= 0)
              argument_list = new char * [MAX_ARGUMENT_COUNT_CHAR];

            if (debug > 1)
              Serial.printf("Switching to argument mode%s", SERIAL_LINE_ENDING);
          }

          else if (argument_bit_depth > 0 && (argument_flag && argument_total_count == 1))
          { // This is the case where we're running a numeric storage command (such as setSequenceValue) and need to collect the number of LEDs in the list (first argument), as provided by the user.
            if (debug > 1) {
              Serial.print("Processing LED count at index ");
              Serial.print(argument_total_count);
              Serial.print(SERIAL_LINE_ENDING);
              delay(10);
            }
            // Get argument LED count
            argument_max_led_count = strtoul(current_argument, NULL, 0);

            if (argument_max_led_count > 0)
            {
              // Initialize argument arrays using bit_depth
              if (argument_bit_depth == 1)
                argument_list_bool = new bool[argument_max_led_count];
              else if (argument_bit_depth == 8)
                argument_list_uint8 = new uint8_t[argument_max_led_count];
              else if (argument_bit_depth == 16)
                argument_list_uint16 = new uint16_t[argument_max_led_count];

              // Initialize LED number list
              argument_led_number_list = new int16_t [argument_max_led_count+1];
            }
            else
            { // Case where user types ssl.0 (no leds on)
              // Initialize argument arrays using bit_depth
              if (argument_bit_depth == 1)
              {
                argument_list_bool = new bool[1];
                argument_list_bool[0] = 0;
              }
              else if (argument_bit_depth == 8)
              {
                argument_list_uint8 = new uint8_t[1];
                argument_list_uint8[0] = 0;
              }
              else if (argument_bit_depth == 16)
              {
                argument_list_uint16 = new uint16_t[1];
                argument_list_uint16[0] = 0;
              }

              // Initialize LED number list
              argument_led_number_list = new int16_t [1];
              argument_led_number_list[0] = 0;
            }
          }
          else if ((argument_led_number_pitch > 0) && (((argument_total_count) % argument_led_number_pitch ) == 0))
          { // In this case, we store a LED number for a numerical list
            if (debug > 1) {
              Serial.print("Processing LED number at index ");
              Serial.print(argument_total_count);
              Serial.print(F(" ("));
              Serial.print(current_argument);
              Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
            }

            // If this argument is a LED number, store it in the appropriate array
            argument_led_number_list[argument_led_count + 1] = strtol(current_argument, NULL, 0);
            argument_led_count++; // Increment number of leds measured

            if (argument_led_count > argument_max_led_count)
            {
              Serial.print(F("ERROR - max led count (")); Serial.print(argument_max_led_count); Serial.printf(F(") reached!%s"), SERIAL_LINE_ENDING);
            }
          }
          else
          {
            if (debug > 1) {
              Serial.print("Processing argument at index ");
              Serial.print(argument_total_count);
              Serial.print(SERIAL_LINE_ENDING);
              delay(10);
            }
            // Copy argument or led value an argument_list

            // character argument (standard)
            if (argument_bit_depth == -1)
            {
              argument_list[argument_count] = new char[argument_element_position + 1]; // Allow for null terminating byte
              memcpy(argument_list[argument_count], current_argument, sizeof(char) * argument_element_position + 1); // Also copy null terminating byte
            }
            else if (argument_bit_depth == 1) // numerical argument (standard)
              argument_list_bool[argument_count]  = atoi(current_argument) > 0;
            else if (argument_bit_depth == 8)
              argument_list_uint8[argument_count]  = (uint8_t)atoi(current_argument);
            else
              argument_list_uint16[argument_count]  = strtoul(current_argument, NULL, 0);

            if (debug > 0)
            {
              Serial.print("Copied new argument with value: ");
              Serial.print(current_argument);
              Serial.print(" and desired bit depth ");
              Serial.print(argument_bit_depth);
              Serial.print(" to new variable which is now ");
              if (argument_bit_depth == -1)
              {
                Serial.print(argument_list[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 1)
              {
                Serial.print(argument_list_bool[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 8)
              {
                Serial.print(argument_list_uint8[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
              else if (argument_bit_depth == 16)
              {
                Serial.print(argument_list_uint16[argument_count]);
                Serial.print(SERIAL_LINE_ENDING);
              }
            }
            argument_count++; // Increment number of optional arguments
          }
          argument_total_count++;

          // Clear current argument string to save memory
          memset(current_argument, 0, sizeof(current_argument));
          argument_element_position = 0;
          break;
        }
      default:
        {
          // keep adding if not full ... allow for terminating null byte
          if (argument_flag)
          {
            if (argument_element_position > MAX_ARGUMENT_ELEMENT_LENGTH)
              Serial.printf(F("ERROR: Optional element was too long!%s"), SERIAL_LINE_ENDING);
            else
            {
              // append this to the current optional argument
              current_argument[argument_element_position] = new_byte;
              argument_element_position++; // increment optional position
            }
          }
          else
          {
            if (command_position >= MAX_COMMAND_LENGTH)
              Serial.printf(F("ERROR: Command was too long! %s"), SERIAL_LINE_ENDING);
            else
              command [command_position++] = new_byte;
          }
          break;
        }
    }  // end of switch
  }
}

#endif

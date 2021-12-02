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

#include "commandrouting.h"
#include <Arduino.h>
#include <errno.h>
#include "constants.h"

// Syntax is: {short code, long error description}
const char* error_code_list[ERROR_CODE_COUNT][2] = {

  // No Error
  {"0", "No Error"},

  // High-level Errors
  {"NI", "Command not implemented."},

  // Sequence
  {"ARGS", "Wrong number of arguments."},

  // Power source disconnected
  {"PSRC", "(LedArray::sourceConnected): Source Disconnected"},

  // Trigger Settings
  {"TRIGT", "Trigger Timeout."},

  // Cosine Factor
  {"COS", "Invalid cosine factor."},

  // MAC address generation
  {"MAC", "Mac address generation failed."},

  // PSU Sensing
  {"PSUSENS", "PSU sensing not supported on this device."},

  // Invalid Argument
  {"BADARG", "Invalid argument."},

  // Invalid Argument
  {"NOT_SUPPORTED_BY_DEVICE", "Device does not support this feature."},

  // Invalid Argument
  {"ARG_RANGE", "Invalid argument range."},

  // Invalid Argument
  {"TRIG_CONFIG", "Triggering not configured."},

  // Sequence zeros too long
  {"END_OF_SEQ", "Reached end of sequence."},

  // Sequence zeros too long
  {"SEQ_DELAY", "Sequence period was shorter than LED update rate."},

  // Sequence zeros too long
  {"TRIG_TIMEOUT", "Trigger timeout."},

  // Sequence zeros too long
  {"INTERRUPTED", "Command interrupted."},

  // Invalid command
  {"INVALID_COMMAND", "Invalid command."},

  // Not enough memory for operation
  {"MEMORY", "Not enough memory for operation."},

  // Invalid command
  {"COMMAND_LENGTH", "Command too long."},

  // Sequence full
  {"SEQUENCE_FULL", "Sequence is full."}

};

int CommandRouter::init(command_item_t *commands, int buffer_size,
                        char *serial_buffer, int argv_max,
                        const char **argv_buffer)
{
  this->buffer = serial_buffer;
  this->argv = argv_buffer;
  this->buffer_size = buffer_size;
  this->argv_max = argv_max;
  this->command_list = commands;
  this->malloc_used = false;
  return NO_ERROR;
}

int CommandRouter::route(int argc, const char **argv) {

  if (argc == 0)
    return NO_ERROR;

  for (int i = 0; command_list[i].name != nullptr; i++)
    if (strcmp(argv[0], command_list[i].name) == 0)
      return command_list[i].func(this, argc, argv);

  return ERROR_INVALID_COMMAND;
}

int CommandRouter::help() {
  Serial.print(F("-----------------------------------\n"));
  Serial.print(F("Command List:\n"));
  Serial.print(F("-----------------------------------\n"));
  for (int i = 0; command_list[i].name != nullptr; i++)
  {
    Serial.print(F("COMMAND: \n  "));
    Serial.print(command_list[i].name);
    Serial.print("\n");
    Serial.print(F("SYNTAX:\n  "));
    Serial.print(command_list[i].syntax);
    Serial.print("\n");
    Serial.print(F("DESCRIPTION:\n  "));
    Serial.print(command_list[i].description);
    Serial.print("\n");
    Serial.print(F("-----------------------------------\n"));
  }
  return NO_ERROR;
}


int CommandRouter::process_serial_stream() {
  int argc;
  int bytes_read = 0;
  int bytes_read_max = buffer_size - 1 - 1;
  int result;

  // Set input buffer to second character in input buffer
  char *input_buffer = &this->buffer[1];
  this->buffer[0] = '\0'; // Null terminate the return string

  // TODO: is this limit correct?
  while (bytes_read < bytes_read_max)
  {
    incoming = Serial.read();

    // Newline
    if (incoming == '\n' || incoming == '\r')
      break;

    input_buffer[bytes_read] = (char)incoming;
    bytes_read++;
  }

  // Flush remaining parts of serial buffer
  if (bytes_read == bytes_read_max)
  {
    while (true)
    {
      incoming = Serial.read();
      if (incoming == '\n' || incoming == '\r')
        break;
    }
    return ERROR_COMMAND_TOO_LONG;
  }

  if (bytes_read == 0)
    return NO_ERROR;

  // Set null terminating character
  input_buffer[bytes_read] = '\0';

  // Tokenize strings
  argc = 0;
  argv[argc] = strtok(input_buffer, ".");
  while (argv[argc] != NULL)
  {
    delayMicroseconds(1); // For teensy 4.0
    argv[++argc] = strtok(NULL, ".");
  }

  // Call route command
  result = route(argc, argv);

  // Print the error message, if any
  if (result > 0)
  {
    if (result < ERROR_CODE_COUNT)
      Serial.printf("ERROR[%d]: %s\n", result, error_code_list[result][1]);
    else
      Serial.printf("ERROR[%d]: INVALID ERROR CODE %s", result, '\n');
  }
  else
    Serial.printf("%s%s", COMMAND_END, SERIAL_LINE_ENDING);

  return result;
}

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

  {"INVALID_COMMAND", "Invalid command."},

  {"MEMORY", "Not enough memory for operation."}

};

int CommandRouter::init(command_item_t *commands, int buffer_size,
                        int argv_max) {
  if (argv_max == 0) {
    return ERROR_INVALID_COMMAND;
  }
  if (buffer_size == 0) {
    return ERROR_INVALID_COMMAND;
  }

  argv = new const char *[argv_max];
  if (argv == nullptr)
    goto fail_argv_alloc;

  this->argv_max = argv_max;

  // TODO: do I need a + 1 here?
  buffer = new char[buffer_size + 1];
  if (buffer == nullptr)
    goto fail_buffer_alloc;
  this->buffer_size = buffer_size;

  command_list = commands;
  malloc_used = true;
  return NO_ERROR;

  delete buffer;
  buffer = nullptr;
  this->buffer_size = 0;
fail_buffer_alloc:
  delete argv;
  argv = nullptr;
  this->argv_max = 0;
fail_argv_alloc:
  return ERROR_MEMORY_ALLOC;
}

int CommandRouter::init_no_malloc(command_item_t *commands, int buffer_size,
                                  char *serial_buffer, int argv_max,
                                  const char **argv_buffer) {

  if (commands == nullptr) {
    return ERROR_INVALID_COMMAND;
  }
  if (buffer_size == 0) {
    return ERROR_INVALID_COMMAND;
  }
  if (argv_max == 0) {
    return ERROR_INVALID_COMMAND;
  }
  if (serial_buffer == nullptr) {
    return ERROR_INVALID_COMMAND;
  }
  if (argv_buffer == nullptr) {
    return ERROR_INVALID_COMMAND;
  }

  cleanup();

  this->buffer = serial_buffer;
  this->argv = argv_buffer;
  this->buffer_size = buffer_size;
  this->argv_max = argv_max;
  this->command_list = commands;
  this->malloc_used = false;
  return NO_ERROR;
}

void CommandRouter::cleanup() {
  if (malloc_used) {
    if (buffer != nullptr) {
      delete buffer;
    }
    if (argv != nullptr) {
      delete argv;
    }
  }

  buffer = nullptr;
  buffer_size = 0;
  argv = nullptr;
  argv_max = 0;
  command_list = nullptr;
  malloc_used = false;
}

CommandRouter::~CommandRouter() {
  cleanup();
}

int CommandRouter::help(const char *command_name) {
  if (command_name == nullptr) {
    Serial.print(F("-----------------------------------\n"));
    Serial.print(F("Command List:\n"));
    Serial.print(F("-----------------------------------\n"));
  }
  for (int i = 0; command_list[i].name != nullptr; i++) {
    if (command_name == nullptr ||
        (strcmp(command_name, command_list[i].name) == 0)) {
      Serial.print(F("COMMAND: \n"));
      Serial.print(command_list[i].name);
      Serial.print("\n");
      Serial.print(F("SYNTAX:\n"));
      Serial.print(command_list[i].syntax);
      Serial.print("\n");
      Serial.print(F("DESCRIPTION:\n"));
      Serial.print(command_list[i].description);
      Serial.print("\n");
      if (command_name == nullptr) {
        Serial.print(F("-----------------------------------\n"));
      } else {
        return NO_ERROR;
      }
    }
  }
  if (command_name == nullptr) {
    return NO_ERROR;
  } else {
    snprintf(buffer, buffer_size, "%s not found", command_name);
    return ERROR_INVALID_COMMAND;
  }
}

int CommandRouter::route(int argc, const char **argv) {
  if (command_list == nullptr)
    return ERROR_INVALID_COMMAND;

  if (argc == 0)
    return ERROR_INVALID_COMMAND;

  if (argv[0] == nullptr)
    return ERROR_INVALID_COMMAND;

  for (int i = 0; command_list[i].name != nullptr; i++) {
    if (strcmp(argv[0], command_list[i].name) == 0) {
      if (command_list[i].func != nullptr) {
        return command_list[i].func(this, argc, argv);
      } else {
        snprintf(buffer, buffer_size, "%s not found", argv[0]);
        return ERROR_INVALID_COMMAND;
      }
    }
  }
  return ERROR_INVALID_COMMAND;
}

int command_help_func(CommandRouter *cmd, int argc, const char **argv) {
  if (argc == 1) {
    return cmd->help(nullptr);
  } else {
    return cmd->help(argv[1]);
  }
}

int CommandRouter::processSerialStream() {
  int argc;
  int bytes_read = 0;
  int bytes_read_max = buffer_size - 1 - 1;
  int result;
  // The results will be placed in the
  // buffer starting at index 0 AFTER they have used the
  // parameter
  // So we place the input parametrers at index 1
  char *input_buffer = &this->buffer[1];
  this->buffer[0] = '\0'; // Null terminate the return string

  // TODO: is this limit correct?
  while (bytes_read < bytes_read_max) {
    int c;
    c = Serial.read();
    if (c == -1) {
      continue;
    }
    if (c == '\n' || c == '\r')
      break;
    if (c == '\b') {
      bytes_read = bytes_read == 0 ? 0 : bytes_read - 1;
      continue;
    }

    input_buffer[bytes_read] = (char)c;
    bytes_read++;
  }
  // Just flush the serial buffer, and terminate the command
  if (bytes_read == bytes_read_max) {
    result = ERROR_MEMORY_ALLOC;
    while (true) {
      int c;
      c = Serial.read();
      if (c == '\n' || c == '\r') {
        goto finish;
      }
    }
  }
  input_buffer[bytes_read] = '\0';
  if (bytes_read == 0) {
    return NO_ERROR;
  }

  // This could likely be replaced by strtok_r, but I really failed
  // at using it on the teensy 4.0
  // And as such, I'm simply ignoring using it.
  int i;
  i = 0;
  argc = 0;
  while (i < bytes_read) {
    char c;
    c = input_buffer[i];
    if (c == '\0')
      break;
    if (input_buffer[i] == DELIMETER) {
      input_buffer[i] = '\0';
      i++;
      continue;
    }
    argv[argc] = &input_buffer[i];
    argc++;
    while (i < bytes_read) {
      i += 1;
      c = input_buffer[i];
      if (c == DELIMETER || c  == '\0')
        break;
    }
  }
  result = route(argc, argv);

finish:
  // Print the error message, if any
  if (result > 0)
  {
    if (result < ERROR_CODE_COUNT)
      Serial.printf("ERROR: %s\n", error_code_list[result][1]);
    else
      Serial.printf("ERROR [%d]: INVALID ERROR CODE %s", result, '\n');
  }
  // And any payload that exists
  if (buffer[0]) {
    Serial.print(" ");
    Serial.print(buffer);
  }
  Serial.print("\n");
  return result;
}

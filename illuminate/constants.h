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

#include "illuminate.h"

#ifndef CONSTANTS_H
#define CONSTANTS_H

// PSU Sensing constants
#define PSU_SENSING_AND_MONITORING 2
#define PSU_SENSING_ONLY 1
#define NO_PSU_SENSING 0

// Trigger mode constants
#define TRIG_MODE_NONE 0
#define TRIG_MODE_ITERATION -1   // trigger at the start of each iteration (when the user
#define TRIG_MODE_START -2   // Triggering at the start of each acquisition

// Trigger timing constants
#define TRIGGER_OUTPUT_PULSE_WIDTH_DEFAULT 500
#define TRIGGER_OUTPUT_DELAY_DEFAULT 0
#define TRIGGER_TIMEOUT_DEFAULT 3600.0
#define TRIGGER_INPUT_POLARITY_DEFAULT 1
#define TRIGGER_OUTPUT_POLARITY_DEFAULT 1

// Misc constants
#define PVALS_USE_UINT8 1     // Whether to return uint8 for pvals instead of 16-bit (Default is on)
#define MIN_SEQUENCE_DELAY 5  // Min deblur pattern delay in ms (set by hardware)
#define MIN_SEQUENCE_DELAY_FAST 2 // Min deblur pattern delay for fast sequence in us (set by hardware)
#define DELAY_MAX 2000        // Global maximum amount to wait inside loop
#define INVALID_NA -2000.0    // Rep```resents an invalid NA

// Command mode constants
#define COMMAND_MODE_LONG 1
#define COMMAND_MODE_SHORT 0

// Response Lengths
#define MAX_RESPONSE_LENGTH_SHORT 100
#define MAX_RESPONSE_LENGTH_LONG 100

// Bit depth to use
#define USE_8_BIT_VALUES 1

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200
#define STORED_NA_ADDRESS 301
#define STORED_DISTANCE_ADDRESS 302
#define STORED_COLOR_R_ADDRESS 303
#define STORED_COLOR_G_ADDRESS 304
#define STORED_COLOR_B_ADDRESS 305
#define STORED_GSCLK_ADDRESS 306
#define STORED_SCLK_ADDRESS 307
#define STORED_BRIGHTNESS_ADDRESS 308

// Error Codes
#define ERROR_CODE_COUNT 18

#define NO_ERROR 0
#define ERROR_NOT_IMPLEMENTED 1
#define ERROR_ARGUMENT_COUNT 2
#define ERROR_SOURCE_DISCONNECTED 3
#define ERROR_ARGS_SET_TRIGGER_TIMEOUT 4
#define ERROR_COSINE_FACTOR 5
#define ERROR_MAC_ADDRESS 6
#define ERROR_POWER_SENSING_NOT_SUPPORTED 7
#define ERROR_INVALID_ARGUMENT 8
#define ERROR_NOT_SUPPORTED_BY_DEVICE 9
#define ERROR_ARGUMENT_RANGE 10
#define ERROR_TRIGGER_CONFIG 11
#define ERROR_END_OF_SEQUENCE 12
#define ERROR_SEQUENCE_DELAY 13
#define ERROR_TRIGGER_TIMEOUT 14
#define ERROR_COMMAND_INTERRUPTED 15
#define ERROR_INVALID_COMMAND 16
#define ERROR_MEMORY_ALLOC 17

#endif

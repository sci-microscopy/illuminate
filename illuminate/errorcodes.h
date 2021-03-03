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

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

// List of command indicies in below array
#define ERROR_CODE_COUNT 5

#define ERROR_CODE_NOT_IMPLEMENTED 0
#define ERROR_CODE_ARG_COUNT 1
#define ERROR_CODE_SOURCE_DISCONNECTED 2
#define ERROR_CODE_ARGS_SET_TRIGGER_TIMEOUT 3
#define ERROR_CODE_COSINE_FACTOR 4


// Syntax is: {short code, long error description}
const char* error_code_list[ERROR_CODE_COUNT][2] = {

  // High-level Errors
  {"NI", "Command not implemented."},

  // Sequence
  {"ARGS", "Wrong number of arguments.%s"},

  // Power source disconnected
  {"PSRC", "(LedArray::sourceConnected): Source Disconnected!%s"},

  // Trigger Settings
  {"TRIGT", "Trigger Timeout.%s"},

  // Cosine Factor
  {"COS", "Invalid cosine factor.%s"},
};

#endif

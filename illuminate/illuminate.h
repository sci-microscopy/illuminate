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

#ifndef ILLUMINATE_H
#define ILLUMINATE_H

#define VERSION 1.19

// This file allows the user to define which LED array interface is used. This should be set before compilation.
// The value these are set to does not matter - only that they are defined.
//#define USE_QUADRANT_ARRAY
//#define USE_LASER
//#define USE_QUASI_DOME_ARRAY
//#define USE_SCI_ROUND_ARRAY
//#define USE_SCI_WING_ARRAY
//#define USE_SCI_EPI_ARRAY
//#define USE_SCI_BIG_WING_ARRAY
//#define USE_C_006_RAMONA

// Serial line ending
static const char SERIAL_LINE_ENDING[] = "\n";

// Serial command termiator
static const char SERIAL_COMMAND_TERMINATOR[] = "-==-";

// Serial delimeter
static const char SERIAL_DELIMITER[] = ".";

#endif

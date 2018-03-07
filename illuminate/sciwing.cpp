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
   DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "illuminate.h"
#ifdef USE_SCI_WING_ARRAY
#include "ledarrayinterface.h"

// Pin definitions (used internally)
#define GSCLK 6 // 10 on Arduino Mega
#define LAT 3   // 44 on Arduino Mega
#define SPI_MOSI 11
#define SPI_CLK 13
#define TRIGGER_OUTPUT_PIN_0 23
#define TRIGGER_OUTPUT_PIN_1 22
#define TRIGGER_INPUT_PIN_0 20
#define TRIGGER_INPUT_PIN_1 19
#define TRIGGER_OUTPUT_COUNT 2
#define TRIGGER_INPUT_COUNT 2

#define SN 1

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "sci-wing";
const int LedArrayInterface::serial_number = SN;
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 1.0;
const int16_t LedArrayInterface::led_count = 225;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.48, 0.525, 0.625};
const int LedArrayInterface::bit_depth = 16;
const int16_t LedArrayInterface::tlc_chip_count = 16;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 60.0;

const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};
bool LedArrayInterface::trigger_input_state[] = {false, false};
//
//const uint8_t TLC5955::tlc_count = 16;                  // Number of TLC chips
//const uint8_t TLC5955::tlc_channel_count = 16;          // Number of channels per chip
//const uint8_t TLC5955::tlc_channel_color_count = 16;    // Number of colors per channel
 
int LedArrayInterface::debug = 0;

/**** Device-specific variables ****/
TLC5955 tlc; // TLC5955 object
uint32_t gsclk_frequency = 1000000;

// FORMAT: hole number, channel, 100*x, 100*y, 100*z
PROGMEM const int16_t LedArrayInterface::led_positions[225][5] = {
    {0, 277, 0, 0, 0},
    {1, 452, -417, 0, 0},
    {2, 337, 0, 417, 0},
    {3, 280, 417, 0, 0},
    {4, 271, 0, -417, 0},
    {5, 488, -417, -417, 0},
    {6, 449, -417, 417, 0},
    {7, 340, 417, 417, 0},
    {8, 274, 417, -417, 0},
    {9, 458, -835, 0, 0},
    {10, 331, 0, 835, 0},
    {11, 266, 835, 0, 0},
    {12, 283, 0, -835, 0},
    {13, 286, -417, -835, 0},
    {14, 263, -835, -417, 0},
    {15, 293, -835, 417, 0},
    {16, 334, -417, 835, 0},
    {17, 485, 417, 835, 0},
    {18, 455, 835, 417, 0},
    {19, 446, 835, -417, 0},
    {20, 494, 417, -835, 0},
    {21, 251, -835, -835, 0},
    {22, 299, -835, 835, 0},
    {23, 491, 835, 835, 0},
    {24, 443, 835, -835, 0},
    {25, 260, -1252, 0, 0},
    {26, 289, 0, 1252, 0},
    {27, 328, 1252, 0, 0},
    {28, 463, 0, -1252, 0},
    {29, 466, -417, -1252, 0},
    {30, 440, -1252, -417, 0},
    {31, 257, -1252, 417, 0},
    {32, 296, -417, 1252, 0},
    {33, 322, 417, 1252, 0},
    {34, 292, 1252, 417, 0},
    {35, 497, 1252, -417, 0},
    {36, 532, 417, -1252, 0},
    {37, 302, -835, -1252, 0},
    {38, 245, -1252, -835, 0},
    {39, 526, -1252, 835, 0},
    {40, 437, -835, 1252, 0},
    {41, 478, 835, 1252, 0},
    {42, 503, 1252, 835, 0},
    {43, 254, 1252, -835, 0},
    {44, 311, 835, -1252, 0},
    {45, 229, -1669, 0, 0},
    {46, 469, 0, 1669, 0},
    {47, 85, 1669, 0, 0},
    {48, 325, 0, -1669, 0},
    {49, 500, -417, -1669, 0},
    {50, 88, -1669, -417, 0},
    {51, 145, -1669, 417, 0},
    {52, 223, -417, 1669, 0},
    {53, 404, 417, 1669, 0},
    {54, 319, 1669, 417, 0},
    {55, 529, 1669, -417, 0},
    {56, 472, 417, -1669, 0},
    {57, 484, -1252, -1252, 0},
    {58, 305, -1252, 1252, 0},
    {59, 248, 1252, 1252, 0},
    {60, 514, 1252, -1252, 0},
    {61, 218, -835, -1669, 0},
    {62, 523, -1669, -835, 0},
    {63, 410, -1669, 835, 0},
    {64, 506, -835, 1669, 0},
    {65, 139, 835, 1669, 0},
    {66, 314, 1669, 835, 0},
    {67, 91, 1669, -835, 0},
    {68, 475, 835, -1669, 0},
    {69, 232, -1252, -1669, 0},
    {70, 79, -1669, -1252, 0},
    {71, 415, -2087, 0, 0},
    {72, 644, -1669, 1252, 0},
    {73, 385, -1252, 1669, 0},
    {74, 97, 0, 2087, 0},
    {75, 511, 1252, 1669, 0},
    {76, 481, 1669, 1252, 0},
    {77, 308, 2087, 0, 0},
    {78, 520, 1669, -1252, 0},
    {79, 136, 1252, -1669, 0},
    {80, 212, 0, -2087, 0},
    {81, 226, -417, -2087, 0},
    {82, 680, -2087, -417, 0},
    {83, 536, -2087, 417, 0},
    {84, 82, -417, 2087, 0},
    {85, 388, 417, 2087, 0},
    {86, 148, 2087, 417, 0},
    {87, 641, 2087, -417, 0},
    {88, 401, 417, -2087, 0},
    {89, 407, -835, -2087, 0},
    {90, 533, -2087, -835, 0},
    {91, 686, -2087, 835, 0},
    {92, 94, -835, 2087, 0},
    {93, 215, 835, 2087, 0},
    {94, 142, 2087, 835, 0},
    {95, 341, 2087, -835, 0},
    {96, 638, 835, -2087, 0},
    {97, 37, -1669, -1669, 0},
    {98, 517, -1669, 1669, 0},
    {99, 421, 1669, 1669, 0},
    {100, 133, 1669, -1669, 0},
    {101, 632, -1252, -2087, 0},
    {102, 209, -2087, -1252, 0},
    {103, 580, -2087, 1252, 0},
    {104, 418, -1252, 2087, 0},
    {105, 130, 1252, 2087, 0},
    {106, 689, 2087, 1252, 0},
    {107, 344, 2087, -1252, 0},
    {108, 100, 1252, -2087, 0},
    {109, 74, -2504, 0, 0},
    {110, 235, 0, 2504, 0},
    {111, 379, 2504, 0, 0},
    {112, 650, 0, -2504, 0},
    {113, 677, -417, -2504, 0},
    {114, 101, -2504, -417, 0},
    {115, 398, -2504, 417, 0},
    {116, 71, -417, 2504, 0},
    {117, 542, 417, 2504, 0},
    {118, 238, 2504, 417, 0},
    {119, 382, 2504, -417, 0},
    {120, 647, 417, -2504, 0},
    {121, 203, -835, -2504, 0},
    {122, 395, -2504, -835, 0},
    {123, 59, -2504, 835, 0},
    {124, 347, -835, 2504, 0},
    {125, 683, 835, 2504, 0},
    {126, 107, 2504, 835, 0},
    {127, 635, 2504, -835, 0},
    {128, 539, 835, -2504, 0},
    {129, 424, -1669, -2087, 0},
    {130, 596, -2087, -1669, 0},
    {131, 577, -2087, 1669, 0},
    {132, 127, -1669, 2087, 0},
    {133, 31, 1669, 2087, 0},
    {134, 692, 2087, 1669, 0},
    {135, 193, 2087, -1669, 0},
    {136, 40, 1669, -2087, 0},
    {137, 695, -1252, -2504, 0},
    {138, 629, -2504, -1252, 0},
    {139, 119, -2504, 1252, 0},
    {140, 206, -1252, 2504, 0},
    {141, 53, 1252, 2504, 0},
    {142, 430, 2504, 1252, 0},
    {143, 574, 2504, -1252, 0},
    {144, 350, 1252, -2504, 0},
    {145, 241, -2921, 0, 0},
    {146, 376, 0, 2921, 0},
    {147, 68, 2921, 0, 0},
    {148, 655, 0, -2921, 0},
    {149, 658, -417, -2921, 0},
    {150, 724, -2087, -2087, 0},
    {151, 593, -2921, -417, 0},
    {152, 728, -2921, 417, 0},
    {153, 196, -2087, 2087, 0},
    {154, 65, -417, 2921, 0},
    {155, 244, 417, 2921, 0},
    {156, 545, 2087, 2087, 0},
    {157, 104, 2921, 417, 0},
    {158, 370, 2921, -417, 0},
    {159, 34, 2087, -2087, 0},
    {160, 392, 417, -2921, 0},
    {161, 427, -1669, -2504, 0},
    {162, 26, -2504, -1669, 0},
    {163, 698, -2504, 1669, 0},
    {164, 187, -1669, 2504, 0},
    {165, 571, 1669, 2504, 0},
    {166, 122, 2504, 1669, 0},
    {167, 43, 2504, -1669, 0},
    {168, 602, 1669, -2504, 0},
    {169, 389, -835, -2921, 0},
    {170, 359, -2921, -835, 0},
    {171, 718, -2921, 835, 0},
    {172, 110, -835, 2921, 0},
    {173, 197, 835, 2921, 0},
    {174, 551, 2921, 835, 0},
    {175, 670, 2921, -835, 0},
    {176, 62, 835, -2921, 0},
    {177, 676, -1252, -2921, 0},
    {178, 706, -2921, -1252, 0},
    {179, 353, -2921, 1252, 0},
    {180, 562, -1252, 2921, 0},
    {181, 436, 1252, 2921, 0},
    {182, 113, 2921, 1252, 0},
    {183, 56, 2921, -1252, 0},
    {184, 200, 1252, -2921, 0},
    {185, 725, -2087, -2504, 0},
    {186, 734, -2504, -2087, 0},
    {187, 190, -2504, 2087, 0},
    {188, 46, -2087, 2504, 0},
    {189, 599, 2087, 2504, 0},
    {190, 149, 2504, 2087, 0},
    {191, 590, 2504, -2087, 0},
    {192, 23, 2087, -2504, 0},
    {193, 607, -1669, -2921, 0},
    {194, 49, -2921, -1669, 0},
    {195, 433, -2921, 1669, 0},
    {196, 703, -1669, 2921, 0},
    {197, 568, 1669, 2921, 0},
    {198, 184, 2921, 1669, 0},
    {199, 116, 2921, -1669, 0},
    {200, 20, 1669, -2921, 0},
    {201, 155, -2504, -2504, 0},
    {202, 731, -2504, 2504, 0},
    {203, 587, 2504, 2504, 0},
    {204, 11, 2504, -2504, 0},
    {205, 737, -2087, -2921, 0},
    {206, 178, -2921, -2087, 0},
    {207, 772, -2921, 2087, 0},
    {208, 52, -2087, 2921, 0},
    {209, 152, 2087, 2921, 0},
    {210, 610, 2921, 2087, 0},
    {211, 17, 2921, -2087, 0},
    {212, 584, 2087, -2921, 0},
    {213, 581, -2504, -2921, 0},
    {214, 5, -2921, -2504, 0},
    {215, 743, -2921, 2504, 0},
    {216, 622, -2504, 2921, 0},
    {217, 167, 2504, 2921, 0},
    {218, 766, 2921, 2504, 0},
    {219, 14, 2921, -2504, 0},
    {220, 158, 2504, -2921, 0},
    {221, 8, -2921, -2921, 0},
    {222, 161, -2921, 2921, 0},
    {223, 628, 2921, 2921, 0},
    {224, 754, 2921, -2921, 0}
};

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
  tlc.setPinOrderSingle(led_number, color_channel_index, position);
}

void LedArrayInterface::notImplemented(const char * command_name)
{
  Serial.print(F("Command "));
  Serial.print(command_name);
  Serial.println(F(" is not implemented for this device."));
}

uint16_t LedArrayInterface::getLedValue(uint16_t led_number, int color_channel_index)
{
  int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
  if (channel_number >= 0)
    return tlc.getChannelValue(channel_number, color_channel_index);
  else
  {
    Serial.print(F("ERROR (LedArrayInterface::getLedValue) - invalid LED number ("));
    Serial.print(led_number);
    Serial.println(F(")"));
    return 0;
  }
}

void LedArrayInterface::setLedFast(int16_t led_number, int color_channel_index, bool value)
{
  notImplemented("setLedFast");
}

// Debug Variables
bool LedArrayInterface::getDebug()
{
  return (LedArrayInterface::debug);
}

void LedArrayInterface::setDebug(int state)
{
  LedArrayInterface::debug = state;
  Serial.printf(F("(LedArrayInterface::setDebug): Set debug level to %d \n"), debug);
}

int LedArrayInterface::setTriggerState(int trigger_index, bool state)
{
  // Get trigger pin
  int trigger_pin = trigger_output_pin_list[trigger_index];
  if (trigger_pin > 0)
  {
    if (state)
      digitalWriteFast(trigger_pin, HIGH);
    else
      digitalWriteFast(trigger_pin, LOW);
    return (1);
  } else {
    return (-1);
  }
}

int LedArrayInterface::getInputTriggerState(int input_trigger_index)
{
  // Get trigger pin
  int trigger_pin = trigger_input_pin_list[input_trigger_index];
  if (trigger_pin > 0)
    return (trigger_input_state[trigger_pin]);
  else
    return (-1);
}

int LedArrayInterface::sendTriggerPulse(int trigger_index, uint16_t delay_us, bool inverse_polarity)
{
  // Get trigger pin
  int trigger_pin = trigger_output_pin_list[trigger_index];

  if (trigger_pin > 0)
  {
    // Write active state
    if (inverse_polarity)
      digitalWriteFast(trigger_pin, LOW);
    else
      digitalWriteFast(trigger_pin, HIGH);

    // Delay if desired
    if (delay_us > 0)
      delayMicroseconds(delay_us);

    // Write normal state
    if (inverse_polarity)
      digitalWriteFast(trigger_pin, HIGH);
    else
      digitalWriteFast(trigger_pin, LOW);
    return (1);
  } else {
    return (-1);
  }
}
void LedArrayInterface::update()
{
  tlc.updateLeds();
}

void LedArrayInterface::clear()
{
  tlc.setAllLed(0);
  tlc.updateLeds();
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint16_t value)
{
  if (debug >= 2)
  {
    Serial.print(F("Drawing channel #"));
    Serial.print(channel_number);
    Serial.print(F(", color_channel #"));
    Serial.print(color_channel_number);
    Serial.print(F(" to value "));
    Serial.println(value);
  }

  if (channel_number >= 0)
  {
    if (color_channel_number < 0)
      tlc.setLed(channel_number, value);
    else if (color_channel_number == 0)
      tlc.setLed(channel_number, value, tlc.getChannelValue(channel_number, 1), tlc.getChannelValue(channel_number, 2));
    else if (color_channel_number == 1)
      tlc.setLed(channel_number, tlc.getChannelValue(channel_number, 0), value, tlc.getChannelValue(channel_number, 2));
    else if (color_channel_number == 2)
      tlc.setLed(channel_number, tlc.getChannelValue(channel_number, 0), tlc.getChannelValue(channel_number, 1), value);
  }
  else
  {
    Serial.print(F("Error (LedArrayInterface::setChannel): Invalid channel ("));
    Serial.print(channel_number);
    Serial.println(F(")"));
  }
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, uint8_t value)
{
  setChannel(channel_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setChannel(int16_t channel_number, int16_t color_channel_number, bool value)
{
  setChannel(channel_number, color_channel_number, (uint16_t) (value > 0 * UINT16_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint16_t value)
{
  if (debug >= 2)
  {
    Serial.print("U16 Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.print(color_channel_number);
    Serial.print(" to value ");
    Serial.println(value);
  }
  if (led_number < 0)
  {
    for (uint16_t led_index = 0; led_index < led_count; led_index++)
    {
      int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_index][1]));
      setChannel(channel_number, color_channel_number, value);
    }
  }
  else
  {
    int16_t channel_number = (int16_t)pgm_read_word(&(led_positions[led_number][1]));
    setChannel(channel_number, color_channel_number, value);
  }
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, uint8_t value)
{
  if (debug >= 2)
  {
    Serial.print("U8 Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.println(color_channel_number);
  }
  setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX / UINT8_MAX));
}

void LedArrayInterface::setLed(int16_t led_number, int16_t color_channel_number, bool value)
{
  if (debug >= 2)
  {
    Serial.print("B Setting led #");
    Serial.print(led_number);
    Serial.print(", color channel #");
    Serial.println(color_channel_number);
  }
  setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::deviceReset()
{
  deviceSetup();
}

void LedArrayInterface::deviceSetup()
{
  // Now set the GSCLK to an output and a 50% PWM duty-cycle
  // For simplicity all three grayscale clocks are tied to the same pin
  pinMode(GSCLK, OUTPUT);
  pinMode(LAT, OUTPUT);

  // Adjust PWM timer for maximum GSCLK frequency
  analogWriteFrequency(GSCLK, gsclk_frequency);
  analogWriteResolution(1);
  analogWrite(GSCLK, 1);

  // The library does not ininiate SPI for you, so as to prevent issues with other SPI libraries
  SPI.setMOSI(SPI_MOSI);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);

  tlc.init(LAT, SPI_MOSI, SPI_CLK);

  // We must set dot correction values, so set them all to the brightest adjustment
  tlc.setAllDcData(127);

  // Set Max Current Values (see TLC5955 datasheet)
  tlc.setMaxCurrent(3, 3, 3); // Go up to 7

  // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
  // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT
  tlc.setFunctionData(true, true, false, true, true); // WORKS with fast update

  // set all brightness levels to max (127)
  int currentR = 127;
  int currentB = 127;
  int currentG = 127;
  tlc.setBrightnessCurrent(currentR, currentB, currentG);

  // Update Control Register
  tlc.updateControl();

  // Provide corrections for LEDs which have issues
  tlc.setRgbPinOrder(2, 1, 0);

  // Update the GS register (ideally LEDs should be dark up to here)
  clear();

  // Output trigger Pins
  for (int trigger_index = 0; trigger_index < trigger_output_count; trigger_index++)
  {
    pinMode(trigger_output_pin_list[trigger_index], OUTPUT);
    digitalWriteFast(trigger_output_pin_list[trigger_index], LOW);
  }

  // Input trigger Pins
  for (int trigger_index = 0; trigger_index < trigger_input_count; trigger_index++)
    pinMode(trigger_input_pin_list[trigger_index], INPUT);
}
#endif

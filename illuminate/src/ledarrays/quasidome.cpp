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
#include "../../illuminate.h"
#ifdef USE_QUASI_DOME_ARRAY
#include "../../ledarrayinterface.h"
#include "../TLC5955/TLC5955.h"

// Pin definitions (used internally)
const int GSCLK = 6;
const int LAT = 3;
const int SPI_MOSI = 11;
const int SPI_CLK = 13;
const int TRIGGER_OUTPUT_PIN_0 = 23;
const int TRIGGER_INPUT_PIN_0 = 22;
const int TRIGGER_OUTPUT_PIN_1 = 20;
const int TRIGGER_INPUT_PIN_1 = 19;
const int TRIGGER_OUTPUT_COUNT = 2;
const int TRIGGER_INPUT_COUNT = 2;

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "Waller Lab Quasi-Dome";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 0.98;
const int16_t LedArrayInterface::led_count = 581;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 2;
const int LedArrayInterface::trigger_input_count = 2;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.48, 0.525, 0.625};
const int LedArrayInterface::bit_depth = 16;
const int16_t LedArrayInterface::tlc_chip_count = 38;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 60.0;

const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0, TRIGGER_OUTPUT_PIN_1};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0, TRIGGER_INPUT_PIN_1};
bool LedArrayInterface::trigger_input_state[] = {false, false};

int LedArrayInterface::debug = 0;

const uint8_t TLC5955::_tlc_count = 37;          // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 2.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc; // TLC5955 object
uint32_t gsclk_frequency = 5000000;

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 0;
const char * LedArrayInterface::deviceCommandNamesShort[] = {};
const char * LedArrayInterface::deviceCommandNamesLong[] = {};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {};

/**** Part number and Serial number addresses in EEPROM ****/
uint16_t pn_address = 100;
uint16_t sn_address = 200;

// FORMAT: hole number, channel, 100*x, 100*y, 100*z
PROGMEM const int16_t LedArrayInterface::led_positions[][5] = {
        {0, 68, 0, 0, 5000},
        {1, 65, -361, 0, 5000},
        {2, 56, 0, -361, 5000},
        {3, 74, 0, 361, 5000},
        {4, 71, 361, 0, 5000},
        {5, 69, -362, -362, 5000},
        {6, 66, -362, 362, 5000},
        {7, 72, 362, -362, 5000},
        {8, 78, 362, 362, 5000},
        {9, 23, -728, 0, 5000},
        {10, 14, 0, -728, 5000},
        {11, 37, 0, 728, 5000},
        {12, 76, 728, 0, 5000},
        {13, 64, -729, -365, 5000},
        {14, 28, -729, 365, 5000},
        {15, 1, -365, -729, 5000},
        {16, 70, -365, 729, 5000},
        {17, 51, 365, -729, 5000},
        {18, 75, 365, 729, 5000},
        {19, 63, 729, -365, 5000},
        {20, 77, 729, 365, 5000},
        {21, 2, -735, -735, 5000},
        {22, 25, -735, 735, 5000},
        {23, 55, 735, -735, 5000},
        {24, 79, 735, 735, 5000},
        {25, 19, -1106, 0, 5000},
        {26, 15, 0, -1106, 5000},
        {27, 32, 0, 1106, 5000},
        {28, 43, 1106, 0, 5000},
        {29, 3, -1109, -370, 5000},
        {30, 18, -1109, 370, 5000},
        {31, 13, -370, -1109, 5000},
        {32, 67, -370, 1109, 5000},
        {33, 60, 370, -1109, 5000},
        {34, 33, 370, 1109, 5000},
        {35, 58, 1109, -370, 5000},
        {36, 73, 1109, 370, 5000},
        {37, 6, -1118, -745, 5000},
        {38, 26, -1118, 745, 5000},
        {39, 5, -745, -1118, 5000},
        {40, 29, -745, 1118, 5000},
        {41, 48, 745, -1118, 5000},
        {42, 41, 745, 1118, 5000},
        {43, 59, 1118, -745, 5000},
        {44, 42, 1118, 745, 5000},
        {45, 22, -1504, 0, 5000},
        {46, 11, 0, -1504, 5000},
        {47, 36, 0, 1504, 5000},
        {48, 47, 1504, 0, 5000},
        {49, 7, -1508, -377, 5000},
        {50, 17, -1508, 377, 5000},
        {51, 10, -377, -1508, 5000},
        {52, 24, -377, 1508, 5000},
        {53, 52, 377, -1508, 5000},
        {54, 34, 377, 1508, 5000},
        {55, 62, 1508, -377, 5000},
        {56, 46, 1508, 377, 5000},
        {57, 0, -1133, -1133, 5000},
        {58, 31, -1133, 1133, 5000},
        {59, 49, 1133, -1133, 5000},
        {60, 44, 1133, 1133, 5000},
        {61, 585, -1520, -760, 5000},
        {62, 21, -1520, 760, 5000},
        {63, 9, -760, -1520, 5000},
        {64, 30, -760, 1520, 5000},
        {65, 57, 760, -1520, 5000},
        {66, 38, 760, 1520, 5000},
        {67, 61, 1520, -760, 5000},
        {68, 45, 1520, 760, 5000},
        {69, 4, -1540, -1155, 5000},
        {70, 16, -1540, 1155, 5000},
        {71, 12, -1155, -1540, 5000},
        {72, 27, -1155, 1540, 5000},
        {73, 53, 1155, -1540, 5000},
        {74, 35, 1155, 1540, 5000},
        {75, 54, 1540, -1155, 5000},
        {76, 40, 1540, 1155, 5000},
        {77, 8, -1570, -1570, 5000},
        {78, 20, -1570, 1570, 5000},
        {79, 50, 1570, -1570, 5000},
        {80, 39, 1570, 1570, 5000},
        {81, 489, -1825, 0, 4785},
        {82, 361, 0, 1825, 4785},
        {83, 105, 0, -1825, 4785},
        {84, 233, 1825, 0, 4785},
        {85, 488, -1832, -523, 4778},
        {86, 464, -1832, 523, 4778},
        {87, 80, -523, -1832, 4778},
        {88, 360, -523, 1832, 4778},
        {89, 336, 523, 1832, 4778},
        {90, 208, 1832, -523, 4778},
        {91, 232, 1832, 523, 4778},
        {92, 104, 523, -1832, 4778},
        {93, 465, -1999, 256, 4611},
        {94, 81, -256, -1999, 4611},
        {95, 494, -1999, -256, 4611},
        {96, 366, -256, 1999, 4611},
        {97, 337, 256, 1999, 4611},
        {98, 110, 256, -1999, 4611},
        {99, 209, 1999, -256, 4611},
        {100, 238, 1999, 256, 4611},
        {101, 484, -1853, -1059, 4757},
        {102, 356, -1059, 1853, 4757},
        {103, 100, 1059, -1853, 4757},
        {104, 228, 1853, 1059, 4757},
        {105, 468, -1853, 1059, 4757},
        {106, 84, -1059, -1853, 4757},
        {107, 340, 1059, 1853, 4757},
        {108, 212, 1853, -1059, 4757},
        {109, 492, -2014, -774, 4597},
        {110, 364, -774, 2014, 4597},
        {111, 85, -774, -2014, 4597},
        {112, 108, 774, -2014, 4597},
        {113, 213, 2014, -774, 4597},
        {114, 236, 2014, 774, 4597},
        {115, 469, -2014, 774, 4597},
        {116, 341, 774, 2014, 4597},
        {117, 467, -2249, 0, 4362},
        {118, 339, 0, 2249, 4362},
        {119, 83, 0, -2249, 4362},
        {120, 211, 2249, 0, 4362},
        {121, 480, -1891, -1621, 4720},
        {122, 352, -1621, 1891, 4720},
        {123, 490, -2256, -501, 4354},
        {124, 470, -2256, 501, 4354},
        {125, 472, -1891, 1621, 4720},
        {126, 88, -1621, -1891, 4720},
        {127, 362, -501, 2256, 4354},
        {128, 86, -501, -2256, 4354},
        {129, 342, 501, 2256, 4354},
        {130, 106, 501, -2256, 4354},
        {131, 214, 2256, -501, 4354},
        {132, 234, 2256, 501, 4354},
        {133, 344, 1621, 1891, 4720},
        {134, 96, 1621, -1891, 4720},
        {135, 224, 1891, 1621, 4720},
        {136, 216, 1891, -1621, 4720},
        {137, 476, -2044, 1310, 4566},
        {138, 92, -1310, -2044, 4566},
        {139, 485, -2044, -1310, 4566},
        {140, 357, -1310, 2044, 4566},
        {141, 348, 1310, 2044, 4566},
        {142, 101, 1310, -2044, 4566},
        {143, 229, 2044, 1310, 4566},
        {144, 220, 2044, -1310, 4566},
        {145, 493, -2280, -1013, 4330},
        {146, 466, -2280, 1013, 4330},
        {147, 365, -1013, 2280, 4330},
        {148, 82, -1013, -2280, 4330},
        {149, 210, 2280, -1013, 4330},
        {150, 338, 1013, 2280, 4330},
        {151, 109, 1013, -2280, 4330},
        {152, 237, 2280, 1013, 4330},
        {153, 516, -2459, -246, 4152},
        {154, 132, 246, -2459, 4152},
        {155, 520, -2459, 246, 4152},
        {156, 388, -246, 2459, 4152},
        {157, 136, -246, -2459, 4152},
        {158, 392, 246, 2459, 4152},
        {159, 264, 2459, -246, 4152},
        {160, 260, 2459, 246, 4152},
        {161, 495, -2475, -743, 4135},
        {162, 471, -2475, 743, 4135},
        {163, 367, -743, 2475, 4135},
        {164, 87, -743, -2475, 4135},
        {165, 343, 743, 2475, 4135},
        {166, 111, 743, -2475, 4135},
        {167, 215, 2475, -743, 4135},
        {168, 239, 2475, 743, 4135},
        {169, 473, -2136, 1869, 4475},
        {170, 481, -2136, -1869, 4475},
        {171, 353, -1869, 2136, 4475},
        {172, 89, -1869, -2136, 4475},
        {173, 345, 1869, 2136, 4475},
        {174, 97, 1869, -2136, 4475},
        {175, 225, 2136, 1869, 4475},
        {176, 217, 2136, -1869, 4475},
        {177, 477, -2322, 1548, 4289},
        {178, 93, -1548, -2322, 4289},
        {179, 482, -2322, -1548, 4289},
        {180, 354, -1548, 2322, 4289},
        {181, 349, 1548, 2322, 4289},
        {182, 98, 1548, -2322, 4289},
        {183, 221, 2322, -1548, 4289},
        {184, 226, 2322, 1548, 4289},
        {185, 524, -2666, 0, 3944},
        {186, 396, 0, 2666, 3944},
        {187, 140, 0, -2666, 3944},
        {188, 268, 2666, 0, 3944},
        {189, 363, -1254, 2509, 4102},
        {190, 491, -2509, -1254, 4102},
        {191, 474, -2509, 1254, 4102},
        {192, 90, -1254, -2509, 4102},
        {193, 512, -2674, -486, 3936},
        {194, 499, -2674, 486, 3936},
        {195, 384, -486, 2674, 3936},
        {196, 115, -486, -2674, 3936},
        {197, 371, 486, 2674, 3936},
        {198, 128, 486, -2674, 3936},
        {199, 346, 1254, 2509, 4102},
        {200, 218, 2509, -1254, 4102},
        {201, 243, 2674, -486, 3936},
        {202, 107, 1254, -2509, 4102},
        {203, 235, 2509, 1254, 4102},
        {204, 256, 2674, 486, 3936},
        {205, 502, -2700, 982, 3910},
        {206, 528, -2700, -982, 3910},
        {207, 400, -982, 2700, 3910},
        {208, 118, -982, -2700, 3910},
        {209, 374, 982, 2700, 3910},
        {210, 144, 982, -2700, 3910},
        {211, 246, 2700, -982, 3910},
        {212, 272, 2700, 982, 3910},
        {213, 478, -2385, 2120, 4225},
        {214, 483, -2385, -2120, 4225},
        {215, 355, -2120, 2385, 4225},
        {216, 94, -2120, -2385, 4225},
        {217, 350, 2120, 2385, 4225},
        {218, 99, 2120, -2385, 4225},
        {219, 227, 2385, 2120, 4225},
        {220, 222, 2385, -2120, 4225},
        {221, 517, -2881, -240, 3729},
        {222, 503, -2881, 240, 3729},
        {223, 389, -240, 2881, 3729},
        {224, 119, -240, -2881, 3729},
        {225, 375, 240, 2881, 3729},
        {226, 133, 240, -2881, 3729},
        {227, 247, 2881, -240, 3729},
        {228, 261, 2881, 240, 3729},
        {229, 486, -2563, -1794, 4048},
        {230, 479, -2563, 1794, 4048},
        {231, 358, -1794, 2563, 4048},
        {232, 95, -1794, -2563, 4048},
        {233, 351, 1794, 2563, 4048},
        {234, 102, 1794, -2563, 4048},
        {235, 223, 2563, -1794, 4048},
        {236, 230, 2563, 1794, 4048},
        {237, 507, -2898, 725, 3712},
        {238, 385, -725, 2898, 3712},
        {239, 123, -725, -2898, 3712},
        {240, 513, -2898, -725, 3712},
        {241, 379, 725, 2898, 3712},
        {242, 129, 725, -2898, 3712},
        {243, 257, 2898, 725, 3712},
        {244, 251, 2898, -725, 3712},
        {245, 475, -2744, 1497, 3866},
        {246, 406, -1497, 2744, 3866},
        {247, 91, -1497, -2744, 3866},
        {248, 347, 1497, 2744, 3866},
        {249, 278, 2744, 1497, 3866},
        {250, 219, 2744, -1497, 3866},
        {251, 534, -2744, -1497, 3866},
        {252, 150, 1497, -2744, 3866},
        {253, 521, -3100, 0, 3511},
        {254, 529, -2934, -1222, 3677},
        {255, 498, -2934, 1222, 3677},
        {256, 401, -1222, 2934, 3677},
        {257, 114, -1222, -2934, 3677},
        {258, 370, 1222, 2934, 3677},
        {259, 145, 1222, -2934, 3677},
        {260, 273, 2934, 1222, 3677},
        {261, 242, 2934, -1222, 3677},
        {262, 393, 0, 3100, 3511},
        {263, 137, 0, -3100, 3511},
        {264, 265, 3100, 0, 3511},
        {265, 514, -3108, -478, 3502},
        {266, 526, -3108, 478, 3502},
        {267, 386, -478, 3108, 3502},
        {268, 142, -478, -3108, 3502},
        {269, 398, 478, 3108, 3502},
        {270, 130, 478, -3108, 3502},
        {271, 270, 3108, -478, 3502},
        {272, 258, 3108, 478, 3502},
        {273, 500, -2641, 2377, 3969},
        {274, 487, -2641, -2377, 3969},
        {275, 359, -2377, 2641, 3969},
        {276, 116, -2377, -2641, 3969},
        {277, 372, 2377, 2641, 3969},
        {278, 103, 2377, -2641, 3969},
        {279, 231, 2641, 2377, 3969},
        {280, 244, 2641, -2377, 3969},
        {281, 122, -964, -3135, 3476},
        {282, 250, 3135, -964, 3476},
        {283, 532, -3135, -964, 3476},
        {284, 506, -3135, 964, 3476},
        {285, 404, -964, 3135, 3476},
        {286, 378, 964, 3135, 3476},
        {287, 148, 964, -3135, 3476},
        {288, 276, 3135, 964, 3476},
        {289, 496, -2812, 2045, 3799},
        {290, 531, -2812, -2045, 3799},
        {291, 403, -2045, 2812, 3799},
        {292, 112, -2045, -2812, 3799},
        {293, 368, 2045, 2812, 3799},
        {294, 147, 2045, -2812, 3799},
        {295, 275, 2812, 2045, 3799},
        {296, 240, 2812, -2045, 3799},
        {297, 530, -2990, -1744, 3620},
        {298, 402, -1744, 2990, 3620},
        {299, 117, -1744, -2990, 3620},
        {300, 146, 1744, -2990, 3620},
        {301, 274, 2990, 1744, 3620},
        {302, 245, 2990, -1744, 3620},
        {303, 501, -2990, 1744, 3620},
        {304, 373, 1744, 2990, 3620},
        {305, 141, -238, -3334, 3276},
        {306, 518, -3334, -238, 3276},
        {307, 525, -3334, 238, 3276},
        {308, 390, -238, 3334, 3276},
        {309, 397, 238, 3334, 3276},
        {310, 134, 238, -3334, 3276},
        {311, 269, 3334, -238, 3276},
        {312, 262, 3334, 238, 3276},
        {313, 515, -3351, -718, 3259},
        {314, 522, -3351, 718, 3259},
        {315, 533, -3180, -1468, 3430},
        {316, 405, -1468, 3180, 3430},
        {317, 387, -718, 3351, 3259},
        {318, 138, -718, -3351, 3259},
        {319, 394, 718, 3351, 3259},
        {320, 131, 718, -3351, 3259},
        {321, 149, 1468, -3180, 3430},
        {322, 277, 3180, 1468, 3430},
        {323, 266, 3351, -718, 3259},
        {324, 259, 3351, 718, 3259},
        {325, 497, -3180, 1468, 3430},
        {326, 113, -1468, -3180, 3430},
        {327, 369, 1468, 3180, 3430},
        {328, 241, 3180, -1468, 3430},
        {329, 511, -3387, 1210, 3223},
        {330, 558, -2908, 2644, 3703},
        {331, 535, -2908, -2644, 3703},
        {332, 407, -2644, 2908, 3703},
        {333, 174, -2644, -2908, 3703},
        {334, 383, 1210, 3387, 3223},
        {335, 540, -3387, -1210, 3223},
        {336, 412, -1210, 3387, 3223},
        {337, 127, -1210, -3387, 3223},
        {338, 156, 1210, -3387, 3223},
        {339, 430, 2644, 2908, 3703},
        {340, 151, 2644, -2908, 3703},
        {341, 279, 2908, 2644, 3703},
        {342, 302, 2908, -2644, 3703},
        {343, 284, 3387, 1210, 3223},
        {344, 255, 3387, -1210, 3223},
        {345, 523, -3582, 0, 3028},
        {346, 548, -3072, 2304, 3538},
        {347, 542, -3072, -2304, 3538},
        {348, 414, -2304, 3072, 3538},
        {349, 164, -2304, -3072, 3538},
        {350, 395, 0, 3582, 3028},
        {351, 139, 0, -3582, 3028},
        {352, 267, 3582, 0, 3028},
        {353, 420, 2304, 3072, 3538},
        {354, 158, 2304, -3072, 3538},
        {355, 286, 3072, 2304, 3538},
        {356, 292, 3072, -2304, 3538},
        {357, 519, -3591, -479, 3020},
        {358, 527, -3591, 479, 3020},
        {359, 391, -479, 3591, 3020},
        {360, 143, -479, -3591, 3020},
        {361, 399, 479, 3591, 3020},
        {362, 135, 479, -3591, 3020},
        {363, 271, 3591, -479, 3020},
        {364, 263, 3591, 479, 3020},
        {365, 508, -3249, 2000, 3361},
        {366, 124, -2000, -3249, 3361},
        {367, 541, -3249, -2000, 3361},
        {368, 413, -2000, 3249, 3361},
        {369, 285, 3249, 2000, 3361},
        {370, 252, 3249, -2000, 3361},
        {371, 380, 2000, 3249, 3361},
        {372, 157, 2000, -3249, 3361},
        {373, 560, -3617, 964, 2994},
        {374, 176, -964, -3617, 2994},
        {375, 432, 964, 3617, 2994},
        {376, 304, 3617, -964, 2994},
        {377, 536, -3617, -964, 2994},
        {378, 408, -964, 3617, 2994},
        {379, 152, 964, -3617, 2994},
        {380, 280, 3617, 964, 2994},
        {381, 537, -3444, -1722, 3166},
        {382, 121, -1722, -3444, 3166},
        {383, 505, -3444, 1722, 3166},
        {384, 409, -1722, 3444, 3166},
        {385, 377, 1722, 3444, 3166},
        {386, 153, 1722, -3444, 3166},
        {387, 281, 3444, 1722, 3166},
        {388, 249, 3444, -1722, 3166},
        {389, 566, -3863, -241, 2747},
        {390, 562, -3863, 241, 2747},
        {391, 434, 241, 3863, 2747},
        {392, 182, 241, -3863, 2747},
        {393, 438, -241, 3863, 2747},
        {394, 178, -241, -3863, 2747},
        {395, 306, 3863, -241, 2747},
        {396, 310, 3863, 241, 2747},
        {397, 576, -3662, -1465, 2948},
        {398, 448, -1465, 3662, 2948},
        {399, 510, -3662, 1465, 2948},
        {400, 126, -1465, -3662, 2948},
        {401, 382, 1465, 3662, 2948},
        {402, 320, 3662, 1465, 2948},
        {403, 192, 1465, -3662, 2948},
        {404, 254, 3662, -1465, 2948},
        {405, 557, -3188, 2922, 3422},
        {406, 539, -3188, -2922, 3422},
        {407, 411, -2922, 3188, 3422},
        {408, 173, -2922, -3188, 3422},
        {409, 429, 2922, 3188, 3422},
        {410, 155, 2922, -3188, 3422},
        {411, 283, 3188, 2922, 3422},
        {412, 301, 3188, -2922, 3422},
        {413, 567, -3880, -727, 2730},
        {414, 565, -3880, 727, 2730},
        {415, 439, -727, 3880, 2730},
        {416, 181, -727, -3880, 2730},
        {417, 437, 727, 3880, 2730},
        {418, 183, 727, -3880, 2730},
        {419, 309, 3880, -727, 2730},
        {420, 311, 3880, 727, 2730},
        {421, 543, -3347, -2575, 3263},
        {422, 415, -2575, 3347, 3263},
        {423, 159, 2575, -3347, 3263},
        {424, 287, 3347, 2575, 3263},
        {425, 544, -3347, 2575, 3263},
        {426, 160, -2575, -3347, 3263},
        {427, 416, 2575, 3347, 3263},
        {428, 288, 3347, -2575, 3263},
        {429, 504, -3526, 2267, 3084},
        {430, 376, 2267, 3526, 3084},
        {431, 538, -3526, -2267, 3084},
        {432, 410, -2267, 3526, 3084},
        {433, 120, -2267, -3526, 3084},
        {434, 154, 2267, -3526, 3084},
        {435, 248, 3526, -2267, 3084},
        {436, 282, 3526, 2267, 3084},
        {437, 580, -3915, -1223, 2696},
        {438, 452, -1223, 3915, 2696},
        {439, 564, -3915, 1223, 2696},
        {440, 180, -1223, -3915, 2696},
        {441, 436, 1223, 3915, 2696},
        {442, 308, 3915, -1223, 2696},
        {443, 196, 1223, -3915, 2696},
        {444, 324, 3915, 1223, 2696},
        {445, 574, -4187, 0, 2423},
        {446, 446, 0, 4187, 2423},
        {447, 190, 0, -4187, 2423},
        {448, 318, 4187, 0, 2423},
        {449, 509, -3730, 1989, 2880},
        {450, 577, -3730, -1989, 2880},
        {451, 125, -1989, -3730, 2880},
        {452, 381, 1989, 3730, 2880},
        {453, 193, 1989, -3730, 2880},
        {454, 253, 3730, -1989, 2880},
        {455, 449, -1989, 3730, 2880},
        {456, 321, 3730, 1989, 2880},
        {457, 563, -4195, -494, 2415},
        {458, 179, 494, -4195, 2415},
        {459, 568, -4195, 494, 2415},
        {460, 184, -494, -4195, 2415},
        {461, 440, 494, 4195, 2415},
        {462, 312, 4195, -494, 2415},
        {463, 435, -494, 4195, 2415},
        {464, 307, 4195, 494, 2415},
        {465, 588, -4220, -993, 2391},
        {466, 453, -1737, 3969, 2641},
        {467, 325, 3969, 1737, 2641},
        {468, 559, -4220, 993, 2391},
        {469, 547, -3969, 1737, 2641},
        {470, 581, -3969, -1737, 2641},
        {471, 163, -1737, -3969, 2641},
        {472, 460, -993, 4220, 2391},
        {473, 175, -993, -4220, 2391},
        {474, 431, 993, 4220, 2391},
        {475, 419, 1737, 3969, 2641},
        {476, 197, 1737, -3969, 2641},
        {477, 291, 3969, -1737, 2641},
        {478, 303, 4220, -993, 2391},
        {479, 204, 993, -4220, 2391},
        {480, 332, 4220, 993, 2391},
        {481, 553, -3485, 3217, 3125},
        {482, 583, -3485, -3217, 3125},
        {483, 455, -3217, 3485, 3125},
        {484, 169, -3217, -3485, 3125},
        {485, 425, 3217, 3485, 3125},
        {486, 199, 3217, -3485, 3125},
        {487, 327, 3485, 3217, 3125},
        {488, 297, 3485, -3217, 3125},
        {489, 579, -3642, -2861, 2969},
        {490, 451, -2861, 3642, 2969},
        {491, 195, 2861, -3642, 2969},
        {492, 323, 3642, 2861, 2969},
        {493, 549, -3642, 2861, 2969},
        {494, 165, -2861, -3642, 2969},
        {495, 421, 2861, 3642, 2969},
        {496, 293, 3642, -2861, 2969},
        {497, 570, -4603, -256, 2008},
        {498, 442, -256, 4603, 2008},
        {499, 186, 256, -4603, 2008},
        {500, 314, 4603, 256, 2008},
        {501, 573, -4603, 256, 2008},
        {502, 551, -4262, 1504, 2349},
        {503, 590, -4262, -1504, 2349},
        {504, 545, -3826, 2551, 2784},
        {505, 161, -2551, -3826, 2784},
        {506, 462, -1504, 4262, 2349},
        {507, 167, -1504, -4262, 2349},
        {508, 189, -256, -4603, 2008},
        {509, 445, 256, 4603, 2008},
        {510, 423, 1504, 4262, 2349},
        {511, 206, 1504, -4262, 2349},
        {512, 417, 2551, 3826, 2784},
        {513, 289, 3826, -2551, 2784},
        {514, 334, 4262, 1504, 2349},
        {515, 295, 4262, -1504, 2349},
        {516, 317, 4603, -256, 2008},
        {517, 582, -3826, -2551, 2784},
        {518, 454, -2551, 3826, 2784},
        {519, 198, 2551, -3826, 2784},
        {520, 326, 3826, 2551, 2784},
        {521, 584, -4617, -770, 1993},
        {522, 555, -4617, 770, 1993},
        {523, 456, -770, 4617, 1993},
        {524, 171, -770, -4617, 1993},
        {525, 200, 770, -4617, 1993},
        {526, 427, 770, 4617, 1993},
        {527, 299, 4617, -770, 1993},
        {528, 328, 4617, 770, 1993},
        {529, 546, -4048, 2277, 2562},
        {530, 578, -4048, -2277, 2562},
        {531, 162, -2277, -4048, 2562},
        {532, 418, 2277, 4048, 2562},
        {533, 194, 2277, -4048, 2562},
        {534, 290, 4048, -2277, 2562},
        {535, 450, -2277, 4048, 2562},
        {536, 322, 4048, 2277, 2562},
        {537, 554, -4647, 1291, 1963},
        {538, 589, -4647, -1291, 1963},
        {539, 461, -1291, 4647, 1963},
        {540, 426, 1291, 4647, 1963},
        {541, 205, 1291, -4647, 1963},
        {542, 333, 4647, 1291, 1963},
        {543, 170, -1291, -4647, 1963},
        {544, 298, 4647, -1291, 1963},
        {545, 550, -4325, 2035, 2286},
        {546, 586, -4325, -2035, 2286},
        {547, 458, -2035, 4325, 2286},
        {548, 166, -2035, -4325, 2286},
        {549, 422, 2035, 4325, 2286},
        {550, 202, 2035, -4325, 2286},
        {551, 330, 4325, 2035, 2286},
        {552, 294, 4325, -2035, 2286},
        {553, 575, -5238, 0, 1373},
        {554, 447, 0, 5238, 1373},
        {555, 191, 0, -5238, 1373},
        {556, 319, 5238, 0, 1373},
        {557, 569, -5243, 552, 1367},
        {558, 185, -552, -5243, 1367},
        {559, 571, -5243, -552, 1367},
        {560, 443, -552, 5243, 1367},
        {561, 187, 552, -5243, 1367},
        {562, 315, 5243, 552, 1367},
        {563, 556, -3804, 3532, 2806},
        {564, 587, -3804, -3532, 2806},
        {565, 459, -3532, 3804, 2806},
        {566, 172, -3532, -3804, 2806},
        {567, 441, 552, 5243, 1367},
        {568, 428, 3532, 3804, 2806},
        {569, 203, 3532, -3804, 2806},
        {570, 331, 3804, 3532, 2806},
        {571, 300, 3804, -3532, 2806},
        {572, 313, 5243, -552, 1367},
        {573, 591, -3960, -3168, 2650}, // THIS IS INCORRECT
        {574, 463, -3168, 3960, 2650},
        {575, 207, 3168, -3960, 2650},
        {576, 335, 3960, 3168, 2650},
        {577, 552, -3960, 3168, 2650},
        {578, 168, -3168, -3960, 2650},
        {579, 424, 3168, 3960, 2650},
        {580, 296, 3960, -3168, 2650}
};

void LedArrayInterface::setMaxCurrentEnforcement(bool enforce)
{
        TLC5955::enforce_max_current = enforce;
}

void LedArrayInterface::setMaxCurrentLimit(float limit)
{
        if (limit > 0)
                TLC5955::max_current_amps = limit;
}

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
        tlc.setPinOrderSingle(led_number, color_channel_index, position);
}

void LedArrayInterface::notImplemented(const char * command_name)
{
        Serial.print(F("Command "));
        Serial.print(command_name);
        Serial.printf(F(" is not implemented for this device.%s"), SERIAL_LINE_ENDING);
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
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
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
                Serial.print(value);
                Serial.print(SERIAL_LINE_ENDING);
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
                Serial.printf(F(")%s"), SERIAL_LINE_ENDING);
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
                Serial.print(value);
                Serial.print(SERIAL_LINE_ENDING);
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
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
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
                Serial.print(color_channel_number);
                Serial.print(SERIAL_LINE_ENDING);
        }
        setLed(led_number, color_channel_number, (uint16_t) (value * UINT16_MAX));
}

void LedArrayInterface::deviceReset()
{
        deviceSetup();
}

uint16_t LedArrayInterface::getSerialNumber()
{
        uint16_t sn_read = (EEPROM.read(SN_ADDRESS + 1) << 8) | EEPROM.read(SN_ADDRESS);
        return (sn_read);
}

void LedArrayInterface::setSerialNumber(uint16_t serial_number)
{
	byte lower_8bits_sn = serial_number & 0xff;
	byte upper_8bits_sn = (serial_number >> 8) & 0xff;
	EEPROM.write(sn_address, lower_8bits_sn);
	EEPROM.write(sn_address + 1, upper_8bits_sn);
}

uint16_t LedArrayInterface::getPartNumber()
{
        uint16_t pn_read = (EEPROM.read(PN_ADDRESS + 1) << 8) | EEPROM.read(PN_ADDRESS);
        return (pn_read);
}

void LedArrayInterface::setPartNumber(uint16_t part_number)
{
	byte lower_8bits_pn = part_number & 0xff;
	byte upper_8bits_pn = (part_number >> 8) & 0xff;
	EEPROM.write(pn_address, lower_8bits_pn);
	EEPROM.write(pn_address + 1, upper_8bits_pn);
}

void LedArrayInterface::deviceSetup()
{
        // Now set the GSCK to an output and a 50% PWM duty-cycle
        // For simplicity all three grayscale clocks are tied to the same pin
        pinMode(GSCLK, OUTPUT);
        pinMode(LAT, OUTPUT);

        // Adjust PWM timer for maximum GSCLK frequency
        analogWriteFrequency(GSCLK, gsclk_frequency);
        analogWriteResolution(1);
        analogWrite(GSCLK, 1);

        // The library does not ininiate SPI for you, so as to prevent issues with other SPI libraries
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

        // Correct LED pins
        tlc.setRgbPinOrderSingle(79, 0, 1, 2); // channel 79 has B/R swapped.
        tlc.setRgbPinOrderSingle(80, 0, 1, 2); // channel 80 has B/R swapped.
        tlc.setRgbPinOrderSingle(82, 0, 1, 2); // channel 82 has B/R swapped.
        tlc.setRgbPinOrderSingle(83, 0, 1, 2); // channel 83 has B/R swapped.
        tlc.setRgbPinOrderSingle(84, 0, 1, 2); // channel 84 has B/R swapped.
        tlc.setRgbPinOrderSingle(85, 0, 1, 2); // channel 85 has B/R swapped.
        tlc.setRgbPinOrderSingle(86, 0, 1, 2); // channel 86 has B/R swapped.
        tlc.setRgbPinOrderSingle(87, 0, 1, 2); // channel 87 has B/R swapped.
        tlc.setRgbPinOrderSingle(88, 0, 1, 2); // channel 88 has B/R swapped.
        tlc.setRgbPinOrderSingle(89, 0, 1, 2); // channel 89 has B/R swapped.
        tlc.setRgbPinOrderSingle(90, 0, 1, 2); // channel 90 has B/R swapped.
        tlc.setRgbPinOrderSingle(91, 0, 1, 2); // channel 91 has B/R swapped.
        tlc.setRgbPinOrderSingle(92, 0, 1, 2); // channel 92 has B/R swapped.
        tlc.setRgbPinOrderSingle(93, 0, 1, 2); // channel 93 has B/R swapped.
        tlc.setRgbPinOrderSingle(94, 0, 1, 2); // channel 94 has B/R swapped.
        tlc.setRgbPinOrderSingle(95, 0, 1, 2); // channel 95 has B/R swapped.
        tlc.setRgbPinOrderSingle(96, 0, 1, 2); // channel 96 has B/R swapped.
        tlc.setRgbPinOrderSingle(97, 0, 1, 2); // channel 97 has B/R swapped.
        tlc.setRgbPinOrderSingle(98, 0, 1, 2); // channel 98 has B/R swapped.
        tlc.setRgbPinOrderSingle(99, 0, 1, 2); // channel 99 has B/R swapped.
        tlc.setRgbPinOrderSingle(100, 0, 1, 2); // channel 100 has B/R swapped.
        tlc.setRgbPinOrderSingle(101, 0, 1, 2); // channel 101 has B/R swapped.
        tlc.setRgbPinOrderSingle(102, 0, 1, 2); // channel 102 has B/R swapped.
        tlc.setRgbPinOrderSingle(103, 0, 1, 2); // channel 103 has B/R swapped.
        tlc.setRgbPinOrderSingle(104, 0, 1, 2); // channel 104 has B/R swapped.
        tlc.setRgbPinOrderSingle(105, 0, 1, 2); // channel 105 has B/R swapped.
        tlc.setRgbPinOrderSingle(106, 0, 1, 2); // channel 106 has B/R swapped.
        tlc.setRgbPinOrderSingle(107, 0, 1, 2); // channel 107 has B/R swapped.
        tlc.setRgbPinOrderSingle(108, 0, 1, 2); // channel 108 has B/R swapped.
        tlc.setRgbPinOrderSingle(109, 0, 1, 2); // channel 109 has B/R swapped.
        tlc.setRgbPinOrderSingle(110, 0, 1, 2); // channel 110 has B/R swapped.
        tlc.setRgbPinOrderSingle(111, 0, 1, 2); // channel 111 has B/R swapped.
        tlc.setRgbPinOrderSingle(116, 0, 1, 2); // channel 116 has B/R swapped.
        tlc.setRgbPinOrderSingle(208, 0, 1, 2); // channel 208 has B/R swapped.
        tlc.setRgbPinOrderSingle(210, 0, 1, 2); // channel 210 has B/R swapped.
        tlc.setRgbPinOrderSingle(211, 0, 1, 2); // channel 211 has B/R swapped.
        tlc.setRgbPinOrderSingle(212, 0, 1, 2); // channel 212 has B/R swapped.
        tlc.setRgbPinOrderSingle(213, 0, 1, 2); // channel 213 has B/R swapped.
        tlc.setRgbPinOrderSingle(214, 0, 1, 2); // channel 214 has B/R swapped.
        tlc.setRgbPinOrderSingle(215, 0, 1, 2); // channel 215 has B/R swapped.
        tlc.setRgbPinOrderSingle(216, 0, 1, 2); // channel 216 has B/R swapped.
        tlc.setRgbPinOrderSingle(217, 0, 1, 2); // channel 217 has B/R swapped.
        tlc.setRgbPinOrderSingle(218, 0, 1, 2); // channel 218 has B/R swapped.
        tlc.setRgbPinOrderSingle(219, 0, 1, 2); // channel 219 has B/R swapped.
        tlc.setRgbPinOrderSingle(220, 0, 1, 2); // channel 220 has B/R swapped.
        tlc.setRgbPinOrderSingle(221, 0, 1, 2); // channel 221 has B/R swapped.
        tlc.setRgbPinOrderSingle(222, 0, 1, 2); // channel 222 has B/R swapped.
        tlc.setRgbPinOrderSingle(223, 0, 1, 2); // channel 223 has B/R swapped.
        tlc.setRgbPinOrderSingle(224, 0, 1, 2); // channel 224 has B/R swapped.
        tlc.setRgbPinOrderSingle(225, 0, 1, 2); // channel 225 has B/R swapped.
        tlc.setRgbPinOrderSingle(226, 0, 1, 2); // channel 226 has B/R swapped.
        tlc.setRgbPinOrderSingle(227, 0, 1, 2); // channel 227 has B/R swapped.
        tlc.setRgbPinOrderSingle(228, 0, 1, 2); // channel 228 has B/R swapped.
        tlc.setRgbPinOrderSingle(229, 0, 1, 2); // channel 229 has B/R swapped.
        tlc.setRgbPinOrderSingle(230, 0, 1, 2); // channel 230 has B/R swapped.
        tlc.setRgbPinOrderSingle(231, 0, 1, 2); // channel 231 has B/R swapped.
        tlc.setRgbPinOrderSingle(232, 0, 1, 2); // channel 232 has B/R swapped.
        tlc.setRgbPinOrderSingle(233, 0, 1, 2); // channel 233 has B/R swapped.
        tlc.setRgbPinOrderSingle(234, 0, 1, 2); // channel 234 has B/R swapped.
        tlc.setRgbPinOrderSingle(235, 0, 1, 2); // channel 235 has B/R swapped.
        tlc.setRgbPinOrderSingle(236, 0, 1, 2); // channel 236 has B/R swapped.
        tlc.setRgbPinOrderSingle(237, 0, 1, 2); // channel 237 has B/R swapped.
        tlc.setRgbPinOrderSingle(238, 0, 1, 2); // channel 238 has B/R swapped.
        tlc.setRgbPinOrderSingle(239, 0, 1, 2); // channel 239 has B/R swapped.
        tlc.setRgbPinOrderSingle(244, 0, 1, 2); // channel 244 has B/R swapped.
        tlc.setRgbPinOrderSingle(336, 0, 1, 2); // channel 336 has B/R swapped.
        tlc.setRgbPinOrderSingle(338, 0, 1, 2); // channel 338 has B/R swapped.
        tlc.setRgbPinOrderSingle(339, 0, 1, 2); // channel 339 has B/R swapped.
        tlc.setRgbPinOrderSingle(340, 0, 1, 2); // channel 340 has B/R swapped.
        tlc.setRgbPinOrderSingle(341, 0, 1, 2); // channel 341 has B/R swapped.
        tlc.setRgbPinOrderSingle(342, 0, 1, 2); // channel 342 has B/R swapped.
        tlc.setRgbPinOrderSingle(343, 0, 1, 2); // channel 343 has B/R swapped.
        tlc.setRgbPinOrderSingle(344, 0, 1, 2); // channel 344 has B/R swapped.
        tlc.setRgbPinOrderSingle(345, 0, 1, 2); // channel 345 has B/R swapped.
        tlc.setRgbPinOrderSingle(346, 0, 1, 2); // channel 346 has B/R swapped.
        tlc.setRgbPinOrderSingle(347, 0, 1, 2); // channel 347 has B/R swapped.
        tlc.setRgbPinOrderSingle(348, 0, 1, 2); // channel 348 has B/R swapped.
        tlc.setRgbPinOrderSingle(349, 0, 1, 2); // channel 349 has B/R swapped.
        tlc.setRgbPinOrderSingle(350, 0, 1, 2); // channel 350 has B/R swapped.
        tlc.setRgbPinOrderSingle(351, 0, 1, 2); // channel 351 has B/R swapped.
        tlc.setRgbPinOrderSingle(352, 0, 1, 2); // channel 352 has B/R swapped.
        tlc.setRgbPinOrderSingle(353, 0, 1, 2); // channel 353 has B/R swapped.
        tlc.setRgbPinOrderSingle(354, 0, 1, 2); // channel 354 has B/R swapped.
        tlc.setRgbPinOrderSingle(355, 0, 1, 2); // channel 355 has B/R swapped.
        tlc.setRgbPinOrderSingle(356, 0, 1, 2); // channel 356 has B/R swapped.
        tlc.setRgbPinOrderSingle(357, 0, 1, 2); // channel 357 has B/R swapped.
        tlc.setRgbPinOrderSingle(358, 0, 1, 2); // channel 358 has B/R swapped.
        tlc.setRgbPinOrderSingle(359, 0, 1, 2); // channel 359 has B/R swapped.
        tlc.setRgbPinOrderSingle(360, 0, 1, 2); // channel 360 has B/R swapped.
        tlc.setRgbPinOrderSingle(361, 0, 1, 2); // channel 361 has B/R swapped.
        tlc.setRgbPinOrderSingle(362, 0, 1, 2); // channel 362 has B/R swapped.
        tlc.setRgbPinOrderSingle(363, 0, 1, 2); // channel 363 has B/R swapped.
        tlc.setRgbPinOrderSingle(364, 0, 1, 2); // channel 364 has B/R swapped.
        tlc.setRgbPinOrderSingle(365, 0, 1, 2); // channel 365 has B/R swapped.
        tlc.setRgbPinOrderSingle(366, 0, 1, 2); // channel 366 has B/R swapped.
        tlc.setRgbPinOrderSingle(367, 0, 1, 2); // channel 367 has B/R swapped.
        tlc.setRgbPinOrderSingle(372, 0, 1, 2); // channel 372 has B/R swapped.
        tlc.setRgbPinOrderSingle(464, 0, 1, 2); // channel 464 has B/R swapped.
        tlc.setRgbPinOrderSingle(466, 0, 1, 2); // channel 466 has B/R swapped.
        tlc.setRgbPinOrderSingle(467, 0, 1, 2); // channel 467 has B/R swapped.
        tlc.setRgbPinOrderSingle(468, 0, 1, 2); // channel 468 has B/R swapped.
        tlc.setRgbPinOrderSingle(469, 0, 1, 2); // channel 469 has B/R swapped.
        tlc.setRgbPinOrderSingle(470, 0, 1, 2); // channel 470 has B/R swapped.
        tlc.setRgbPinOrderSingle(471, 0, 1, 2); // channel 471 has B/R swapped.
        tlc.setRgbPinOrderSingle(472, 0, 1, 2); // channel 472 has B/R swapped.
        tlc.setRgbPinOrderSingle(473, 0, 1, 2); // channel 473 has B/R swapped.
        tlc.setRgbPinOrderSingle(474, 0, 1, 2); // channel 474 has B/R swapped.
        tlc.setRgbPinOrderSingle(475, 0, 1, 2); // channel 475 has B/R swapped.
        tlc.setRgbPinOrderSingle(476, 0, 1, 2); // channel 476 has B/R swapped.
        tlc.setRgbPinOrderSingle(477, 0, 1, 2); // channel 477 has B/R swapped.
        tlc.setRgbPinOrderSingle(478, 0, 1, 2); // channel 478 has B/R swapped.
        tlc.setRgbPinOrderSingle(479, 0, 1, 2); // channel 479 has B/R swapped.
        tlc.setRgbPinOrderSingle(480, 0, 1, 2); // channel 480 has B/R swapped.
        tlc.setRgbPinOrderSingle(481, 0, 1, 2); // channel 481 has B/R swapped.
        tlc.setRgbPinOrderSingle(482, 0, 1, 2); // channel 482 has B/R swapped.
        tlc.setRgbPinOrderSingle(483, 0, 1, 2); // channel 483 has B/R swapped.
        tlc.setRgbPinOrderSingle(484, 0, 1, 2); // channel 484 has B/R swapped.
        tlc.setRgbPinOrderSingle(485, 0, 1, 2); // channel 485 has B/R swapped.
        tlc.setRgbPinOrderSingle(486, 0, 1, 2); // channel 486 has B/R swapped.
        tlc.setRgbPinOrderSingle(487, 0, 1, 2); // channel 487 has B/R swapped.
        tlc.setRgbPinOrderSingle(488, 0, 1, 2); // channel 488 has B/R swapped.
        tlc.setRgbPinOrderSingle(489, 0, 1, 2); // channel 489 has B/R swapped.
        tlc.setRgbPinOrderSingle(490, 0, 1, 2); // channel 490 has B/R swapped.
        tlc.setRgbPinOrderSingle(491, 0, 1, 2); // channel 491 has B/R swapped.
        tlc.setRgbPinOrderSingle(492, 0, 1, 2); // channel 492 has B/R swapped.
        tlc.setRgbPinOrderSingle(493, 0, 1, 2); // channel 493 has B/R swapped.
        tlc.setRgbPinOrderSingle(494, 0, 1, 2); // channel 494 has B/R swapped.
        tlc.setRgbPinOrderSingle(495, 0, 1, 2); // channel 495 has B/R swapped.
        tlc.setRgbPinOrderSingle(500, 0, 1, 2); // channel 500 has B/R swapped.
        tlc.setRgbPinOrderSingle(585, 0, 1, 2); // channel 79 has B/R swapped.
}

uint8_t LedArrayInterface::getDeviceCommandCount()
{
        return (LedArrayInterface::device_command_count);
}

const char * LedArrayInterface::getDeviceCommandNameShort(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::deviceCommandNamesShort[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

const char * LedArrayInterface::getDeviceCommandNameLong(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
                return (LedArrayInterface::deviceCommandNamesLong[device_command_index]);
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return ("");
        }
}

uint32_t LedArrayInterface::getDeviceCommandLedListSize(int device_command_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
        {
                // Get stored pattern cound and led per pattern for this command
                uint16_t pattern_count = LedArrayInterface::device_command_pattern_dimensions[device_command_index][0];
                uint16_t leds_per_pattern = LedArrayInterface::device_command_pattern_dimensions[device_command_index][1];

                // Concatenate these two into 32-bit unsigned integer
                uint32_t concatenated = ((uint32_t)pattern_count) << 16 | leds_per_pattern;
                return (concatenated);
        }
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

uint16_t LedArrayInterface::getDeviceCommandLedListElement(int device_command_index, uint16_t pattern_index, uint16_t led_index)
{
        if ((device_command_index >= 0) && (device_command_index < LedArrayInterface::device_command_count))
        {
                uint32_t concatenated = getDeviceCommandLedListSize(device_command_index);
                uint16_t pattern_count  = (uint16_t)(concatenated >> 16);
                uint16_t leds_per_pattern = (uint16_t)concatenated;

                if ((pattern_index < pattern_count) && (led_index < leds_per_pattern))
                {
                        //      if (device_command_index == 0)
                        //        return (uint16_t)pgm_read_word(&(hole_led_list[pattern_index][led_index]));
                        //      else if (device_command_index == 1)
                        //        return (uint16_t)pgm_read_word(&(uv_led_list[pattern_index][led_index]));
                        //      else
                        return 0;

                }
                else
                {
                        Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid pattern index (%d) / led index (%d)"), pattern_index, led_index, SERIAL_LINE_ENDING);
                        return (0);
                }
        }
        else
        {
                Serial.printf(F("ERROR (LedArrayInterface::getDeviceCommandLedListSize): Invalid device command index (%d)"), device_command_index, SERIAL_LINE_ENDING);
                return (0);
        }
}

#endif


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
 #ifdef USE_SCI_EPI_ARRAY
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
const int TRIGGER_OUTPUT_COUNT = 1;
const int TRIGGER_INPUT_COUNT = 1;

// EEPROM Addresses
#define DEMO_MODE_ADDRESS 50
#define PN_ADDRESS 100
#define SN_ADDRESS 200

// LED pin swap
const bool LED_SWAP_GROUP_1 = true;

// Device and Software Descriptors
const char * LedArrayInterface::device_name = "SCI Epi-Dome (c-005)";
const char * LedArrayInterface::device_hardware_revision = "1.0";
const float LedArrayInterface::max_na = 0.8;
const float LedArrayInterface::default_na = 0.4;
const int16_t LedArrayInterface::led_count = 720;
const uint16_t LedArrayInterface::center_led = 0;
const int LedArrayInterface::trigger_output_count = 1;
const int LedArrayInterface::trigger_input_count = 1;
const int LedArrayInterface::color_channel_count = 3;
const char LedArrayInterface::color_channel_names[] = {'r', 'g', 'b'};
const float LedArrayInterface::color_channel_center_wavelengths[] = {0.48, 0.525, 0.625};
const int LedArrayInterface::bit_depth = 16;
const bool LedArrayInterface::supports_fast_sequence = false;
const float LedArrayInterface::led_array_distance_z_default = 50.0;
float LedArrayInterface::led_position_list_na[LedArrayInterface::led_count][2];
int LedArrayInterface::debug = 0;

const int LedArrayInterface::trigger_output_pin_list[] = {TRIGGER_OUTPUT_PIN_0};
const int LedArrayInterface::trigger_input_pin_list[] = {TRIGGER_INPUT_PIN_0};
bool LedArrayInterface::trigger_input_state[] = {false};

const uint8_t TLC5955::_tlc_count = 50;    // Change to reflect number of TLC chips
float TLC5955::max_current_amps = 8.0;      // Maximum current output, amps
bool TLC5955::enforce_max_current = true;   // Whether to enforce max current limit

// Define dot correction, pin rgb order, and grayscale data arrays in program memory
uint8_t TLC5955::_dc_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint8_t TLC5955::_rgb_order[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];
uint16_t TLC5955::_grayscale_data[TLC5955::_tlc_count][TLC5955::LEDS_PER_CHIP][TLC5955::COLOR_CHANNEL_COUNT];

/**** Device-specific variables ****/
TLC5955 tlc;                            // TLC5955 object
uint32_t gsclk_frequency = 4000000;     // Grayscale clock speed
uint32_t spi_baud_rate   = 2500000;

/**** Device-specific commands ****/
const uint8_t LedArrayInterface::device_command_count = 1;
const char * LedArrayInterface::deviceCommandNamesShort[] = {"h"};
const char * LedArrayInterface::deviceCommandNamesLong[] = {"hole"};
const uint16_t LedArrayInterface::device_command_pattern_dimensions[][2] = {{1,20}}; // Number of commands, number of LEDs in each command.

/**** Part number and Serial number addresses in EEPROM ****/
uint16_t pn_address = 100;
uint16_t sn_address = 200;

PROGMEM const int16_t center_led_list[1][20] = {
  {0, 1, 2, 3, 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19}
};

PROGMEM const int16_t LedArrayInterface::led_positions[LedArrayInterface::led_count][5] = {
    {0, 26, 1046, -1440, 6500},
    {1, 11, -1693, -550, 6500},
    {2, 10, -1440, -1046, 6500},
    {3, 21, -1046, -1440, 6500},
    {4, 18, -550, -1693, 6500},
    {5, 58, -1046, 1440, 6500},
    {6, 27, 550, -1693, 6500},
    {7, 5, -1440, 1046, 6500},
    {8, 2, -1693, 550, 6500},
    {9, 34, 1693, -550, 6500},
    {10, 59, -550, 1693, 6500},
    {11, 43, 1693, 550, 6500},
    {12, 42, 1440, 1046, 6500},
    {13, 53, 1046, 1440, 6500},
    {14, 50, 550, 1693, 6500},
    {15, 37, 1440, -1046, 6500},
    {16, 55, 0, 1780, 6500},
    {17, 7, -1780, 0, 6500},
    {18, 23, 0, -1780, 6500},
    {19, 39, 1780, 0, 6500},
    {20, 35, 2100, 0, 6500},
    {21, 19, 0, -2100, 6500},
    {22, 3, -2100, 0, 6500},
    {23, 51, 0, 2100, 6500},
    {24, 114, 1500, 1500, 6500},
    {25, 98, 1500, -1500, 6500},
    {26, 82, -1500, 1500, 6500},
    {27, 66, -1500, -1500, 6500},
    {28, 63, -500, 2100, 6500},
    {29, 49, 500, 2100, 6500},
    {30, 15, -2100, -500, 6500},
    {31, 1, -2100, 500, 6500},
    {32, 47, 2100, 500, 6500},
    {33, 17, -500, -2100, 6500},
    {34, 31, 500, -2100, 6500},
    {35, 33, 2100, -500, 6500},
    {36, 29, 1000, -2000, 6500},
    {37, 61, -1000, 2000, 6500},
    {38, 0, -2000, 1000, 6500},
    {39, 16, -1000, -2000, 6500},
    {40, 45, 2000, 1000, 6500},
    {41, 32, 2000, -1000, 6500},
    {42, 48, 1000, 2000, 6500},
    {43, 13, -2000, -1000, 6500},
    {44, 6, -2500, 0, 6500},
    {45, 71, -1500, -2000, 6500},
    {46, 81, -1500, 2000, 6500},
    {47, 97, 1500, -2000, 6500},
    {48, 119, 1500, 2000, 6500},
    {49, 87, -2000, 1500, 6500},
    {50, 54, 0, 2500, 6500},
    {51, 113, 2000, 1500, 6500},
    {52, 22, 0, -2500, 6500},
    {53, 103, 2000, -1500, 6500},
    {54, 38, 2500, 0, 6500},
    {55, 65, -2000, -1500, 6500},
    {56, 52, 500, 2500, 6500},
    {57, 14, -2500, -500, 6500},
    {58, 36, 2500, -500, 6500},
    {59, 30, 500, -2500, 6500},
    {60, 46, 2500, 500, 6500},
    {61, 4, -2500, 500, 6500},
    {62, 62, -500, 2500, 6500},
    {63, 20, -500, -2500, 6500},
    {64, 96, 1000, -2500, 6500},
    {65, 75, -1000, -2500, 6500},
    {66, 80, -1000, 2500, 6500},
    {67, 91, -2500, 1000, 6500},
    {68, 112, 2500, 1000, 6500},
    {69, 123, 1000, 2500, 6500},
    {70, 107, 2500, -1000, 6500},
    {71, 64, -2500, -1000, 6500},
    {72, 115, 2000, 2000, 6500},
    {73, 83, -2000, 2000, 6500},
    {74, 67, -2000, -2000, 6500},
    {75, 99, 2000, -2000, 6500},
    {76, 79, -1500, -2500, 6500},
    {77, 85, -1500, 2500, 6500},
    {78, 101, 1500, -2500, 6500},
    {79, 127, 1500, 2500, 6500},
    {80, 117, 2500, 1500, 6500},
    {81, 95, -2500, 1500, 6500},
    {82, 69, -2500, -1500, 6500},
    {83, 111, 2500, -1500, 6500},
    {84, 40, 3000, 0, 6500},
    {85, 24, 0, -3000, 6500},
    {86, 56, 0, 3000, 6500},
    {87, 8, -3000, 0, 6500},
    {88, 41, 3000, 500, 6500},
    {89, 60, 500, 3000, 6500},
    {90, 9, -3000, -500, 6500},
    {91, 25, 500, -3000, 6500},
    {92, 44, 3000, -500, 6500},
    {93, 28, -500, -3000, 6500},
    {94, 57, -500, 3000, 6500},
    {95, 12, -3000, 500, 6500},
    {96, 94, -3000, 1000, 6500},
    {97, 100, 1000, -3000, 6500},
    {98, 68, -3000, -1000, 6500},
    {99, 126, 1000, 3000, 6500},
    {100, 84, -1000, 3000, 6500},
    {101, 116, 3000, 1000, 6500},
    {102, 110, 3000, -1000, 6500},
    {103, 78, -1000, -3000, 6500},
    {104, 118, 2500, 2000, 6500},
    {105, 122, 2000, 2500, 6500},
    {106, 70, -2500, -2000, 6500},
    {107, 102, 2000, -2500, 6500},
    {108, 90, -2500, 2000, 6500},
    {109, 86, -2000, 2500, 6500},
    {110, 74, -2000, -2500, 6500},
    {111, 106, 2500, -2000, 6500},
    {112, 236, -250, 3129, 6242},
    {113, 556, 250, -3129, 6242},
    {114, 396, 3129, 250, 6242},
    {115, 393, 3129, -250, 6242},
    {116, 713, -3129, 250, 6242},
    {117, 716, -3129, -250, 6242},
    {118, 233, 250, 3129, 6242},
    {119, 553, -250, -3129, 6242},
    {120, 558, -750, -3129, 6242},
    {121, 383, 3129, 750, 6242},
    {122, 703, -3129, -750, 6242},
    {123, 718, -3129, 750, 6242},
    {124, 238, 750, 3129, 6242},
    {125, 543, 750, -3129, 6242},
    {126, 223, -750, 3129, 6242},
    {127, 398, 3129, -750, 6242},
    {128, 135, 1500, 3000, 6500},
    {129, 141, 3000, -1500, 6500},
    {130, 104, 1500, -3000, 6500},
    {131, 88, -1500, 3000, 6500},
    {132, 128, -3000, 1500, 6500},
    {133, 120, 3000, 1500, 6500},
    {134, 72, -3000, -1500, 6500},
    {135, 143, -1500, -3000, 6500},
    {136, 234, 1250, 3129, 6242},
    {137, 218, -1250, 3129, 6242},
    {138, 698, -3129, -1250, 6242},
    {139, 538, 1250, -3129, 6242},
    {140, 394, 3129, -1250, 6242},
    {141, 714, -3129, 1250, 6242},
    {142, 378, 3129, 1250, 6242},
    {143, 554, -1250, -3129, 6242},
    {144, 93, -2500, 2500, 6500},
    {145, 109, 2500, -2500, 6500},
    {146, 125, 2500, 2500, 6500},
    {147, 77, -2500, -2500, 6500},
    {148, 137, 3000, -2000, 6500},
    {149, 139, 2000, 3000, 6500},
    {150, 108, 2000, -3000, 6500},
    {151, 131, -2000, -3000, 6500},
    {152, 92, -2000, 3000, 6500},
    {153, 124, 3000, 2000, 6500},
    {154, 133, -3000, 2000, 6500},
    {155, 76, -3000, -2000, 6500},
    {156, 548, 0, -3379, 5888},
    {157, 388, 3379, 0, 5888},
    {158, 708, -3379, 0, 5888},
    {159, 228, 0, 3379, 5888},
    {160, 222, -1750, 3129, 6242},
    {161, 382, 3129, 1750, 6242},
    {162, 542, 1750, -3129, 6242},
    {163, 702, -3129, -1750, 6242},
    {164, 239, 1750, 3129, 6242},
    {165, 559, -1750, -3129, 6242},
    {166, 399, 3129, -1750, 6242},
    {167, 719, -3129, 1750, 6242},
    {168, 552, 500, -3379, 5888},
    {169, 387, 3379, -500, 5888},
    {170, 232, -500, 3379, 5888},
    {171, 712, -3379, -500, 5888},
    {172, 392, 3379, 500, 5888},
    {173, 227, 500, 3379, 5888},
    {174, 707, -3379, 500, 5888},
    {175, 547, -500, -3379, 5888},
    {176, 539, 1000, -3379, 5888},
    {177, 391, 3379, -1000, 5888},
    {178, 379, 3379, 1000, 5888},
    {179, 231, 1000, 3379, 5888},
    {180, 711, -3379, 1000, 5888},
    {181, 551, -1000, -3379, 5888},
    {182, 219, -1000, 3379, 5888},
    {183, 699, -3379, -1000, 5888},
    {184, 129, -3000, 2500, 6500},
    {185, 134, -2500, -3000, 6500},
    {186, 121, 3000, 2500, 6500},
    {187, 140, 3000, -2500, 6500},
    {188, 89, -2500, 3000, 6500},
    {189, 138, 2500, 3000, 6500},
    {190, 73, -3000, -2500, 6500},
    {191, 105, 2500, -3000, 6500},
    {192, 620, -2250, -3129, 6242},
    {193, 377, 3129, 2250, 6242},
    {194, 460, 3129, -2250, 6242},
    {195, 300, 2250, 3129, 6242},
    {196, 780, -3129, 2250, 6242},
    {197, 217, -2250, 3129, 6242},
    {198, 697, -3129, -2250, 6242},
    {199, 537, 2250, -3129, 6242},
    {200, 535, 1500, -3379, 5888},
    {201, 555, -1500, -3379, 5888},
    {202, 715, -3379, 1500, 5888},
    {203, 695, -3379, -1500, 5888},
    {204, 235, 1500, 3379, 5888},
    {205, 375, 3379, 1500, 5888},
    {206, 215, -1500, 3379, 5888},
    {207, 395, 3379, -1500, 5888},
    {208, 136, 3000, -3000, 6500},
    {209, 132, -3000, -3000, 6500},
    {210, 142, 3000, 3000, 6500},
    {211, 130, -3000, 3000, 6500},
    {212, 705, -3629, 250, 5535},
    {213, 389, 3629, 250, 5535},
    {214, 549, 250, -3629, 5535},
    {215, 385, 3629, -250, 5535},
    {216, 225, 250, 3629, 5535},
    {217, 229, -250, 3629, 5535},
    {218, 709, -3629, -250, 5535},
    {219, 545, -250, -3629, 5535},
    {220, 776, -3379, 2000, 5888},
    {221, 616, -2000, -3379, 5888},
    {222, 296, 2000, 3379, 5888},
    {223, 456, 3379, -2000, 5888},
    {224, 371, 3379, 2000, 5888},
    {225, 691, -3379, -2000, 5888},
    {226, 211, -2000, 3379, 5888},
    {227, 531, 2000, -3379, 5888},
    {228, 297, 2750, 3129, 6242},
    {229, 700, -3129, -2750, 6242},
    {230, 457, 3129, -2750, 6242},
    {231, 380, 3129, 2750, 6242},
    {232, 617, -2750, -3129, 6242},
    {233, 777, -3129, 2750, 6242},
    {234, 220, -2750, 3129, 6242},
    {235, 540, 2750, -3129, 6242},
    {236, 544, 750, -3629, 5535},
    {237, 224, -750, 3629, 5535},
    {238, 384, 3629, 750, 5535},
    {239, 704, -3629, -750, 5535},
    {240, 386, 3629, -750, 5535},
    {241, 706, -3629, 750, 5535},
    {242, 546, -750, -3629, 5535},
    {243, 226, 750, 3629, 5535},
    {244, 390, 3629, -1250, 5535},
    {245, 534, 1250, -3629, 5535},
    {246, 230, 1250, 3629, 5535},
    {247, 374, 3629, 1250, 5535},
    {248, 710, -3629, 1250, 5535},
    {249, 694, -3629, -1250, 5535},
    {250, 214, -1250, 3629, 5535},
    {251, 550, -1250, -3629, 5535},
    {252, 292, 2500, 3379, 5888},
    {253, 532, 2500, -3379, 5888},
    {254, 452, 3379, -2500, 5888},
    {255, 692, -3379, -2500, 5888},
    {256, 612, -2500, -3379, 5888},
    {257, 372, 3379, 2500, 5888},
    {258, 212, -2500, 3379, 5888},
    {259, 772, -3379, 2500, 5888},
    {260, 530, 1750, -3629, 5535},
    {261, 690, -3629, -1750, 5535},
    {262, 370, 3629, 1750, 5535},
    {263, 448, 3629, -1750, 5535},
    {264, 768, -3629, 1750, 5535},
    {265, 608, -1750, -3629, 5535},
    {266, 288, 1750, 3629, 5535},
    {267, 210, -1750, 3629, 5535},
    {268, 206, 0, 3880, 5181},
    {269, 686, -3880, 0, 5181},
    {270, 366, 3880, 0, 5181},
    {271, 526, 0, -3880, 5181},
    {272, 681, -3880, -500, 5181},
    {273, 362, 3880, -500, 5181},
    {274, 361, 3880, 500, 5181},
    {275, 521, 500, -3880, 5181},
    {276, 522, -500, -3880, 5181},
    {277, 202, 500, 3880, 5181},
    {278, 201, -500, 3880, 5181},
    {279, 682, -3880, 500, 5181},
    {280, 451, 3379, -3000, 5888},
    {281, 291, 3000, 3379, 5888},
    {282, 216, -3000, 3379, 5888},
    {283, 376, 3379, 3000, 5888},
    {284, 536, 3000, -3379, 5888},
    {285, 696, -3379, -3000, 5888},
    {286, 611, -3000, -3379, 5888},
    {287, 771, -3379, 3000, 5888},
    {288, 293, 2250, 3629, 5535},
    {289, 613, -2250, -3629, 5535},
    {290, 773, -3629, 2250, 5535},
    {291, 453, 3629, -2250, 5535},
    {292, 689, -3629, -2250, 5535},
    {293, 209, -2250, 3629, 5535},
    {294, 369, 3629, 2250, 5535},
    {295, 529, 2250, -3629, 5535},
    {296, 364, 3880, 1000, 5181},
    {297, 204, -1000, 3880, 5181},
    {298, 684, -3880, -1000, 5181},
    {299, 524, 1000, -3880, 5181},
    {300, 207, 1000, 3880, 5181},
    {301, 527, -1000, -3880, 5181},
    {302, 367, 3880, -1000, 5181},
    {303, 687, -3880, 1000, 5181},
    {304, 511, 1500, -3880, 5181},
    {305, 268, 1500, 3880, 5181},
    {306, 351, 3880, 1500, 5181},
    {307, 748, -3880, 1500, 5181},
    {308, 671, -3880, -1500, 5181},
    {309, 588, -1500, -3880, 5181},
    {310, 428, 3880, -1500, 5181},
    {311, 191, -1500, 3880, 5181},
    {312, 533, 2750, -3629, 5535},
    {313, 693, -3629, -2750, 5535},
    {314, 373, 3629, 2750, 5535},
    {315, 449, 3629, -2750, 5535},
    {316, 769, -3629, 2750, 5535},
    {317, 609, -2750, -3629, 5535},
    {318, 289, 2750, 3629, 5535},
    {319, 213, -2750, 3629, 5535},
    {320, 506, 2000, -3880, 5181},
    {321, 666, -3880, -2000, 5181},
    {322, 265, 2000, 3880, 5181},
    {323, 346, 3880, 2000, 5181},
    {324, 585, -2000, -3880, 5181},
    {325, 745, -3880, 2000, 5181},
    {326, 425, 3880, -2000, 5181},
    {327, 186, -2000, 3880, 5181},
    {328, 679, -4130, 250, 4828},
    {329, 359, 4130, -250, 4828},
    {330, 519, -250, -4130, 4828},
    {331, 355, 4130, 250, 4828},
    {332, 515, 250, -4130, 4828},
    {333, 675, -4130, -250, 4828},
    {334, 199, 250, 4130, 4828},
    {335, 195, -250, 4130, 4828},
    {336, 356, 4130, 750, 4828},
    {337, 516, 750, -4130, 4828},
    {338, 683, -4130, 750, 4828},
    {339, 676, -4130, -750, 4828},
    {340, 523, -750, -4130, 4828},
    {341, 196, -750, 4130, 4828},
    {342, 363, 4130, -750, 4828},
    {343, 203, 750, 4130, 4828},
    {344, 368, 3629, 3250, 5535},
    {345, 688, -3629, -3250, 5535},
    {346, 208, -3250, 3629, 5535},
    {347, 770, -3629, 3250, 5535},
    {348, 528, 3250, -3629, 5535},
    {349, 610, -3250, -3629, 5535},
    {350, 290, 3250, 3629, 5535},
    {351, 450, 3629, -3250, 5535},
    {352, 750, -3880, 2500, 5181},
    {353, 590, -2500, -3880, 5181},
    {354, 430, 3880, -2500, 5181},
    {355, 270, 2500, 3880, 5181},
    {356, 190, -2500, 3880, 5181},
    {357, 670, -3880, -2500, 5181},
    {358, 350, 3880, 2500, 5181},
    {359, 510, 2500, -3880, 5181},
    {360, 360, 4130, 1250, 4828},
    {361, 744, -4130, 1250, 4828},
    {362, 520, 1250, -4130, 4828},
    {363, 200, -1250, 4130, 4828},
    {364, 680, -4130, -1250, 4828},
    {365, 264, 1250, 4130, 4828},
    {366, 584, -1250, -4130, 4828},
    {367, 424, 4130, -1250, 4828},
    {368, 187, -1750, 4130, 4828},
    {369, 347, 4130, 1750, 4828},
    {370, 507, 1750, -4130, 4828},
    {371, 667, -4130, -1750, 4828},
    {372, 420, 4130, -1750, 4828},
    {373, 260, 1750, 4130, 4828},
    {374, 580, -1750, -4130, 4828},
    {375, 740, -4130, 1750, 4828},
    {376, 426, 3880, -3000, 5181},
    {377, 586, -3000, -3880, 5181},
    {378, 746, -3880, 3000, 5181},
    {379, 266, 3000, 3880, 5181},
    {380, 185, -3000, 3880, 5181},
    {381, 665, -3880, -3000, 5181},
    {382, 345, 3880, 3000, 5181},
    {383, 505, 3000, -3880, 5181},
    {384, 739, -4130, 2250, 4828},
    {385, 419, 4130, -2250, 4828},
    {386, 503, 2250, -4130, 4828},
    {387, 663, -4130, -2250, 4828},
    {388, 259, 2250, 4130, 4828},
    {389, 579, -2250, -4130, 4828},
    {390, 183, -2250, 4130, 4828},
    {391, 343, 4130, 2250, 4828},
    {392, 674, -4380, 0, 4474},
    {393, 514, 0, -4380, 4474},
    {394, 194, 0, 4380, 4474},
    {395, 354, 4380, 0, 4474},
    {396, 673, -4380, -500, 4474},
    {397, 513, 500, -4380, 4474},
    {398, 353, 4380, 500, 4474},
    {399, 198, 500, 4380, 4474},
    {400, 193, -500, 4380, 4474},
    {401, 678, -4380, 500, 4474},
    {402, 518, -500, -4380, 4474},
    {403, 358, 4380, -500, 4474},
    {404, 357, 4380, 1000, 4474},
    {405, 197, -1000, 4380, 4474},
    {406, 677, -4380, -1000, 4474},
    {407, 517, 1000, -4380, 4474},
    {408, 576, -1000, -4380, 4474},
    {409, 416, 4380, -1000, 4474},
    {410, 256, 1000, 4380, 4474},
    {411, 736, -4380, 1000, 4474},
    {412, 591, -3500, -3880, 5181},
    {413, 668, -3880, -3500, 5181},
    {414, 271, 3500, 3880, 5181},
    {415, 431, 3880, -3500, 5181},
    {416, 751, -3880, 3500, 5181},
    {417, 188, -3500, 3880, 5181},
    {418, 348, 3880, 3500, 5181},
    {419, 508, 3500, -3880, 5181},
    {420, 743, -4130, 2750, 4828},
    {421, 499, 2750, -4130, 4828},
    {422, 339, 4130, 2750, 4828},
    {423, 423, 4130, -2750, 4828},
    {424, 179, -2750, 4130, 4828},
    {425, 659, -4130, -2750, 4828},
    {426, 583, -2750, -4130, 4828},
    {427, 263, 2750, 4130, 4828},
    {428, 352, 4380, 1500, 4474},
    {429, 192, -1500, 4380, 4474},
    {430, 581, -1500, -4380, 4474},
    {431, 672, -4380, -1500, 4474},
    {432, 741, -4380, 1500, 4474},
    {433, 261, 1500, 4380, 4474},
    {434, 512, 1500, -4380, 4474},
    {435, 421, 4380, -1500, 4474},
    {436, 737, -4380, 2000, 4474},
    {437, 502, 2000, -4380, 4474},
    {438, 182, -2000, 4380, 4474},
    {439, 662, -4380, -2000, 4474},
    {440, 417, 4380, -2000, 4474},
    {441, 342, 4380, 2000, 4474},
    {442, 257, 2000, 4380, 4474},
    {443, 577, -2000, -4380, 4474},
    {444, 747, -4130, 3250, 4828},
    {445, 660, -4130, -3250, 4828},
    {446, 500, 3250, -4130, 4828},
    {447, 427, 4130, -3250, 4828},
    {448, 587, -3250, -4130, 4828},
    {449, 340, 4130, 3250, 4828},
    {450, 180, -3250, 4130, 4828},
    {451, 267, 3250, 4130, 4828},
    {452, 490, 250, -4630, 4121},
    {453, 330, 4630, 250, 4121},
    {454, 170, -250, 4630, 4121},
    {455, 650, -4630, -250, 4121},
    {456, 495, -250, -4630, 4121},
    {457, 335, 4630, -250, 4121},
    {458, 175, 250, 4630, 4121},
    {459, 655, -4630, 250, 4121},
    {460, 418, 4380, -2500, 4474},
    {461, 178, -2500, 4380, 4474},
    {462, 258, 2500, 4380, 4474},
    {463, 498, 2500, -4380, 4474},
    {464, 658, -4380, -2500, 4474},
    {465, 738, -4380, 2500, 4474},
    {466, 338, 4380, 2500, 4474},
    {467, 578, -2500, -4380, 4474},
    {468, 494, 750, -4630, 4121},
    {469, 654, -4630, -750, 4121},
    {470, 334, 4630, 750, 4121},
    {471, 412, 4630, -750, 4121},
    {472, 572, -750, -4630, 4121},
    {473, 252, 750, 4630, 4121},
    {474, 732, -4630, 750, 4121},
    {475, 174, -750, 4630, 4121},
    {476, 344, 4130, 3750, 4828},
    {477, 184, -3750, 4130, 4828},
    {478, 504, 3750, -4130, 4828},
    {479, 281, 3750, 4130, 4828},
    {480, 761, -4130, 3750, 4828},
    {481, 664, -4130, -3750, 4828},
    {482, 441, 4130, -3750, 4828},
    {483, 601, -3750, -4130, 4828},
    {484, 169, -1250, 4630, 4121},
    {485, 329, 4630, 1250, 4121},
    {486, 729, -4630, 1250, 4121},
    {487, 249, 1250, 4630, 4121},
    {488, 489, 1250, -4630, 4121},
    {489, 649, -4630, -1250, 4121},
    {490, 409, 4630, -1250, 4121},
    {491, 569, -1250, -4630, 4121},
    {492, 582, -3000, -4380, 4474},
    {493, 422, 4380, -3000, 4474},
    {494, 177, -3000, 4380, 4474},
    {495, 262, 3000, 4380, 4474},
    {496, 742, -4380, 3000, 4474},
    {497, 657, -4380, -3000, 4474},
    {498, 337, 4380, 3000, 4474},
    {499, 497, 3000, -4380, 4474},
    {500, 172, -1750, 4630, 4121},
    {501, 574, -1750, -4630, 4121},
    {502, 734, -4630, 1750, 4121},
    {503, 414, 4630, -1750, 4121},
    {504, 492, 1750, -4630, 4121},
    {505, 332, 4630, 1750, 4121},
    {506, 652, -4630, -1750, 4121},
    {507, 254, 1750, 4630, 4121},
    {508, 639, -4630, -2250, 4121},
    {509, 479, 2250, -4630, 4121},
    {510, 730, -4630, 2250, 4121},
    {511, 159, -2250, 4630, 4121},
    {512, 570, -2250, -4630, 4121},
    {513, 410, 4630, -2250, 4121},
    {514, 319, 4630, 2250, 4121},
    {515, 250, 2250, 4630, 4121},
    {516, 661, -4380, -3500, 4474},
    {517, 760, -4380, 3500, 4474},
    {518, 341, 4380, 3500, 4474},
    {519, 501, 3500, -4380, 4474},
    {520, 181, -3500, 4380, 4474},
    {521, 280, 3500, 4380, 4474},
    {522, 440, 4380, -3500, 4474},
    {523, 600, -3500, -4380, 4474},
    {524, 331, 4880, 0, 3767},
    {525, 491, 0, -4880, 3767},
    {526, 171, 0, 4880, 3767},
    {527, 651, -4880, 0, 3767},
    {528, 728, -4880, 500, 3767},
    {529, 327, 4880, 500, 3767},
    {530, 487, 500, -4880, 3767},
    {531, 248, 500, 4880, 3767},
    {532, 167, -500, 4880, 3767},
    {533, 568, -500, -4880, 3767},
    {534, 647, -4880, -500, 3767},
    {535, 408, 4880, -500, 3767},
    {536, 634, -4630, -2750, 4121},
    {537, 735, -4630, 2750, 4121},
    {538, 575, -2750, -4630, 4121},
    {539, 474, 2750, -4630, 4121},
    {540, 154, -2750, 4630, 4121},
    {541, 255, 2750, 4630, 4121},
    {542, 415, 4630, -2750, 4121},
    {543, 314, 4630, 2750, 4121},
    {544, 163, -1000, 4880, 3767},
    {545, 643, -4880, -1000, 3767},
    {546, 483, 1000, -4880, 3767},
    {547, 323, 4880, 1000, 3767},
    {548, 724, -4880, 1000, 3767},
    {549, 404, 4880, -1000, 3767},
    {550, 564, -1000, -4880, 3767},
    {551, 244, 1000, 4880, 3767},
    {552, 176, -4000, 4380, 4474},
    {553, 496, 4000, -4380, 4474},
    {554, 336, 4380, 4000, 4474},
    {555, 656, -4380, -4000, 4474},
    {556, 766, -4380, 4000, 4474},
    {557, 286, 4000, 4380, 4474},
    {558, 446, 4380, -4000, 4474},
    {559, 606, -4000, -4380, 4474},
    {560, 324, 4880, 1500, 3767},
    {561, 164, -1500, 4880, 3767},
    {562, 243, 1500, 4880, 3767},
    {563, 644, -4880, -1500, 3767},
    {564, 723, -4880, 1500, 3767},
    {565, 484, 1500, -4880, 3767},
    {566, 563, -1500, -4880, 3767},
    {567, 403, 4880, -1500, 3767},
    {568, 318, 4630, 3250, 4121},
    {569, 478, 3250, -4630, 4121},
    {570, 158, -3250, 4630, 4121},
    {571, 756, -4630, 3250, 4121},
    {572, 436, 4630, -3250, 4121},
    {573, 276, 3250, 4630, 4121},
    {574, 596, -3250, -4630, 4121},
    {575, 638, -4630, -3250, 4121},
    {576, 648, -4880, -2000, 3767},
    {577, 488, 2000, -4880, 3767},
    {578, 328, 4880, 2000, 3767},
    {579, 567, -2000, -4880, 3767},
    {580, 247, 2000, 4880, 3767},
    {581, 727, -4880, 2000, 3767},
    {582, 407, 4880, -2000, 3767},
    {583, 168, -2000, 4880, 3767},
    {584, 604, -3750, -4630, 4121},
    {585, 153, -3750, 4630, 4121},
    {586, 633, -4630, -3750, 4121},
    {587, 444, 4630, -3750, 4121},
    {588, 313, 4630, 3750, 4121},
    {589, 473, 3750, -4630, 4121},
    {590, 764, -4630, 3750, 4121},
    {591, 284, 3750, 4630, 4121},
    {592, 251, 2500, 4880, 3767},
    {593, 411, 4880, -2500, 3767},
    {594, 475, 2500, -4880, 3767},
    {595, 315, 4880, 2500, 3767},
    {596, 635, -4880, -2500, 3767},
    {597, 155, -2500, 4880, 3767},
    {598, 571, -2500, -4880, 3767},
    {599, 731, -4880, 2500, 3767},
    {600, 486, 250, -5130, 3414},
    {601, 240, 250, 5130, 3414},
    {602, 560, -250, -5130, 3414},
    {603, 720, -5130, 250, 3414},
    {604, 400, 5130, -250, 3414},
    {605, 646, -5130, -250, 3414},
    {606, 326, 5130, 250, 3414},
    {607, 166, -250, 5130, 3414},
    {608, 322, 5130, 750, 3414},
    {609, 725, -5130, 750, 3414},
    {610, 162, -750, 5130, 3414},
    {611, 565, -750, -5130, 3414},
    {612, 482, 750, -5130, 3414},
    {613, 642, -5130, -750, 3414},
    {614, 405, 5130, -750, 3414},
    {615, 245, 750, 5130, 3414},
    {616, 631, -4880, -3000, 3767},
    {617, 437, 4880, -3000, 3767},
    {618, 151, -3000, 4880, 3767},
    {619, 757, -4880, 3000, 3767},
    {620, 311, 4880, 3000, 3767},
    {621, 597, -3000, -4880, 3767},
    {622, 277, 3000, 4880, 3767},
    {623, 471, 3000, -4880, 3767},
    {624, 762, -4630, 4250, 4121},
    {625, 316, 4630, 4250, 4121},
    {626, 442, 4630, -4250, 4121},
    {627, 636, -4630, -4250, 4121},
    {628, 602, -4250, -4630, 4121},
    {629, 282, 4250, 4630, 4121},
    {630, 156, -4250, 4630, 4121},
    {631, 476, 4250, -4630, 4121},
    {632, 321, 5130, 1250, 3414},
    {633, 641, -5130, -1250, 3414},
    {634, 721, -5130, 1250, 3414},
    {635, 481, 1250, -5130, 3414},
    {636, 161, -1250, 5130, 3414},
    {637, 561, -1250, -5130, 3414},
    {638, 241, 1250, 5130, 3414},
    {639, 401, 5130, -1250, 3414},
    {640, 242, 1750, 5130, 3414},
    {641, 485, 1750, -5130, 3414},
    {642, 722, -5130, 1750, 3414},
    {643, 325, 5130, 1750, 3414},
    {644, 645, -5130, -1750, 3414},
    {645, 165, -1750, 5130, 3414},
    {646, 562, -1750, -5130, 3414},
    {647, 402, 5130, -1750, 3414},
    {648, 147, -3500, 4880, 3767},
    {649, 272, 3500, 4880, 3767},
    {650, 592, -3500, -4880, 3767},
    {651, 432, 4880, -3500, 3767},
    {652, 467, 3500, -4880, 3767},
    {653, 752, -4880, 3500, 3767},
    {654, 627, -4880, -3500, 3767},
    {655, 307, 4880, 3500, 3767},
    {656, 726, -5130, 2250, 3414},
    {657, 566, -2250, -5130, 3414},
    {658, 160, -2250, 5130, 3414},
    {659, 246, 2250, 5130, 3414},
    {660, 480, 2250, -5130, 3414},
    {661, 320, 5130, 2250, 3414},
    {662, 406, 5130, -2250, 3414},
    {663, 640, -5130, -2250, 3414},
    {664, 283, 4000, 4880, 3767},
    {665, 628, -4880, -4000, 3767},
    {666, 148, -4000, 4880, 3767},
    {667, 763, -4880, 4000, 3767},
    {668, 603, -4000, -4880, 3767},
    {669, 443, 4880, -4000, 3767},
    {670, 308, 4880, 4000, 3767},
    {671, 468, 4000, -4880, 3767},
    {672, 753, -5130, 2750, 3414},
    {673, 630, -5130, -2750, 3414},
    {674, 593, -2750, -5130, 3414},
    {675, 470, 2750, -5130, 3414},
    {676, 273, 2750, 5130, 3414},
    {677, 433, 5130, -2750, 3414},
    {678, 150, -2750, 5130, 3414},
    {679, 310, 5130, 2750, 3414},
    {680, 447, 4880, -4500, 3767},
    {681, 287, 4500, 4880, 3767},
    {682, 607, -4500, -4880, 3767},
    {683, 152, -4500, 4880, 3767},
    {684, 312, 4880, 4500, 3767},
    {685, 472, 4500, -4880, 3767},
    {686, 632, -4880, -4500, 3767},
    {687, 767, -4880, 4500, 3767},
    {688, 594, -3250, -5130, 3414},
    {689, 434, 5130, -3250, 3414},
    {690, 274, 3250, 5130, 3414},
    {691, 306, 5130, 3250, 3414},
    {692, 754, -5130, 3250, 3414},
    {693, 146, -3250, 5130, 3414},
    {694, 466, 3250, -5130, 3414},
    {695, 626, -5130, -3250, 3414},
    {696, 145, -3750, 5130, 3414},
    {697, 465, 3750, -5130, 3414},
    {698, 625, -5130, -3750, 3414},
    {699, 278, 3750, 5130, 3414},
    {700, 758, -5130, 3750, 3414},
    {701, 305, 5130, 3750, 3414},
    {702, 598, -3750, -5130, 3414},
    {703, 438, 5130, -3750, 3414},
    {704, 469, 4250, -5130, 3414},
    {705, 629, -5130, -4250, 3414},
    {706, 595, -4250, -5130, 3414},
    {707, 755, -5130, 4250, 3414},
    {708, 309, 5130, 4250, 3414},
    {709, 435, 5130, -4250, 3414},
    {710, 149, -4250, 5130, 3414},
    {711, 275, 4250, 5130, 3414},
    {712, 144, -4750, 5130, 3414},
    {713, 304, 5130, 4750, 3414},
    {714, 599, -4750, -5130, 3414},
    {715, 464, 4750, -5130, 3414},
    {716, 439, 5130, -4750, 3414},
    {717, 759, -5130, 4750, 3414},
    {718, 279, 4750, 5130, 3414},
    {719, 624, -5130, -4750, 3414},
};

void LedArrayInterface::setPinOrder(int16_t led_number, int16_t color_channel_index, uint8_t position)
{
        tlc.setPinOrderSingle(led_number, color_channel_index, position);
}

void LedArrayInterface::setMaxCurrentEnforcement(bool enforce)
{
        TLC5955::enforce_max_current = enforce;
}

void LedArrayInterface::setMaxCurrentLimit(float limit)
{
        if (limit > 0)
                TLC5955::max_current_amps = limit;
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
	EEPROM.write(SN_ADDRESS, lower_8bits_sn);
	EEPROM.write(SN_ADDRESS + 1, upper_8bits_sn);
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
	EEPROM.write(PN_ADDRESS, lower_8bits_pn);
	EEPROM.write(PN_ADDRESS + 1, upper_8bits_pn);
}


void LedArrayInterface::setGsclkFreq(uint32_t gsclk_frequency)
{
  tlc.setGsclkFreq(gsclk_frequency);
}

uint32_t LedArrayInterface::getGsclkFreq()
{
  return tlc.getGsclkFreq();
}

void LedArrayInterface::deviceSetup()
{

        // Initialize TLC5955
        tlc.init(LAT, SPI_MOSI, SPI_CLK, GSCLK);

        // Adjust PWM timer for maximum GSCLK frequency
        setGsclkFreq(gsclk_frequency);

        // Set SPI Baud rate
        setBaudRate(spi_baud_rate);

        // We must set dot correction values, so set them all to the brightest adjustment
        tlc.setAllDcData(127);

        // Set Max Current Values (see TLC5955 datasheet)
        tlc.setMaxCurrent(4, 4, 4); // Go up to 7

        // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
        // DSPRPT, TMGRST, RFRESH, ESPWM, LSDVLT

        // DSPRPT
        // Auto display repeat mode enable bit
        // 0 = Disabled, 1 = Enabled
        // When this bit is 0, the auto display repeat function is disabled. Each constant-current output is turned on and off for one display period.
        // When this bit is 1, each output repeats the PWM control every 65,536 GSCLKs.

        // TMGRST
        // Display timing reset mode enable bit
        // 0 = Disabled, 1 = Enabled
        // When this bit is 0, the GS counter is not reset and the outputs are not forced off even when a LAT rising edge is input for a GS data write.
        // When this bit is 1, the GS counter is reset to 0 and all outputs are forced off at the LAT rising edge for a GS data write. Afterwards, PWM control resumes from the next GSCLK rising edge

        // RFRESH
        // Auto data refresh mode enable bit
        // 0 = Disabled, 1 = Enabled
        // When this bit is 0, the auto data refresh function is disabled. The data in the common shift register are copied to the GS data latch at the next LAT rising edge for a GS data write. DC data in the control data latch are copied to the DC data latch at the same time.
        // When this bit is 1, the auto data refresh function is enabled. The data in the common shift register are copied to the GS data latch at the 65,536th GSCLK after the LAT rising edge for a GS data write. DC data in the control data latch are copied to the DC data latch at the same time.

        // ESPWM
        // ES-PWM mode enable bit
        // 0 = Disabled, 1 = Enabled
        // When this bit is 0, the conventional PWM control mode is selected. If the TLC5955 is used for multiplexing a drive, the conventional PWM mode should be selected to prevent excess on or off switching.
        // When this bit is 1, ES-PWM control mode is selected.

        // LSDVLT
        // LSD detection voltage selection bit
        // LED short detection (LSD) detects a fault caused by a shorted LED by comparing the OUTXn voltage to the LSD detection threshold voltage. The threshold voltage is selected by this bit.
        // When this bit is 0, the LSD voltage is VCC × 70%. When this bit is 1, the LSD voltage is VCC × 90%.

        tlc.setFunctionData(true, true, true, true, false);

        // Set LED current levels (7-bit, max is 127)
        int currentR = 127;
        int currentB = 127;
        int currentG = 127;
        tlc.setBrightnessCurrent(currentR, currentB, currentG);

        // Update vontrol register
        tlc.updateControl();
        tlc.updateControl();

        // Set RGB pin order
        tlc.setRgbPinOrder(0, 1, 2);

        // SN-specific pin corrections
        if (getSerialNumber() == 17)
        {
                tlc.setRgbPinOrderSingle(141, 2, 1, 0);
                tlc.setRgbPinOrderSingle(1209, 2, 1, 0);
                tlc.setRgbPinOrderSingle(1251, 2, 1, 0);
        }

        // swap green and red for custom led connection
        tlc.setRgbPinOrderSingle(459, 1, 0, 2); // led 879
        tlc.setRgbPinOrderSingle(1131, 1, 0, 2); // led 881
        tlc.setRgbPinOrderSingle(795, 1, 0, 2); // led 882
        tlc.setRgbPinOrderSingle(1467, 1, 0, 2); // led 883

        // Clear the array
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
                     if (device_command_index == 0)
                       return (uint16_t)pgm_read_word(&(center_led_list[pattern_index][led_index]));
                     else
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
void LedArrayInterface::setBaudRate(uint32_t new_baud_rate)
{
  tlc.setSpiBaudRate(new_baud_rate);
}

uint32_t LedArrayInterface::getBaudRate()
{
  return tlc.getSpiBaudRate();
}

int8_t LedArrayInterface::getDemoMode()
{
  int8_t demo_mode_read = EEPROM.read(DEMO_MODE_ADDRESS);
  return (demo_mode_read);
}

void LedArrayInterface::setDemoMode(int8_t demo_mode)
{
	EEPROM.write(DEMO_MODE_ADDRESS, demo_mode);
}

#endif

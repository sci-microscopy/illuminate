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

#define LICENSE_TEXT                                                           \
  "Copyright (c) 2021, Zack Phillips\n" \
  "Copyright (c) 2018, Zachary Phillips (UC Berkeley)\n" \
  "All rights reserved.\n" \
  "\n"\
  "BSD 3-Clause License\n"\
  "\n"\
  "Redistribution and use in source and binary forms, with or without\n" \
  "modification, are permitted provided that the following conditions are met:\n" \
  "    Redistributions of source code must retain the above copyright\n" \
  "    notice, this list of conditions and the following disclaimer.\n" \
  "    Redistributions in binary form must reproduce the above copyright\n" \
  "    notice, this list of conditions and the following disclaimer in the\n" \
  "    documentation and/or other materials provided with the distribution.\n" \
  "    Neither the name of the UC Berkley nor the\n" \
  "    names of its contributors may be used to endorse or promote products\n" \
  "    derived from this software without specific prior written permission.\n" \
  "\n"\
  "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND\n" \
  "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n" \
  "WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n" \
  "DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY\n" \
  "DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n" \
  "(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n" \
  "LOSS OF USE, DATA , OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n" \
  "ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n" \
  "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n" \
  "  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
#define SERIAL_BAUD_RATE 115200

// Include various files depending on which LED array is used
#include "illuminate.h"
#include "commandrouting.h"
#include "commandconstants.h"
#include "ledarray.h"
#include "ledarrayinterface.h"

#define USE_STATIC_ALLOCATION 1

#if USE_STATIC_ALLOCATION
  #define BUFFER_SIZE 1024 * 16
  #define ARGV_MAX 300
  char serial_buffer[BUFFER_SIZE];
  const char *argv_buffer[ARGV_MAX];
#endif

// Initialize objects
LedArrayInterface led_array_interface;
LedArray led_array;
CommandRouter cmd;

// This command runs once at power-on
void setup() 
{
  // Initialize serial interface
  Serial.begin(SERIAL_BAUD_RATE);

  #if USE_STATIC_ALLOCATION
  cmd.init_no_malloc(command_list, BUFFER_SIZE, serial_buffer, ARGV_MAX,
                     argv_buffer);
  #else
    cmd.init(command_list, 1024, 10);
  #endif

  // Initialize LED Array
  led_array.setInterface(&led_array_interface);
  led_array.setup();
  
  // Print the about screen when connected
  led_array.printAbout(0, (char **) NULL);

}

// This command runs continuously after setup() runs once
void loop()
{
  // Loop until we recieve a command, then parse it.
  if (Serial.available())
    cmd.processSerialStream();
}

int info_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printAbout(argc, (char * *) argv);}
int reset_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.reset(argc, (char * *) argv);}
int version_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printVersion(argc, (char * *) argv);}
int license_func(CommandRouter *cmd, int argc, const char **argv) { Serial.print(LICENSE_TEXT); return NO_ERROR; }
int demo_func(CommandRouter *cmd, int argc, const char **argv) { return led_array.setDemoMode(argc, (char * *) argv); }
int help_func(CommandRouter *cmd, int argc, const char **argv) { return cmd->help();}
int store_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.store_parameters(argc, (char * *) argv);}
int recall_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.recall_parameters(argc, (char * *) argv);}

int autoclear_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.toggleAutoClear(argc, (char * *) argv); }
int na_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setNa(argc, (char * *) argv); }
int na_inner_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setInnerNa(argc, (char * *) argv); }
int color_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setColor(argc, (char * *) argv); }
int brightness_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setBrightness(argc, (char * *) argv); }
int array_distance_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setArrayDistance(argc, (char * *) argv); }
int cosine_func(CommandRouter *cmd, int argc, const char **argv) { return led_array.setCosineFactor(argc, (char * *) argv); }

int set_led_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawLedList(argc, (char * *)argv); }

int clear_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.clear(); }
int fill_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.fillArray(); }
int brightfield_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawBrightfield(argc, (char * *) argv); }
int darkfield_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawDarkfield(argc, (char * *) argv); }
int dpc_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawDpc(argc, (char * *) argv); }
int cdpc_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawCdpc(argc, (char * *)argv); }
int annulus_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawAnnulus(argc, (char * *)argv); }
int half_annulus_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawHalfAnnulus(argc, (char * *)argv); }
int quadrant_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawQuadrant(argc, (char * *) argv); }
int color_darkfield_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawColorDarkfield(argc, (char * *) argv); }
int navigator_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawNavigator(argc, (char * *) argv); }

int scan_full_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.scanAllLeds(argc, (char * *) argv); }
int scan_brightfield_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.scanBrightfieldLeds(argc, (char * *) argv); }
int scan_darkfield_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.scanDarkfieldLeds(argc, (char * *) argv); }

int set_sequence_length_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setSequenceLength(argc, (char * *) argv); }
int set_sequence_value_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setSequenceValue(argc, (char * *) argv); }
int run_sequence_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.runSequence(argc, (char * *) argv); }
int run_sequence_fast_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.runSequenceFast(argc, (char * *) argv); }
int print_sequence_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printSequence(argc, (char * *) argv); }
int print_sequence_length_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printSequenceLength(argc, (char * *) argv); }
int step_sequence_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.stepSequence(argc, (char * *) argv); }
int reset_sequence_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.resetSequence(argc, (char * *) argv); }
int set_sequence_zeros_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setSequenceZeros(argc, (char * *) argv); }
int set_sequence_bit_depth_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setSequenceBitDepth(argc, (char * *) argv); }

int trigger_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.sendTriggerPulse(argc, (char * *) argv); }
int trigger_setup_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.triggerSetup(argc, (char * *) argv); }
int trigger_test_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.triggerInputTest(strtoul((char *) argv[0], NULL, 0)); }
int draw_channel_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawChannel(argc, (char * *) argv); }
int debug_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setDebug(argc, (char * *) argv); }
int set_pin_order_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setPinOrder(argc, (char * *) argv); }
int wait_func(CommandRouter *cmd, int argc, const char **argv){ if (argc == 0) delay(1000); else delay(strtoul(argv[1], NULL, 0)); return NO_ERROR; }
int set_max_current_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setMaxCurrentLimit(argc, (char * *) argv); }
int set_max_current_enforcement_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setMaxCurrentEnforcement(argc, (char * *) argv); }

int print_led_values_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printCurrentLedValues(argc, (char * *) argv); }
int print_led_positions_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printLedPositions(argc, (char * *) argv, false); }
int print_parameters_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.printSystemParams(argc, (char * *) argv); }
int print_led_positions_na(CommandRouter *cmd, int argc, const char **argv){ return led_array.printLedPositions(argc, (char * *) argv, true); }

int disco_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.drawDiscoPattern(); }
int water_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.waterDrop(); }

int set_pn_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setPartNumber(argc, (char * *) argv); }
int set_sn_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setSerialNumber(argc, (char * *) argv); }

int run_fpm_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.runSequenceFpm(argc, (char * *) argv); }
int run_dpc_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.runSequenceDpc(argc, (char * *) argv); }

int set_baud_rate_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setBaudRate(argc, (char * *) argv); }
int set_gsclk_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setGsclkFreq(argc, (char * *) argv); }

int set_human_mode_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setCommandMode("human"); }
int set_machine_mode_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setCommandMode("machine"); }

int power_sensing_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.set_power_supply_sensing(argc, (char * *) argv); }
int power_source_voltage_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.print_power_supply_voltage(argc, (char * *) argv); }

int trigger_input_timeout_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setTriggerInputTimeout(argc, (char * *) argv); }
int trigger_output_pulse_width_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setTriggerOutputPulseWidth(argc, (char * *) argv); }
int trigger_input_polarity_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setTriggerInputPolarity(argc, (char * *) argv); }
int trigger_output_polarity_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setTriggerOutputPolarity(argc, (char * *) argv); }
int trigger_output_delay_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.setTriggerOutputDelay(argc, (char * *) argv); }
int trigger_input_pin_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.getTriggerInputPins(argc, (char * *) argv); }
int trigger_output_pin_func(CommandRouter *cmd, int argc, const char **argv){ return led_array.getTriggerOutputPins(argc, (char * *) argv); }

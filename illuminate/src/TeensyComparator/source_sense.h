/*
  TeensyComparator (CMP0):
  - 0. CMP0_IN0 (Teensy Pin 11)
  - 1. CMP0_IN1 (Teensy Pin 12)
  - 2. CMP0_IN2 (Teensy Pin 28)
  - 3. CMP0_IN3 (Teensy Pin 27)
  - 4. CMP0_IN4 (Teensy Pin 29)
  - 5. VREF Output (1.2V)
  - 6. Bandgap
  - 7. 6 bit DAC0 Reference

  TeensyComparator1 (CMP1):
  - 0. CMP1_IN0 (Teensy Pin 23)
  - 1. CMP1_IN1 (Teensy Pin 9)
  - 3. 12-bit DAC0_OUT/CMP1_IN3 (Teensy Pin 40/A14/DAC)
  - 5. VREF Output (1.2V)
  - 6. Bandgap
  - 7. 6 bit DAC1 Reference
  TeensyComparator2 (CMP2 Teensy 3.[12] only):
  - 0. CMP2_IN0 (Teensy Pin 3)
  - 1. CMP2_IN1 (Teensy Pin 4)
  - 6. Bandgap
  - 7. 6 bit DAC2 Reference
*/

#ifndef SOURCE_SENSE_H
#define SOURCE_SENSE_H

#include "error_codes.h"

#define SOURCE_SENSE_PIN 23

static bool source_is_connected = false;
uint32_t warning_delay_ms = 1000;
elapsedMillis time_elapsed;

void source_sense_interrupt()
{
  noInterrupts();
  source_is_connected = TeensyComparator1.state();

  if ((source_is_connected == false) && (time_elapsed > warning_delay_ms))
  {
    Serial.printf("[%s] WARNING: Power is disconnected! LED array will not illuminate.\n", ERROR_CODE_POWER_DISCONNECTED);
    time_elapsed = 0; // Reset timer
  }
  interrupts();
}

void setup_source_sensing()
{
  TeensyComparator1.set_pin(0, 5);
  TeensyComparator1.set_interrupt(source_sense_interrupt, CHANGE);

  // Read source sense pin voltage
  source_is_connected = TeensyComparator1.state();
}

#endif

/*
  TeensyComparator
  Exposes the comparator functionality in Teensy 3.2

  Inspired by analogComp, written by Leonardo Miliani
*/


#ifndef TEENSY_COMP_H
#define TEENSY_COMP_H

#include "Arduino.h"

//used to set the man number of analog pins
#ifdef NUM_ANALOG_INPUTS
#undef NUM_ANALOG_INPUTS
#endif

// Ensure we're using a Teensy 3.2
#ifndef __MK20DX256__
#error Teensy 3.2 Required!
#endif

#ifndef ADMUX
#define ADMUX ADMUXA
#endif

#ifndef ACBG
#define ACBG ACPMUX2
#endif

#ifndef ACME
#define ACME ACNMUX0
#endif

// AIN constants
const uint8_t AIN0 = 0;
const uint8_t INTERNAL_REFERENCE = 1;
const uint8_t AIN1 = 255;

class TeensyComparator
{
    public:
        //public methods
        TeensyComparator(volatile uint8_t *base, void(*set_pin_cb)(uint8_t, uint8_t));

        // Set and unset pin
        int8_t set_pin(uint8_t = AIN0, uint8_t = AIN1);
        int8_t unset_pin(void);

        // Set and unset reference
        int8_t set_reference_value(uint8_t value);
        int8_t unset_reference_value(void);

        // Set and unset interrupt
        int8_t set_interrupt(void (*)(void), uint8_t tempMode = CHANGE);
        int8_t unset_interrupt(void);

        // Current state of pin relative to reference
        int8_t state(void);

        // To be deleted
        int8_t set_comparator_dac(uint8_t val);

        // Interrupt function
        void (*_interrupt_function)(void);

    private:
        int8_t _comparator_index;
        uint8_t _initialized;
        uint8_t _interrupt_enabled;
        volatile uint8_t *CMP_BASE_ADDR;
        void (*_set_pinCb)(uint8_t, uint8_t);
};

extern TeensyComparator TeensyComparator0;
extern TeensyComparator TeensyComparator1;
extern TeensyComparator TeensyComparator2;

#endif

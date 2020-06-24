/* This file is part of the TeensyComparator library.
   Please check the README file and the notes
   inside the TeensyComparator.h file
*/

//include required libraries
#include "TeensyComparator.h"


//#define CMP_SCR_DMAEN   ((uint8_t)0x40) // DMA Enable Control
//#define CMP_SCR_IER     ((uint8_t)0x10) // Comparator Interrupt Enable Rising
//#define CMP_SCR_IEF     ((uint8_t)0x08) // Comparator Interrupt Enable Falling
//#define CMP_SCR_CFR     ((uint8_t)0x04) // Analog Comparator Flag Rising
//#define CMP_SCR_CFF     ((uint8_t)0x02) // Analog Comparator Flag Falling
//#define CMP_SCR_COUT    ((uint8_t)0x01) // Analog Comparator Output
#define CMPx_CR0        0 // CMP Control Register 0
#define CMPx_CR1        1 // CMP Control Register 1
#define CMPx_FPR        2 // CMP Filter Period Register
#define CMPx_SCR        3 // CMP Status and Control Register
#define CMPx_DACCR      4 // DAC Control Register
#define CMPx_MUXCR      5 // MUX Control Register

TeensyComparator::TeensyComparator(volatile uint8_t *base, void(*set_pin_cb)(uint8_t, uint8_t)) :
  _initialized(0), _interrupt_enabled(0), CMP_BASE_ADDR(base), _set_pinCb(set_pin_cb)
{
}

//setting and switching on the analog comparator
int8_t TeensyComparator::set_pin(uint8_t tempAIN0, uint8_t tempAIN1)
{

  // Check if comparator is already running
  if (_initialized)
    return -1;

  // Clock gate: comparator clock set on.
  // It is off by default for conserving power.
  SIM_SCGC4 |= SIM_SCGC4_CMP;

  _set_pinCb(tempAIN0, tempAIN1);
  CMP_BASE_ADDR[CMPx_CR1] = 0; // set CMPx_CR1 to a known state
  CMP_BASE_ADDR[CMPx_CR0] = 0; // set CMPx_CR0 to a known state
  CMP_BASE_ADDR[CMPx_CR1] = B00000001; // enable comparator
  CMP_BASE_ADDR[CMPx_MUXCR] = tempAIN0 << 3 | tempAIN1; // set comparator input PSEL=tempAIN0 / MSEL=tempAIN1

  // Set initialization flag
  _initialized = 1;
  return 1;
}

int8_t TeensyComparator::unset_pin()
{
  if (!_initialized)
    return -1;
  else
  {
    // Disable interrupt if enabled
    if (_interrupt_enabled) {
      CMP_BASE_ADDR[CMPx_SCR] = 0; //disable interrupts on AC events
      _interrupt_enabled = 0;
    }

    // Switch off the analog comparator
    CMP_BASE_ADDR[CMPx_CR1] = 0; //switch off the AC

    // Unset initialization flag
    _initialized = 0;

    // Return
    return 1;
  }
}

int8_t TeensyComparator::set_interrupt(void (*temp_interrupt_function)(void), uint8_t interrupt_mode)
{
  // Disable interrupt if it's enabled
  if (_interrupt_enabled) {
    CMP_BASE_ADDR[CMPx_SCR] = 0;
  }

  // Ensure we're initialized
  if (!_initialized)
    return -1;

  // Set the interrupt function
  _interrupt_function = temp_interrupt_function;

  // Set the interrupt mode
  if (interrupt_mode == CHANGE)
    CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IER | CMP_SCR_IEF;
  else if (interrupt_mode == FALLING)
    CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IEF;
  else if (interrupt_mode == RISING)
    CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IER;
  else
    return -1; // We should not reach here.

  // Set enabled flag
  _interrupt_enabled = 1;

  // Return
  return 1;
}

int8_t TeensyComparator::unset_interrupt(void)
{
  // Ensure we're initialized and the interrupt is enabled
  if ((!_initialized) || (!_interrupt_enabled))
    return -1;

  // Disable interrupt
  CMP_BASE_ADDR[CMPx_SCR] = 0; //disable interrupt
  _interrupt_enabled = 0;

  // Return
  return 1;
}

int8_t TeensyComparator::state()
{
  // Output variable
  uint8_t _state = 0;

  // Return error if not initialized
  if (!_initialized)
    return -1;

  // Check if output flag is true
  if ((CMP_BASE_ADDR[CMPx_SCR] & CMP_SCR_COUT))
    _state = 1;
  
  return _state;
}

int8_t TeensyComparator::set_comparator_dac(uint8_t val)
{
  uint8_t tmp8 = 0U;

  if (val == 0)
  {
    CMP0_DACCR = 0U;
    return 1;
  }

  /* CMPx_DACCR. */
  tmp8 |= CMP_DACCR_DACEN; /* Enable the internal DAC. */
  tmp8 |= CMP_DACCR_VRSEL; /* Vin2 */
  tmp8 |= CMP_DACCR_VOSEL(val);

  CMP0_DACCR = tmp8;

  // Return
  return 1;
}


//ISR (Interrupt Service Routine) called by the analog comparator when
//the user choose the raise of an interrupt
void cmp0_isr()
{
  // Clear CFR and CFF
  CMP0_SCR |= CMP_SCR_CFR | CMP_SCR_CFF;

  // Call the interrput function
  TeensyComparator0._interrupt_function();
}

static void _init_comparator_0(uint8_t inp, uint8_t inm)
{

  // Set interrupt priority
  NVIC_SET_PRIORITY(IRQ_CMP0, 64);

  // Set interrupt
  NVIC_ENABLE_IRQ(IRQ_CMP0);

  // Define pins
  if (inp == 0 || inm == 0) // CMP0_IN0 (Teensy Pin 11)
    CORE_PIN11_CONFIG = PORT_PCR_MUX(0);
  if (inp == 1 || inm == 1) // CMP0_IN1 (Teensy Pin 12)
    CORE_PIN12_CONFIG = PORT_PCR_MUX(0);
  if (inp == 2 || inm == 2) // CMP0_IN2 (Teensy Pin 28)
    CORE_PIN28_CONFIG = PORT_PCR_MUX(0);
  if (inp == 3 || inm == 3) // CMP0_IN3 (Teensy Pin 27)
    CORE_PIN27_CONFIG = PORT_PCR_MUX(0);
  if (inp == 4 || inm == 4) // CMP0_IN4 (Teensy Pin 29)
    CORE_PIN29_CONFIG = PORT_PCR_MUX(0);
  if (inp == 5 || inm == 5) // VREF Output/CMP0_IN5
    ; // do nothing
  if (inp == 6 || inm == 6) // Bandgap
    ; // do nothing
  if (inp == 7 || inm == 7) // 6b DAC0 Reference
    ; // do nothing
}

void cmp1_isr()
{
  // Clear CFR and CFF
  CMP1_SCR |= CMP_SCR_CFR | CMP_SCR_CFF;

  // Call the interrupt function
  TeensyComparator1._interrupt_function(); //call the user function
}

static void _init_comparator_1(uint8_t inp, uint8_t inm)
{
  // Set interrupt priority
  NVIC_SET_PRIORITY(IRQ_CMP1, 64);

  // Set interrupt
  NVIC_ENABLE_IRQ(IRQ_CMP1);

  // Assign Pin
  if (inp == 0 || inm == 0) { // CMP1_IN0 (Teensy Pin 23)
    CORE_PIN23_CONFIG = PORT_PCR_MUX(0);
  }
  if (inp == 1 || inm == 1) { // CMP1_IN1 (Teensy Pin 9)
    CORE_PIN9_CONFIG = PORT_PCR_MUX(0);
  }
  if (inp == 3 || inm == 3) { // 12-bit DAC0_OUT/CMP1_IN3
    ; // We should not reach here.
  }
  if (inp == 5 || inm == 5) { // VREF Output/CMP1_IN5
    ; // do nothing
  }
  if (inp == 6 || inm == 6) { // Bandgap
    ; // do nothing
  }
  if (inp == 7 || inm == 7) { // 6b DAC1 Reference
    ; // do nothing
  }
}

void cmp2_isr()
{
  // Clear CFR and CFF
  CMP2_SCR |= CMP_SCR_CFR | CMP_SCR_CFF; // clear CFR and CFF

  // Call the interrupt function
  TeensyComparator2._interrupt_function(); //call the user function
}

static void _init_comparator_2(uint8_t inp, uint8_t inm)
{
  // Set interrupt priority
  NVIC_SET_PRIORITY(IRQ_CMP2, 64);

  // Set interrupt
  NVIC_ENABLE_IRQ(IRQ_CMP2);

  // Assign Pin
  if (inp == 0 || inm == 0) // CMP2_IN0 (Teensy Pin 3)
    CORE_PIN3_CONFIG = PORT_PCR_MUX(0);
  if (inp == 1 || inm == 1) // CMP2_IN1 (Teensy Pin 4)
    CORE_PIN4_CONFIG = PORT_PCR_MUX(0);
  if (inp == 6 || inm == 6) // Bandgap
    ; // do nothing
  if (inp == 7 || inm == 7) // 6b DAC2 Reference
    ; // do nothing
}

// Create comparator objects
TeensyComparator TeensyComparator0(&CMP0_CR0, _init_comparator_0);
TeensyComparator TeensyComparator1(&CMP1_CR0, _init_comparator_1);
TeensyComparator TeensyComparator2(&CMP2_CR0, _init_comparator_2);

#include "pisk68xx.h"

void __attribute__((noinline)) writePixels8Mhz(uint8_t pin, uint8_t data[], uint8_t data_len) {
  /*
  8 MHz - 125ns (0.125us)

  Min code period time: 1.2us

  0 code timing: 
    - 0.375us HIGH (3 cycles)
    - 0.875us LOW (7 cycles)
  1 code timing:
    - 0.75us HIGH (6 cycles)
    - 0.5us LOW (4 cycles)

  */

  asm volatile(
    "cli \n"  // disable interrupts

    "start: "
    " ld r16, Z+ \n"  // Load next byte into r16
    " ldi r17, 8 \n"  // set bit counter to 8

    "bitloop: "
    " lsl r16 \n"                 // Shift the bit we're working on into C flag
    " sts %[setport], %[pin] \n"  // Set pin to HIGH (2 cycles)
    " brcs sendhigh \n"

    "sendlow:"
    " sts %[clrport], %[pin] \n"  // Send 0 bit (2 cycles)
    " rjmp next \n"

    "sendhigh:"
    " nop \n nop \n"

    "endlow:"
    " sts %[clrport], %[pin] \n"  // End of 1 code (2 cycles)

    "next:"
    " dec r17 \n"  // Are we through all 8 bits?
    " breq nextbyte \n"
    " rjmp bitloop \n"

    "nextbyte:"
    " dec %[data_len] \n"
    " brne start \n"

    "sei \n"
    :
    : [setport] "n"(_SFR_MEM_ADDR(PORTB.OUTSET)),
      [clrport] "n"(_SFR_MEM_ADDR(PORTB.OUTCLR)),
      [pin] "r"(pin),
      [data_len] "r"(data_len),
      [data] "z"(data)
    : "r16", "r17");
}

void __attribute__((noinline)) writePixels20Mhz(uint8_t pin, uint8_t data[], uint8_t data_len) {
  /*
  20 MHz - 50ns (0.05us)

  Min code period time: 1.2us

  0 code timing: 
    - 0.35us HIGH (7 cycles)
    - 0.85us LOW (17 cycles)
  1 code timing:
    - 0.65us HIGH (13 cycles)
    - 0.55us LOW (11 cycles)

  */

  asm volatile(
    "cli \n"  // disable interrupts

    "start: "
    " ld r16, Z+ \n"  // Load next byte into r16
    " ldi r17, 8 \n"  // set bit counter to 8

    "bitloop: "
    " sts %[setport], %[pin] \n"  // Set pin to HIGH (2 cycles)
    " lsl r16 \n"                 // Shift the bit we're working on into C flag
    " nop \n nop \n nop \n"
    " brcs sendhigh \n"

    "sendlow:"
    " sts %[clrport], %[pin] \n"  // Send 0 bit (2 cycles)
    " nop \n nop \n"
    " rjmp endlow \n"

    "sendhigh:"
    " nop \n nop \n nop \n nop \n nop \n"

    "endlow:"
    " sts %[clrport], %[pin] \n"  // End of 1 code (2 cycles)

    "next:"
    " dec r17 \n"  // Are we through all 8 bits?
    " nop \n"
    " breq nextbyte \n"
    " nop \n nop \n nop \n nop \n"
    " rjmp bitloop \n"

    "nextbyte:"
    " dec %[data_len] \n"
    " brne start \n"

    "sei \n"
    :
    : [setport] "n"(_SFR_MEM_ADDR(PORTB.OUTSET)),
      [clrport] "n"(_SFR_MEM_ADDR(PORTB.OUTCLR)),
      [pin] "r"(pin),
      [data_len] "r"(data_len),
      [data] "z"(data)
    : "r16", "r17");
}
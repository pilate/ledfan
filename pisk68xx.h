#ifndef pisk68xx_h
#define pisk68xx_h

#include "Arduino.h"

void __attribute__((noinline)) writePixels20Mhz(uint8_t pin, uint8_t data[], uint8_t data_len);
void __attribute__((noinline)) writePixels8Mhz(uint8_t pin, uint8_t data[], uint8_t data_len);

#if F_CPU == 8000000L
    #define writePixels writePixels8Mhz
#elif F_CPU == 20000000L
    #define writePixels writePixels20Mhz
#else
    #error "Unsupported clock frequency. Please use 8MHz or 20MHz."
#endif

#endif

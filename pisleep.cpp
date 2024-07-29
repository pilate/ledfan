#include <avr/sleep.h>
#include "pisleep.h"

ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS = RTC_PI_bm;
}

void sleep_setup() {
  while (RTC.STATUS)
    ;
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
}

// cant get dynamic / accurate sleep timing to work, fk it just sleep 4ms at a time
void sleep(uint16_t sleep_ms) {
  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
    ;

  RTC.PITINTCTRL = RTC_PI_bm;
  RTC.PITCTRLA = RTC_PERIOD_CYC4_gc | RTC_PITEN_bm;

  sleep_ms = max((uint16_t)1, sleep_ms / 4);

  sei();
  for (uint8_t i = 0; i < sleep_ms; i++) {
    sleep_cpu();
  }

  RTC.PITCTRLA = 0;
  RTC.PITINTCTRL = RTC_PI_bp;
}

// void __attribute__((noinline)) sleep(uint16_t delay_ms) {
//   uint16_t timeout;
//   uint8_t period;

//   delay_ms = delay_ms * 2;

//   RTC.PITINTCTRL = RTC_PI_bm;

//   for (timeout = 256, period = 7; period > 0; timeout /= 2, period--) {
//     if (delay_ms >= timeout) {
//       RTC.PITCTRLA = (period << 3) | RTC_PITEN_bm;
//       while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
//         ;
//       while (delay_ms >= timeout) {
//         sleep_cpu();
//         delay_ms -= timeout;
//       }

//       // RTC.PITCTRLA = 0;
//       // while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
//       //   ;
//     }
//     if (delay_ms) {
//       delay(delay_ms / 2);
//     }
//   }

//   RTC.PITINTCTRL = RTC_PI_bp;
// }

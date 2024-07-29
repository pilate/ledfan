
#include <SPI.h>
#include "lib/fastled/FastLED.h"
#include "pisk68xx.h"
#include "pisleep.h"

#include <tinyNeoPixel.h>

#define NUM_LEDS 5
#define TOTAL_LEDS 10

#define BRIGHTNESS_DIV 4

// #pragma GCC optimize "O0"

CRGB STRIP1[NUM_LEDS];
CRGB STRIP2[NUM_LEDS];
CRGB STRIP3[TOTAL_LEDS];

uint8_t dynamic_data[NUM_LEDS * 3];
uint8_t dynamic_data2[NUM_LEDS * 3];

tinyNeoPixel leds = tinyNeoPixel(5, PIN_PB0, NEO_GRB);
tinyNeoPixel leds2 = tinyNeoPixel(5, PIN_PB1, NEO_GRB);

// SK6803MINI-E is GRB not RGB
uint8_t RGB_ORDER[] = { 1, 0, 2 };

// Convert CRGB array to raw RGB data
void drawCRGB(CRGB leds[], uint8_t data[]) {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      data[(i * 3) + RGB_ORDER[j]] = leds[i][j] / BRIGHTNESS_DIV;
    }
  }
}

// Convert CRGB array into two arrays of RGB data
void drawCRGB2(CRGB leds[], uint8_t data[], uint8_t data2[]) {
  uint8_t offset = 0;
  uint8_t value = 0;

  for (uint8_t i = 0; i < TOTAL_LEDS; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      value = leds[i][j] / BRIGHTNESS_DIV;
      offset = ((i / 2) * 3) + RGB_ORDER[j];
      if ((i % 2) == 0) {
        data[offset] = value;
      } else {
        data2[offset] = value;
      }
    }
  }
}

// Write a single byte to an SPI register
uint16_t writeSPIByte(uint8_t reg, uint8_t value) {
  uint16_t data = 0;

  PORTC.OUTCLR = PIN3_bm;

  SPI.transfer(reg);
  SPI.transfer(value);

  PORTC.OUTSET = PIN3_bm;
  SPI.endTransaction();

  return data;
}

// Return two bytes from the SPI register, second byte first
uint16_t readTwoSPIBytes(uint8_t reg) {
  uint16_t data = 0;

  PORTC.OUTCLR = PIN3_bm;

  SPI.transfer(reg | 0x80);

  data |= SPI.transfer(0x00);
  data |= ((uint16_t)SPI.transfer(0x00)) << 8;

  PORTC.OUTSET = PIN3_bm;

  return data;
}

// Flash green if the STK chip_id validates, red if not
void checkChipId() {
  uint16_t chipId = readTwoSPIBytes(0x00);

  if (chipId == 0x23) {
    fill_solid(STRIP1, NUM_LEDS, CRGB::Green);
  } else {
    fill_solid(STRIP1, NUM_LEDS, CRGB::Red);
  }

  drawCRGB(STRIP1, dynamic_data);

  writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  writePixels(PIN1_bm, dynamic_data, NUM_LEDS * 3);

  sleep(1000);

  clearPixels();
}

// Turn all LEDs off
void clearPixels() {
  fill_solid(STRIP1, NUM_LEDS, CRGB::Black);

  drawCRGB(STRIP1, dynamic_data);

  writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  writePixels(PIN1_bm, dynamic_data, NUM_LEDS * 3);
}

void drawBits(uint16_t value) {
  fill_solid(STRIP3, TOTAL_LEDS, CRGB::Black);

  uint16_t mask = 0b1000000000;
  for (int i = 0; i < 10; i++) {
    if (value & mask) {
      STRIP3[i] = CRGB::Blue;
    }
    mask >>= 1;
  }

  drawCRGB2(STRIP3, dynamic_data, dynamic_data2);

  writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  writePixels(PIN1_bm, dynamic_data2, NUM_LEDS * 3);
}

void setup() {
  // Configure all pins as output to save power
  // https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PowerSave.md
  PORTA.DIRSET = 0xff;
  PORTA.OUTSET = 0xff;

  PORTB.DIRSET = 0xff;
  PORTB.OUTSET = 0xff;

  PORTC.DIRSET = 0xff;
  PORTC.OUTSET = 0xff;

  // Disable ADCs
  ADC0.CTRLA = 0;
  ADC1.CTRLA = 0;

  // Disable default timer
  TCA0.SPLIT.CTRLA = 0;
  TCD0.CTRLA = 0;

  sleep_setup();

  // Use alternate SPI pins
  SPI.swap(1);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  checkChipId();

  writeSPIByte(0x11, 0b10000000);  // suspend
  writeSPIByte(0x0f, 0b00000101);  // set sensitivity to 4g
  writeSPIByte(0x11, 0b00000000);  // resume
}

// uint8_t gHue = 0;
uint8_t dot = 0;
uint8_t colorIndex = 0;

void loop() {
  int16_t xAccel = (int16_t)readTwoSPIBytes(0x82);  // READ from 0x02
  uint16_t posXAccel = abs(xAccel) >> 8;

  int8_t direction = 0;
  if (posXAccel > 1) {
    if (xAccel > 0) {
      direction = -1;
    } else {
      direction = 1;
    }
  }

  // -- Moving two dots

  dot = (dot + TOTAL_LEDS + direction) % TOTAL_LEDS;
  uint8_t dot2 = (dot + NUM_LEDS) % TOTAL_LEDS;

  fill_solid(STRIP3, TOTAL_LEDS, CRGB::Black);
  STRIP3[dot] = ColorFromPalette(PartyColors_p, colorIndex++);
  STRIP3[dot2] = ColorFromPalette(PartyColors_p, colorIndex++);

  drawCRGB2(STRIP3, dynamic_data, dynamic_data2);

  writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  writePixels(PIN1_bm, dynamic_data2, NUM_LEDS * 3);

  // -- Moving one dot

  // dot = (dot + NUM_LEDS + direction) % NUM_LEDS;

  // fill_solid(STRIP1, NUM_LEDS, CRGB::Black);
  // STRIP1[dot] = CRGB::Blue;

  // drawCRGB(STRIP1, dynamic_data);

  // writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  // writePixels(PIN1_bm, dynamic_data, NUM_LEDS * 3);

  // -- Moving rainbow

  // uint8_t thisHue = beat8(20, 255);                     // A simple rainbow march.
  // fill_rainbow(STRIP3, TOTAL_LEDS, thisHue, direction);            // Use FastLED's fill_rainbow routine.
  // drawCRGB2(STRIP3, dynamic_data, dynamic_data2);

  // writePixels(PIN0_bm, dynamic_data, NUM_LEDS * 3);
  // writePixels(PIN1_bm, dynamic_data2, NUM_LEDS * 3);

  sleep(90 - min((uint16_t)90, (posXAccel * 2)));
}

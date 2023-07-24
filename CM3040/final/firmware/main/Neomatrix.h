#ifndef NEOMATRIX_WRAPPER_H
#define NEOMATRIX_WRAPPER_H

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

class Neomatrix {
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
 private:
  const uint64_t waqi_mask = 0x000000000F0F0F0F;
  const uint64_t aqi_mask = 0x0F0F0F0F00000000;
  uint64_t humid_mask = 0x3030303030303030;
  uint64_t temp_mask = 0xC0C0C0C0C0C0C0C0;
  uint64_t wifi_mask = 0x0F09090F;
  uint64_t linking_mask = 0xFF81BDA5A5BD81FF;

  const uint32_t level1 = 0xe400;
  const uint32_t level2 = 0xffff00;
  const uint32_t level3 = 0xff7e00;
  const uint32_t level4 = 0xff0000;
  const uint32_t level5 = 0x8f3f97;
  const uint32_t level6 = 0x7e0023;

  const uint32_t temp1 = 0x11f4f7;
  const uint32_t temp2 = 0x4df211;
  const uint32_t temp3 = 0xbf040d;

  uint32_t co2Color = 1;
  uint32_t AQIColor = 1;
  uint32_t tempColor = 0x6a0000;

  Adafruit_NeoMatrix *matrix;

  bool wifiMode;
  bool linkingMode;

  void applyMasks(void);

 public:
  Neomatrix(uint8_t pin);
  ~Neomatrix();
  void initialize(void);
  void setAQI(uint16_t value);
  void setWAQI(uint16_t value);
  void setHumidity(uint8_t value);
  void setTemp(float value);
  void setWifi(void);
  void disableWifi(void);
  void setLinkingMode(void);
  void disableLinkingMode(void);
};

#endif

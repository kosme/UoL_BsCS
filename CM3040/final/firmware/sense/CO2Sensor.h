#ifndef CO2_SENSOR_WRAPPER_H
#define CO2_SENSOR_WRAPPER_H

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Adafruit_CCS811.h>

class CO2Sensor {
 private:
  Adafruit_CCS811 _ccs;
  bool _initialized;
  uint8_t _scl;
  uint8_t _sda;

 public:
  enum sampling_modes_t {
    SAMPLING_IDLE = 0,
    SAMPLING_1_SEC,
    SAMPLING_10_SEC,
    SAMPLING_60_SEC,
    SAMPLING_250_MS
  };
  CO2Sensor(uint8_t sda, uint8_t scl);
  ~CO2Sensor();
  bool initialize(void);
  bool isInitialized(void);
  bool isAvailable(void);
  bool read(uint16_t* eCO2, uint16_t* VOC);
  void setSamplingInterval(sampling_modes_t mode);
  void compensateEnvironment(float humidity, float temperature);
};

#endif

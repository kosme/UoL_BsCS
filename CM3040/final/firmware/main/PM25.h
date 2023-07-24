#ifndef PM25_SENSOR_WRAPPER_H
#define PM25_SENSOR_WRAPPER_H

#include "Adafruit_PM25AQI.h"

// This PM2.5 sensor is UART only, so software serial must be used to allow
// debugging and regular programming
// rx is IN from sensor (TX pin on sensor), leave pin tx disconnected
#include <SoftwareSerial.h>

#define BP_INDEX 0
#define I_INDEX 1

class PM25Sensor {
 private:
  /* data */
  SoftwareSerial* pmSerial;

  Adafruit_PM25AQI* _aqi;
  uint8_t sleep_pin;
  bool _initialized;

  const float maxes[7][2] = {{12.0, 50.0},   {35.4, 100.0},  {55.4, 150.0},
                             {150.4, 200.0}, {250.4, 300.0}, {350.4, 400.0},
                             {500.4, 500.0}};

 public:
  PM25Sensor(uint8_t rx, uint8_t tx, uint8_t sleep);
  ~PM25Sensor();
  bool initialize(void);
  bool isInitialized(void);
  void sleep(void);
  void wakeup(void);
  bool read(uint16_t* data);
  bool readAQI(uint16_t* aqi);
};

#endif

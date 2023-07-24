#include "PM25.h"

PM25Sensor::PM25Sensor(uint8_t rx, uint8_t tx, uint8_t sleep)
    : sleep_pin(sleep) {
  pmSerial = new SoftwareSerial(rx, tx);
  _aqi = new Adafruit_PM25AQI();
}

PM25Sensor::~PM25Sensor() {
  pmSerial->~SoftwareSerial();
  _initialized = false;
}

bool PM25Sensor::isInitialized(void) { return _initialized; }

// Initialize a SoftSerial part and connect it to the Particulate Matter sensor
bool PM25Sensor::initialize(void) {
  uint8_t count = 10;
  pinMode(sleep_pin, OUTPUT);
  digitalWrite(sleep_pin, HIGH);
  // Wait for value to stabilize and sensor to wake-up
  delay(25);
  pmSerial->begin(9600);
  // connect to the sensor over software serial
  if (!_aqi->begin_UART(pmSerial) && (--count)) {
    Serial.println("Could not find PM 2.5 sensor!");
    delay(500);
  } else {
    Serial.println("PM25 found!");
  }
  _initialized = (count > 0);
  return _initialized;
}

// Convert a raw count of PM2.5 into an Air Quality Index value according to the
// formula on pages 13-14 of
// https://www.airnow.gov/sites/default/files/2020-05/aqi-technical-assistance-document-sept2018.pdf
bool PM25Sensor::readAQI(uint16_t* aqi) {
  PM25_AQI_Data reading;
  uint16_t rawData;
  if (!_aqi->read(&reading)) {
    Serial.println("Could not read from PM Sensor");
    return false;
  }
  rawData = reading.pm25_env;
  float minI = 0.0;
  float minBP = 0.0;
  for (uint8_t i = 0; i < 7; i++) {
    if (rawData <= maxes[i][BP_INDEX]) {
      *aqi = round(((maxes[i][I_INDEX] - minI) / (maxes[i][BP_INDEX] - minBP) *
                    ((float)rawData - minBP)) +
                   minI);
      break;
    } else {
      minBP = maxes[i][BP_INDEX] + 0.1;
      minI = maxes[i][I_INDEX] + 1;
    }
  }
  return true;
}

void PM25Sensor::sleep(void) { digitalWrite(sleep_pin, LOW); }

void PM25Sensor::wakeup(void) { digitalWrite(sleep_pin, HIGH); }

// Reads the complete bulk of data from the Particulate Matter sensor and only
// returns the raw count of PM2.5
bool PM25Sensor::read(uint16_t* data) {
  PM25_AQI_Data reading;
  if (!_aqi->read(&reading)) {
    Serial.println("Could not read from PM Sensor");
    return false;
  }
  *data = reading.pm25_env;
  return true;
}

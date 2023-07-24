#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(uint8_t sda, uint8_t scl) : _sda(sda), _scl(scl) {}

CO2Sensor::~CO2Sensor() {
  this->_ccs.~Adafruit_CCS811();
  this->_initialized = false;
}

bool CO2Sensor::initialize(void) {
  Wire.begin(this->_sda, this->_scl);
  this->_initialized = this->_ccs.begin(0x5A, &Wire);
  return _initialized;
}

bool CO2Sensor::isInitialized(void) { return this->_initialized; }

bool CO2Sensor::isAvailable(void) { return this->_ccs.available(); }

bool CO2Sensor::read(uint16_t* eCO2, uint16_t* VOC) {
  if (this->_ccs.available()) {
    if (!this->_ccs.readData()) {
      if (this->_ccs.geteCO2() == 0) {
        // The minimum value of eCO2 should be 400 according to the datasheet.
        // Therefore, a value of 0 is considered an error
        return false;
      }
      *eCO2 = this->_ccs.geteCO2();
      *VOC = this->_ccs.getTVOC();
      return true;
    }
  }
  return false;
}

void CO2Sensor::setSamplingInterval(sampling_modes_t mode) {
  this->_ccs.setDriveMode(mode);
}

void CO2Sensor::compensateEnvironment(float humidity, float temperature) {
  this->_ccs.setEnvironmentalData(humidity, temperature);
}

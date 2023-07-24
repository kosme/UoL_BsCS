#include "Neomatrix.h"

Neomatrix::Neomatrix(uint8_t pin) {
  // MATRIX DECLARATION:
  // Parameter 1 = width of NeoPixel matrix
  // Parameter 2 = height of matrix
  // Parameter 3 = pin number (most are valid)
  // Parameter 4 = matrix layout flags, add together as needed
  matrix = new Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, pin,
                                  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                      NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                  NEO_GRB + NEO_KHZ800);
  wifiMode = false;
  linkingMode = false;
}

Neomatrix::~Neomatrix() { matrix->~Adafruit_NeoMatrix(); }

void Neomatrix::initialize(void) {
  matrix->begin();
  matrix->setBrightness(5);
  matrix->fillScreen(0);
  matrix->show();
}

void Neomatrix::applyMasks(void) {
  matrix->fillScreen(0);
  for (uint8_t i = 0; i < 64; i++) {
    if (linkingMode) {
      if (bitRead(linking_mask, i)) {
        matrix->setPixelColor(i, 0, 200, 0);
      }
    } else {
      if (wifiMode) {
        if (bitRead(wifi_mask, i)) {
          matrix->setPixelColor(i, 0, 0, 200);
        }
      } else {
        if (bitRead(waqi_mask, i)) {
          matrix->setPixelColor(i, AQIColor);
        }
      }
      if (bitRead(aqi_mask, i)) {
        matrix->setPixelColor(i, co2Color);
      }
      if (bitRead(temp_mask, i)) {
        matrix->setPixelColor(i, tempColor);
      }
      if (bitRead(humid_mask, i)) {
        matrix->setPixelColor(i, 0, 0, 200);
      }
    }
  }
  matrix->show();
}

void Neomatrix::setAQI(uint16_t value) {
  if (value <= 50) {
    co2Color = level1;
  } else if (value <= 100) {
    co2Color = level2;
  } else if (value <= 150) {
    co2Color = level3;
  } else if (value <= 200) {
    co2Color = level4;
  } else if (value <= 300) {
    co2Color = level5;
  } else {
    co2Color = level6;
  }
  applyMasks();
}

void Neomatrix::setWAQI(uint16_t value) {
  if (value <= 50) {
    AQIColor = level1;
  } else if (value <= 100) {
    AQIColor = level2;
  } else if (value <= 150) {
    AQIColor = level3;
  } else if (value <= 200) {
    AQIColor = level4;
  } else if (value <= 300) {
    AQIColor = level5;
  } else {
    AQIColor = level6;
  }
  applyMasks();
}

void Neomatrix::setHumidity(uint8_t value) {
  float humidity = (float)value;
  humid_mask = 0x3000000000000000;
  for (uint8_t i = 0; i < MATRIX_HEIGHT; i++) {
    humidity -= 12.5;
    if (humidity > 0) {
      humid_mask = humid_mask >> 8;
      humid_mask += 0x3000000000000000;
    } else {
      break;
    }
  }
  applyMasks();
}

void Neomatrix::setTemp(float value) {
  float factor;
  if (value <= 18) {
    tempColor = temp1;
    factor = 18 / MATRIX_HEIGHT;
  } else if (value <= 30) {
    tempColor = temp2;
    factor = 12 / MATRIX_HEIGHT;
    value -= 18;
  } else {
    tempColor = temp3;
    factor = 20 / MATRIX_HEIGHT;
    value -= 30;
  }
  temp_mask = 0xC000000000000000;
  for (uint8_t i = 0; i < MATRIX_HEIGHT; i++) {
    value -= factor;
    if (value > 0) {
      temp_mask = temp_mask >> 8;
      temp_mask += 0xC000000000000000;
    } else {
      break;
    }
  }
  applyMasks();
}

void Neomatrix::setWifi(void) {
  wifiMode = true;
  wifi_mask = wifi_mask ^ 0x0f0f0f0f;
  applyMasks();
}

void Neomatrix::disableWifi(void) {
  wifiMode = false;
  wifi_mask = 0x0F09090F;
  applyMasks();
}

void Neomatrix::setLinkingMode(void) {
  linkingMode = true;
  linking_mask = linking_mask ^ 0xFFFFFFFFFFFFFFFF;
  applyMasks();
}

void Neomatrix::disableLinkingMode(void) {
  linkingMode = false;
  linking_mask = 0xFF81BDA5A5BD81FF;
  applyMasks();
}

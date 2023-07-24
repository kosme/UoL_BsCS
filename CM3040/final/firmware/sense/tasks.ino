#include "CO2Sensor.h"

// Read the CO2 sensor
void readAQI(void) {
  if (!co2Sensor.read(&eCO2, &VOC)) {
    Serial.println("Error reading CCS811");
    task_co2.restartDelayed(5 * TASK_SECOND);
  } else {
    if ((VOC > 0 || eCO2 > 400) &&
        (task_co2.getInterval() == 5 * TASK_SECOND)) {
      co2Sensor.setSamplingInterval(CO2Sensor::SAMPLING_10_SEC);
      sampleCO2EveryMinute();
    }
    Serial.print("CO2: ");
    Serial.print(eCO2);
    Serial.print("ppm, TVOC: ");
    Serial.println(VOC);
    if (eCO2 >= 600) {
      task_blinkCO2.enable();
      task_blinkCO2.forceNextIteration();
    } else {
      task_blinkCO2.disable();
    }
  }
}

// Blink the blue led to show the device is in "sync mode"
void blinkWifi(void) {
  if (task_blinkWifi.isFirstIteration()) {
    blueLedState = false;
  }
  digitalWrite(RGB_LED_B, blueLedState);
  blueLedState = !blueLedState;

  if (task_blinkWifi.isLastIteration()) {
    digitalWrite(RGB_LED_B, LOW);
  }
}

// Blink the red led to signal dangerous concentrations of CO2
void blinkCO2(void) {
  if (task_blinkCO2.isFirstIteration()) {
    redLedState = false;
  }
  digitalWrite(RGB_LED_R, redLedState);
  redLedState = !redLedState;
}

void compensateCCS(void) {
  // Compensate the environmental conditions
  co2Sensor.compensateEnvironment(dht11.humidity, dht11.temperature);
}

void readDHT(void) {
  dht11.humidity = dht.readHumidity();
  // Read temperature as Celsius
  dht11.temperature = dht.readTemperature();

  Serial.print(F("Humidity: "));
  Serial.print(dht11.humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(dht11.temperature);
  Serial.println(F("°C "));
  if (task_readDHT.isFirstIteration()) {
    task_compensateEnvironment.enableIfNot();
  }
}

void sampleCO2EveryMinute(void) {
  task_co2.setInterval(1 * TASK_MINUTE);
  digitalWrite(RGB_LED_R, LOW);
}

bool sampleCO2Every30Seconds(void) {
  task_co2.setInterval(30 * TASK_SECOND);
  return true;
}

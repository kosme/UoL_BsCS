#include "Neomatrix.h"
#include "PM25.h"

// Remove a sensor node from the list of registered devices
void clearRemoteNodeConfig(void) {
  if (deleteNode(nodeToDelete)) {
    nodeToDelete = -1;
  }
}

void readRemoteSensors(void) {
  Serial.println("Reading remote");
  if (!getAllNodesData()) {
    task_readRemote.delay(5 * TASK_SECOND);
  } else {
    fillSystemValues(true);
  }
}

// Prepare the PM2.5 sensor for taking a reading.
// The datasheet recommends waiting 30 seconds after wakeup before a measurement
// is taken so a callback for doing the actual reading is used
void preparePMSensor(void) {
  pmSensor.wakeup();
  if (!task_readPM.enableIfNot()) {
    task_readPM.delay(30 * TASK_SECOND);
  }
  Serial.println("Sensor enabled");
}

// Perform an actual PM2.5 reading
void readParticles(void) {
  if (pmSensor.readAQI(&aqi_value)) {
    display.setAQI(aqi_value);
    pmSensor.sleep();
    Serial.println("Sensor disabled");
    task_readPM.disable();
  } else {
    Serial.println("Reading failed. Retrying");
    task_readPM.delay(5 * TASK_SECOND);
  }
}

// Turn off the AP used for adding new sensor nodes
void turnAPOff(void) {
  Serial.println("Turning AP off");
  WiFi.softAPdisconnect(true);
  task_blinkLinking.disable();
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

  fillSystemValues(false);
}

// Fill the structure that holds the maximum values from all the system sensors
// on each of the measured categories
void fillSystemValues(bool externalSensor) {
  systemWideMaximums.temperature = dht11.temperature;
  systemWideMaximums.humidity = dht11.humidity;
  if (externalSensor) {
    systemWideMaximums.co2 = 0;
  }
  for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
    if (strlen(nodeNames[i])) {
      if (nodesData[i].temperature > systemWideMaximums.temperature) {
        systemWideMaximums.temperature = nodesData[i].temperature;
      }
      if (nodesData[i].humidity > systemWideMaximums.humidity) {
        systemWideMaximums.humidity = nodesData[i].humidity;
      }
      if (externalSensor) {
        if (nodesData[i].co2 > systemWideMaximums.co2) {
          systemWideMaximums.co2 = nodesData[i].co2;
        }
      }
    }
  }
  display.setTemp(systemWideMaximums.temperature);
  display.setHumidity((uint8_t)systemWideMaximums.humidity);
}

// Fetch the data from the metropolitan air quality reading
void getWaqi(void) {
  int reading = getAirQualityReading();
  // Negative values are used for errors
  if (reading >= 0) {
    waqi = reading;
    display.setWAQI((uint16_t)reading);
  }
}

void blinkWifi(void) { display.setWifi(); }

void stopWifiBlink(void) { display.disableWifi(); }

void blinkLinking(void) { display.setLinkingMode(); }

void stopLinkingBlink(void) { display.disableLinkingMode(); }

// This function only returns true
// It allows skipping the onEnableCallback parameter on tasks
bool dummyFunction(void) { return true; }

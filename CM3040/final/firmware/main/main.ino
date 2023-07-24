/* Required external libraries and platform versions
Used library             Version Source
Adafruit_BusIO           1.13.2  Arduino Library Manager
Adafruit_GFX_Library     1.11.3  Arduino Library Manager
Adafruit_NeoMatrix       1.3.0   Arduino Library Manager
Adafruit_NeoPixel        1.10.5  Arduino Library Manager
Adafruit_PM25_AQI_Sensor 1.0.6   Arduino Library Manager
Adafruit_Unified_Sensor  1.1.6   Arduino Library Manager
ArduinoJson              6.19.4  Arduino Library Manager
ArduinoOTA               1.0     Included with ESP8266 platform files
DHT_sensor_library       1.4.4   Arduino Library Manager
EEPROM                   1.0     Included with ESP8266 platform files
ESP8266HTTPClient        1.2     Included with ESP8266 platform files
ESP8266mDNS              1.2     Included with ESP8266 platform files
ESP8266WiFi              1.0     Included with ESP8266 platform files
ESPAsyncTCP-master       1.2.2   https://github.com/me-no-dev/ESPAsyncTCP
ESPAsyncWebServer-master 1.2.3   https://github.com/me-no-dev/ESPAsyncWebServer
Hash                     1.0     Included with ESP8266 platform files
LittleFS                 0.1.0   Included with ESP8266 platform files
SoftwareSerial           6.12.7  Included with ESP8266 platform files
SPI                      1.0     Included with ESP8266 platform files
StreamUtils              1.6.3   Arduino Library Manager
TaskScheduler            3.6.0   Arduino Library Manager
Wire                     1.0     Included with ESP8266 platform files

Used platform   Version
esp8266:esp8266 3.0.2
*/

#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <TaskScheduler.h>

#include "DHT.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include "Neomatrix.h"
#include "PM25.h"

#define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between
                                 // tasks if no callback methods were invoked
                                 // during the pass

struct networkConfig {
  char ssid[64];
  char pass[64];
  char alias[64];
} tempConfig;

struct dht11Reading {
  float temperature;
  float humidity;
} dht11;

struct maximumValues {
  float temperature;
  float humidity;
  uint16_t co2;
} systemWideMaximums;

#define MAX_NODES_NUMBER 3

struct remoteSensorData {
  char alias[64];
  uint16_t co2;
  uint16_t voc;
  float temperature;
  float humidity;
} nodesData[MAX_NODES_NUMBER];

char nodeNames[MAX_NODES_NUMBER][64];

// struct keyPair {
//   uint8_t pub[32];
//   uint8_t pk[32];
// } keys;

bool networkConfigured = false;
bool shouldReboot = false;
bool attemptConnection = false;

AsyncWebServer server(80);

Scheduler ts;
uint16_t waqi;
uint16_t aqi_value;

DHT dht(14, DHT11);

uint8_t nodeToDelete;

Neomatrix display(5);
PM25Sensor pmSensor(4, 11, 12);

// function prototypes required for creating tasks
void turnAPOff(void);
void getWaqi(void);
void readDHT(void);
void preparePMSensor(void);
void readParticles(void);
void readRemoteSensors(void);
void clearRemoteNodeConfig(void);
void blinkWifi(void);
void stopWifiBlink(void);
void blinkLinking(void);
void stopLinkingBlink(void);
bool dummyFunction(void);
Task task_APOff(5 * TASK_SECOND, 1, &turnAPOff, &ts, false);
Task task_readWaqi(15 * TASK_MINUTE, TASK_FOREVER, &getWaqi, &ts, false);
Task task_readDHT(5 * TASK_MINUTE, TASK_FOREVER, &readDHT, &ts, true);
Task task_preparePMSensor(10 * TASK_MINUTE, TASK_FOREVER, &preparePMSensor, &ts,
                          false);
Task task_readPM(0, TASK_FOREVER, &readParticles, &ts, false);
Task task_readRemote(1 * TASK_MINUTE, TASK_FOREVER, &readRemoteSensors, &ts,
                     false);
Task task_deleteNode(5 * TASK_SECOND, 1, &clearRemoteNodeConfig, &ts, false);
Task task_blinkWifi(0.5 * TASK_SECOND, TASK_FOREVER, &blinkWifi, &ts, true,
                    dummyFunction, stopWifiBlink);
Task task_blinkLinking(0.5 * TASK_SECOND, TASK_FOREVER, &blinkLinking, &ts,
                       false, dummyFunction, stopLinkingBlink);

void setup() {
  createWaqiFilter();
  dht.begin();
  Serial.begin(115200);
  delay(100);
  display.initialize();
  if (pmSensor.initialize()) {
    pmSensor.sleep();
    // Take first reading
    task_preparePMSensor.enable();
    task_preparePMSensor.forceNextIteration();
  }
  Serial.println("");
  Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  readNodesFile();
  networkConfig network;
  networkConfigured = loadConfig(&network);
  // Blink to show it is connecting to the network or in AP mode
  task_blinkWifi.enable();
  if (!networkConfigured) {
    // configure device as AP
    configureAP();
  } else {
    // Try to connect
    if (connectToNetwork(&network, 10)) {
      task_blinkWifi.disable();
      // configure device as station
      configureStation();
      // Create API endpoints
      configureApi();
      // Initialize OTA updates handler
      configureOTA("aq-monitor-main");
      if (!task_readWaqi.enableIfNot()) {
        task_readWaqi.forceNextIteration();
      }
      task_readRemote.enable();
      task_readRemote.forceNextIteration();
      task_APOff.enableIfNot();
    } else {
      // configure device as AP
      configureAP();
    }
  }
  // Start server
  server.begin();
  readDHT();
}

void loop() {
  if (shouldReboot) {
    LittleFS.end();
    delay(500);
    ESP.restart();
  }
  if (attemptConnection) {
    attemptConnection = false;
    if (connectToNetwork(&tempConfig, 10)) {
      saveConfig(&tempConfig);
      Serial.println("Successful credential setting");
      shouldReboot = true;
    } else {
      Serial.println("Bad credentials received");
    }
  }
  ArduinoOTA.handle();
  ts.execute();
}

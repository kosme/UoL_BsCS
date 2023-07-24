/* Required external libraries and platform versions
Used library             Version Source
Adafruit_BusIO           1.13.2  Arduino Library Manager
Adafruit_CCS811_Library  1.1.1   Arduino Library Manager
Adafruit_Unified_Sensor  1.1.6   Arduino Library Manager
ArduinoJson              6.19.4  Arduino Library Manager
ArduinoOTA               1.0     Included with ESP8266 platform files
DHT_sensor_library       1.4.4   Arduino Library Manager
ESP8266HTTPClient        1.2     Included with ESP8266 platform files
ESP8266mDNS              1.2     Included with ESP8266 platform files
ESP8266WiFi              1.0     Included with ESP8266 platform files
ESPAsyncTCP-master       1.2.2   https://github.com/me-no-dev/ESPAsyncTCP
ESPAsyncWebServer-master 1.2.3   https://github.com/me-no-dev/ESPAsyncWebServer
Hash                     1.0     Included with ESP8266 platform files
LittleFS                 0.1.0   Included with ESP8266 platform files
SPI                      1.0     Included with ESP8266 platform files
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

#include "CO2Sensor.h"
#include "DHT.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"

#define _TASK_SLEEP_ON_IDLE_RUN  // Enable 1 ms SLEEP_IDLE powerdowns between
                                 // tasks if no callback methods were invoked
                                 // during the pass

CO2Sensor co2Sensor(12, 14);
uint16_t eCO2;
uint16_t VOC;

#define RGB_LED_R 4
#define RGB_LED_G 2
#define RGB_LED_B 5

struct networkConfig {
  char ssid[64];
  char pass[64];
  char alias[64];
} tempConfig;

struct dht11Reading {
  float temperature;
  float humidity;
} dht11;

DHT dht(13, DHT11);

bool configured = false;
bool shouldReboot = false;
bool attemptConnection = false;

AsyncWebServer server(80);

char hostname[17];

Scheduler ts;

// function prototypes required for creating tasks
void blinkWifi(void);
void blinkCO2(void);
void readAQI(void);
void readDHT(void);
void compensateCCS(void);
void sampleCO2EveryMinute(void);
bool sampleCO2Every30Seconds(void);
Task task_blinkWifi(0.1 * TASK_SECOND, TASK_FOREVER, &blinkWifi, &ts, false);
Task task_blinkCO2(0.75 * TASK_SECOND, TASK_FOREVER, &blinkCO2, &ts, false,
                   sampleCO2Every30Seconds, sampleCO2EveryMinute);
Task task_co2(5 * TASK_SECOND, TASK_FOREVER, &readAQI, &ts, false);
Task task_readDHT(5 * TASK_MINUTE, TASK_FOREVER, &readDHT, &ts, false);
Task task_compensateEnvironment(15 * TASK_MINUTE, TASK_FOREVER, &compensateCCS,
                                &ts, false);
bool blueLedState;
bool redLedState;

void setup() {
  pinMode(RGB_LED_R, OUTPUT);
  pinMode(RGB_LED_G, OUTPUT);
  pinMode(RGB_LED_B, OUTPUT);
  digitalWrite(RGB_LED_R, LOW);
  digitalWrite(RGB_LED_G, LOW);
  digitalWrite(RGB_LED_B, LOW);
  Serial.begin(115200);
  dht.begin();
  task_readDHT.enable();
  delay(100);
  if (co2Sensor.initialize()) {
    task_co2.enableDelayed(5 * TASK_SECOND);
  } else {
    Serial.println("Failed to start sensor! Please check your wiring.");
  }
  Serial.println("\nMounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  networkConfig network;
  configured = loadConfig(&network);
  // Hostname defaults to aq-sensor-[ChipID]
  sprintf(hostname, "aq-sensor-%06x", ESP.getChipId());
  if (!configured) {
    // Try to connect to the main device's AP
    strlcpy(network.ssid, "IoT-Main-AP", sizeof(network.ssid));
    strlcpy(network.pass, "BwpX56FMI$#!IgGq", sizeof(network.pass));
    // Blink led to show device is in "sync mode"
    task_blinkWifi.enable();
    if (connectToNetwork(&network, 60)) {
      // request "/netconfig" from main device
      shouldReboot = fetchCredentials(&network);
    } else {
      WiFi.disconnect();
      task_blinkWifi.disable();
    }
  } else {
    // Try to connect with stored network credentials
    if (connectToNetwork(&network, 10)) {
      // configure device as station
      configureStation();
      // Create API endpoints
      configureApi();
      // Initialize OTA updates handler
      configureOTA(hostname);
    }
  }
  // Start server
  server.begin();
  // Copy current network config to RAM
  loadConfig(&tempConfig);
}

void loop() {
  if (shouldReboot) {
    LittleFS.end();
    delay(500);
    ESP.restart();
  }
  ArduinoOTA.handle();
  ts.execute();
}

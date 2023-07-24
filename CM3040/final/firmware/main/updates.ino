#include <ArduinoOTA.h>
#include <LittleFS.h>

#include "FS.h"

void configureOTA(char* hostname) {
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    // Stop server
    server.end();
    // Stop all scheduled tasks
    ts.disableAll();
    // Avoid network configuration from being overwritten on OTA FS update
    if (ArduinoOTA.getCommand() == U_FS) {
      ArduinoOTA.setRebootOnSuccess(false);
      // Load network config to RAM
      loadConfig(&tempConfig);
      LittleFS.end();
    }
    Serial.println("Starting update");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    if (ArduinoOTA.getCommand() == U_FS) {
      // Save in FS the network config and nodes data stored in RAM
      if (LittleFS.begin()) {
        saveConfig(&tempConfig);
        saveNodes();
      }
      shouldReboot = true;
    }
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    shouldReboot = true;
  });
  ArduinoOTA.begin();
}

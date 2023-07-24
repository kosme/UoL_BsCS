#include <ArduinoJson.h>
#include <LittleFS.h>

#include "FS.h"

#define WIFI_JSON_DOC_SIZE 200
#define NET_CONFIG_FILE "/netConfig.json"

// The existence of this file is vital for the correct operation of the sketch
// It should have been uploaded with the FS
// If it is missing, create a blank copy of it
void createConfigFile(void) {
  // Double checking if the file exists as it will be truncated
  if (!LittleFS.exists(NET_CONFIG_FILE)) {
    // Create file, write basic structure, and close it immediately
    File configFile = LittleFS.open(NET_CONFIG_FILE, "w");
    configFile.write("{\"SSID\": \"\", \"PASS\": \"\", \"alias\": \"\"}");
    configFile.close();
  }
}

// Read network configuration from the filesystem and parse it
bool loadConfig(networkConfig* config) {
  if (!LittleFS.exists(NET_CONFIG_FILE)) {
    createConfigFile();
    return false;
  }
  File configFile = LittleFS.open(NET_CONFIG_FILE, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<WIFI_JSON_DOC_SIZE> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  if (strlen(doc["alias"]) > 0) {
    strlcpy(config->alias, doc["alias"], sizeof(config->alias));
  } else {
    strlcpy(config->alias, hostname, sizeof(config->alias));
  }

  if (strlen(doc["SSID"]) == 0) {
    return false;
  }

  // Make sure to avoid buffer overflows by using safe copy functions
  strlcpy(config->ssid, doc["SSID"], sizeof(config->ssid));
  strlcpy(config->pass, doc["PASS"], sizeof(config->pass));
  return true;
}

// Replace the network configuration stored on the filesystem with a copy of the
// current settings
bool saveConfig(networkConfig* config) {
  StaticJsonDocument<WIFI_JSON_DOC_SIZE> doc;
  doc["SSID"] = config->ssid;
  doc["PASS"] = config->pass;
  doc["alias"] = config->alias;

  File configFile = LittleFS.open(NET_CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}

// Delete the stored network credentials
bool clearConfig(void) {
  Serial.println("Clearing config");
  networkConfig config;
  strlcpy(config.ssid, "", sizeof(config.ssid));
  strlcpy(config.pass, "", sizeof(config.pass));
  return saveConfig(&config);
}

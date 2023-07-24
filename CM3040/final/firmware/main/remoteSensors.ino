#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "FS.h"

#define NODES_FILE "nodes.json"
#ifndef MAX_NODES_NUMBER
#define MAX_NODES_NUMBER 3
#endif

// The existence of this file is required for the correct operation of the
// sketch. It should have been uploaded with the FS but if it is missing, create
// a blank copy of it
void createNodesFile(void) {
  // Double checking if the file exists as it will be truncated
  if (!LittleFS.exists(NODES_FILE)) {
    // Create file, write basic structure, and close it immediately
    File nodesFile = LittleFS.open(NODES_FILE, "w");
    nodesFile.write("[]");
    nodesFile.close();
  }
}

// Load from the filesystem the data regarding the registered sensor nodes
bool readNodesFile(void) {
  if (!LittleFS.exists(NODES_FILE)) {
    createNodesFile();
    return false;
  }
  File nodesFile = LittleFS.open(NODES_FILE, "r");
  if (!nodesFile) {
    Serial.println("Failed to open nodes file");
    return false;
  }
  size_t size = nodesFile.size();
  if (size > 1024) {
    Serial.println("Nodes file size is too large");
    return false;
  }
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  nodesFile.readBytes(buf.get(), size);

  const size_t MAX_NODES = JSON_ARRAY_SIZE(MAX_NODES_NUMBER);
  StaticJsonDocument<MAX_NODES> doc;
  auto error = deserializeJson(doc, buf.get());

  if (error) {
    Serial.println("Failed to parse nodes file");
    return false;
  }
  JsonArray nodesArr = doc.as<JsonArray>();
  uint8_t position = 0;
  for (JsonVariant v : nodesArr) {
    strlcpy(nodeNames[position], v.as<char *>(), sizeof(nodeNames[position]));
    position++;
  }
  return true;
}

bool saveNodes(void) {
  Serial.println("Saving nodes");
  File nodesFile = LittleFS.open(NODES_FILE, "w");
  if (!nodesFile) {
    Serial.println("Failed to open nodes file for writing");
    return false;
  }
  nodesFile.write("[");
  for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
    if (strlen(nodeNames[i])) {
      if (i) {
        nodesFile.write(",");
      }
      nodesFile.write("\"");
      nodesFile.write(nodeNames[i], strlen(nodeNames[i]));
      nodesFile.write("\"");
    }
  }
  nodesFile.write("]");
  nodesFile.close();
  return true;
}

// Remove a registered node from the memory structure and store the changes on
// the filesystem. It makes sure that there are no gaps inside the memory
// structure
bool deleteNode(uint8_t position) {
  if (position < MAX_NODES_NUMBER) {
    if (strlen(nodeNames[position])) {
      if (!clearNodeNetwork(nodeNames[position])) {
        return false;
      }
      // Remove the last element by clearing it
      if (position == (MAX_NODES_NUMBER - 1)) {
        strlcpy(nodeNames[position], "", sizeof(nodeNames[position]));
      } else {
        for (uint8_t i = position; i < MAX_NODES_NUMBER - 1; i++) {
          strlcpy(nodeNames[i], nodeNames[i + 1], sizeof(nodeNames[i]));
        }
      }
      return saveNodes();
    }
  }
  return false;
}

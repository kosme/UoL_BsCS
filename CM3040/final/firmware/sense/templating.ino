#include <ESP8266WiFi.h>

// Replace placeholders on the html files
String processor(const String& var) {
  if (var == "ALIAS") {
    return tempConfig.alias;
  }
  if (var == "TEMP") {
    return String(dht11.temperature, 1);
  }
  if (var == "HUMID") {
    return String(dht11.humidity, 0);
  }
  if (var == "CO2") {
    return String(eCO2);
  }
  if (var == "V0C") {
    return String(VOC);
  }
}

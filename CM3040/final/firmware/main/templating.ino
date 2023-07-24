#include <Arduino.h>

#ifndef MAX_NODES_NUMBER
#define MAX_NODES_NUMBER 3
#endif

// Replace placeholders on the html files
String processor(const String &var) {
  if (var == "NETWORKS") {
    return scanAvailableNetworks(true);
  }
  if (var == "ALIAS") {
    return String(tempConfig.alias);
  }
  if (var == "WAQI") {
    return String(waqi);
  }
  if (var == "TEMP") {
    return String(dht11.temperature, 1);
  }
  if (var == "HUMID") {
    return String(dht11.humidity, 0);
  }
  if (var == "AQI") {
    return String(aqi_value);
  }
  if (var == "SENSORS") {
    return showNodesAsList();
  }
  if (var == "NODESDATA") {
    return showNodesData();
  }
  if (var == "MAINDATA") {
    return showSystemWideData();
  }
}

// Generate HTML structure to display the general status of the system. This
// must be performed on the sketch code because the Server library does not
// support loops and logic during templating.
String showSystemWideData(void) {
  String ans = "<div class=\"inner-element house\">";
  if (waqi > aqi_value) {
    ans.concat(waqi);
  } else {
    ans.concat(aqi_value);
  }
  ans.concat("</div><div class=\"inner-element co2\">");
  ans.concat(systemWideMaximums.co2);
  ans.concat("</div><div class=\"inner-element temp\">");
  ans.concat(String(systemWideMaximums.temperature, 1));
  ans.concat("</div><div class=\"inner-element humidity\">");
  ans.concat(String(systemWideMaximums.humidity, 0));
  ans.concat("</div>");
  return ans;
}

// Generate HTML structures to display the readings of all the registered sensor
// nodes. This must be performed on the sketch code because the Server library
// does not support loops and logic during templating.
String showNodesData(void) {
  String answer;
  for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
    // If the name has length zero, there is no node registered at that position
    if (strlen(nodeNames[i])) {
      answer.concat("<div class=\"inner-display dashboard-element\">");
      answer.concat("<div class=\"dashboard-element-name\">");
      answer.concat(nodesData[i].alias);
      answer.concat("</div>");
      answer.concat("<div class=\"inner-element co2\">");
      answer.concat(nodesData[i].co2);
      answer.concat("</div><div class=\"inner-element voc\">");
      answer.concat(nodesData[i].voc);
      answer.concat("</div><div class=\"inner-element temp\">");
      answer.concat(String(nodesData[i].temperature, 1));
      answer.concat("</div><div class=\"inner-element humidity\">");
      answer.concat(String(nodesData[i].humidity, 0));
      answer.concat("</div></div>");
    }
  }
  return answer;
}

// Generate an HTML table containing the network names and urls of all the
// registered sensor nodes
String showNodesAsList(void) {
  String nodes = "";

  for (uint8_t i = 0; i < MAX_NODES_NUMBER; i++) {
    if (strlen(nodeNames[i])) {
      nodes.concat("<tr><td><a href=\"http://");
      nodes.concat(nodeNames[i]);
      nodes.concat(".local\">");
      nodes.concat(nodeNames[i]);
      nodes.concat("</a></td><td class=\"deleteButton\"id=\"");
      nodes.concat(i);
      nodes.concat("\"></td></tr>");
    }
  }
  return nodes;
}

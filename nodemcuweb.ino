#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "index.h"
 
const char* ssid = "<YOUR SSID HERE";
const char* password = "<YOUR PASSPHRASE HERE>";
int button1State;
int button2State;
 
ESP8266WebServer server(80);

// Serving Hello world
/*
void getHelloWord() {
      DynamicJsonDocument doc(512);
      doc["name"] = "Hello world";
      Serial.println(F("Streaming Hello World..."));
      String buf;
      serializeJson(doc, buf);
      server.send(200, "application/json", buf);
      Serial.println(F("done."));
*/
//serve index.h at root level
void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}
//Read garage door reed relay
void sensor_data() { 
  String sensor_value = "";
  int a = digitalRead(5);
  if (a == 0) {
    sensor_value = "CLOSED"; 
  }
  else {
    sensor_value = "OPEN";
  }
  server.send(200, "text/plane", sensor_value);
}
//toggle garage door button
void door_control() {
  String state = "OFF";
  String act_state = server.arg("state");
  if(act_state == "1") {
    Serial.println("Button Pressed");
    digitalWrite(4, HIGH);
    delay(500); //
    digitalWrite(4, LOW);
    state = "ON";
  }
  else {
    state = "OFF";
  }
  server.send(200, "text/plane", state);
}

// Serving Button State
void getButton() {
      DynamicJsonDocument doc(512);
      if (button1state == 1) {
        doc["garagedoor"] = "open"
      }
      else {
        doc["garagedoor"] = "closed"
      }
      //doc["button2state"] = button2State;
      Serial.println(F("Streaming Button State..."));
      String buf;
      Serial.println(buf);
      serializeJson(doc, buf);
      server.send(200, "application/json", buf);
      Serial.println(F("done."));
}
// REST GET to Toggle Garage Door
void garageDoor() {
  DynamicJsonDocument doc(512);
  if (server.arg("toggle")== "true") {
    Serial.println("Button Pressed");
    digitalWrite(4, HIGH);
    delay(500); //
    digitalWrite(4, LOW);
  }
  doc["toggle"] = "true";
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
}
// Serving Settings
void getSettings() {
      DynamicJsonDocument doc(512);
 
      doc["ip"] = WiFi.localIP().toString();
      doc["gw"] = WiFi.gatewayIP().toString();
      doc["nm"] = WiFi.subnetMask().toString();
 
      if (server.arg("signalStrength")== "true"){
          doc["signalStrengh"] = WiFi.RSSI();
      }
 
      if (server.arg("chipInfo")== "true"){
          doc["chipId"] = ESP.getChipId();
          doc["flashChipId"] = ESP.getFlashChipId();
          doc["flashChipSize"] = ESP.getFlashChipSize();
          doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
      }
      if (server.arg("freeHeap")== "true"){
          doc["freeHeap"] = ESP.getFreeHeap();
      }
 
      Serial.println(F("Streaming Settings..."));
      String buf;
      serializeJson(doc, buf);
      server.send(200, F("application/json"), buf);
      Serial.println(F("done."));
}
 
// Define routing
void restServerRouting() {
    /*server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server"));
    });*/
    //server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on("/", handleRoot);
    server.on(F("/settings"), HTTP_GET, getSettings);
    server.on(F("/button"), HTTP_GET, getButton);
    server.on("/door_control", door_control);
    server.on("/doorread", sensor_data);
    server.on(F("/toggle"), HTTP_GET, garageDoor);
}
 
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
 
void setup(void) {
  //Configure Pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, INPUT);
  pinMode(10, INPUT);
  pinMode(4, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(4, LOW);
  
  //Configure Serial
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Activate mDNS this is used to be able to connect to the server with local DNS hostmane esp8266.local
  if (MDNS.begin("garage")) {
    Serial.println("MDNS responder started");
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void) {
  server.handleClient();
  button1State = digitalRead(5); // door reed sensor, 
  button2State = digitalRead(10); //Physical Button to trigger relay
  if (button2State == LOW)
  {
    digitalWrite(4, HIGH);
    delay(500); //make 500 in final code
    digitalWrite(4, LOW);
  }
  if (button1State == LOW)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

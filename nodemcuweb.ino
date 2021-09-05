#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "index.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
 
const char* ssid = 'YOUR SSID';
const char* password = 'YOUR PASSPHRASE';
int button1State;
Adafruit_BMP280 bmp;
ESP8266WebServer server(80);

int doorSensor = 9;
int doorRelay = 14;

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// DHT Sensor
//uint8_t DHTPin = D4; 
#define DHTPin 3     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
               
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

//DHT11
float dhtTemperature, dhtHumidity;

// BME_BMP
float temperature, pressure;

//serve index.h at root level
void handleRoot() {
  String s = webpage;
  server.send(200, F("text/html"), s);
}
//Read garage door reed relay for AJAX Web Page
void sensor_data() { 
  String sensor_value = "";
  int a = digitalRead(doorSensor);
  if (a == 0) {
    sensor_value = "CLOSED"; 
  }
  else {
    sensor_value = "OPEN";
  }
  server.send(200, F("text/plane"), sensor_value);
}
//toggle garage door button
void door_control() {
  String state = "OFF";
  String act_state = server.arg("state");
  if(act_state == "1") {
    Serial.println(F("Button Pressed"));
    digitalWrite(doorRelay, HIGH);
    delay(500); //
    digitalWrite(doorRelay, LOW);
    state = "ON";
  }
  else {
    state = "OFF";
  }
  server.send(200, F("text/plane"), state);
}

// REST GET Serving Button State
void getButton() {
      DynamicJsonDocument doc(512);
      if (button1State == 1) {
        doc["garagedoor"] = "open";
      }
      else {
        doc["garagedoor"] = "closed";
      }
      Serial.println(F("REST GET Streaming Button State..."));
      String buf;
      Serial.println(buf);
      serializeJson(doc, buf);
      server.send(200, F("application/json"), buf);
      Serial.println(F("done."));
}
// REST GET to Toggle Garage Door
void garageDoor() {
  DynamicJsonDocument doc(512);
  if (server.arg("toggle")== "true") {
    Serial.println(F("REST GET Button Pressed"));
    digitalWrite(doorRelay, HIGH);
    delay(500); //
    digitalWrite(doorRelay, LOW);
  }
  doc["toggle"] = "true";
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
}

// REST GET to read BME280
void weatherSensor() {
  //BME Read
  temperature = bmp.readTemperature();
  Serial.print(F("Temperature = "));
  Serial.print(temperature);
  Serial.println(" *C");
  
  pressure = bmp.readPressure();
  Serial.print(F("Pressure = "));
  Serial.print(pressure);
  Serial.println(" Pa");

  Serial.print(F("Temperature = "));
  float tempF = 0.0;
  tempF = ((bmp.readTemperature()*1.8)+32);    // (0°C × 9/5) + 32 = 32°F
  Serial.print(tempF);
  Serial.println(" *F");

  Serial.print(F("Pressure = "));
  float inHg = 0.0;
  inHg = bmp.readPressure()*0.00029530; // Pa * 0.00029530
  Serial.print(inHg);
  Serial.println(" in");

  //DHT Read
  dhtTemperature = dht.readTemperature(); // Gets the values of the temperature
  Serial.print(F("dhtTemperature = "));
  Serial.print(dhtTemperature);
  Serial.println(" *C");
  dhtHumidity = dht.readHumidity(); // Gets the values of the humidity 
  Serial.print(F("dhtHumidity = "));
  Serial.print(dhtHumidity);
  Serial.println(" *RH");
  
  DynamicJsonDocument doc(512);
  
  doc["tempC"] = temperature;
  doc["pressurePa"] = pressure;
  doc["tempF"] = tempF;
  doc["pressureIn"] = inHg;
  doc["dhtTempC"] = dhtTemperature;
  doc["dhtHumidityRH"] = dhtHumidity;

  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);

}

// Serving Settings
void getSettings() {
      //StaticJsonDocument<512> doc;
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
    server.on(F("/"), handleRoot);
    server.on(F("/settings"), HTTP_GET, getSettings); //Shows some of the basic ESP stats
    server.on(F("/button"), HTTP_GET, getButton); // Gets the status of the Garge Door Sensor
    server.on(F("/toggle"), HTTP_GET, garageDoor); // Allows you to toggle the garage door button
    server.on(F("/door_control"), door_control); // Control the garage door from the AJAX web page
    server.on(F("/doorread"), sensor_data); // Allows you to know the status of the garage door from the AJAX web page
    server.on(F("/weather"), HTTP_GET , weatherSensor); //Read data from a BME280 sensor
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
  server.send(404, F("text/plain"), message);
  Serial.println(F("HTTP Error 404"));
}
 
void setup(void) {
  //Configure Pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(doorSensor, INPUT); // Door Sensor IO
  pinMode(doorRelay, OUTPUT); // Door Relay IO
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(doorRelay, LOW);
  pinMode(DHTPin, INPUT);

  //Configure Serial
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

/* If a static IP is needed
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8); // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

// Configures static IP address
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  Serial.println("STA Failed to configure");
}
*/

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.print(F("Connected to "));
  Serial.println(ssid);
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.println(F("ESP Board MAC Address:  "));
  Serial.println(WiFi.macAddress());
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane
  if (MDNS.begin("garage")) {
    Serial.println(F("MDNS responder started"));
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println(F("HTTP server started"));

  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }
  
  dht.begin();  

} 
void loop(void) {
  server.handleClient();
  button1State = digitalRead(doorSensor); // door reed sensor, 

  if (button1State == LOW)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  
}

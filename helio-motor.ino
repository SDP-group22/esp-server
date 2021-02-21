#include <WiFi.h>
#include <ESPAsyncWebServer.h>;
#include <AsyncJson.h>
#include <ArduinoJson.h>

// Initialise variables
const char* ssid = "";
const char* password = "";

// blindLength measured in encoder rotations
// stopCalibrate = False boolean variable

// server running on port 4310
AsyncWebServer server(4310);

void connectToWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
 
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"message\":\"Resource not found\"}");
}

void getHello(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{\"message\":\"Welcome\"}");
};

void getStartCalibrate(AsyncWebServerRequest *request) {
  // move blinds to top
  // length = 0
  // stopCalibrate = False
  // while (not stopCalibrate) {
  //   move motor down
  //   increment length 
  // }
  // stopCalibrate = False
  request->send(200, "application/json", "{\"message\":\"Welcome\"}");
};

void getStopCalibrate(AsyncWebServerRequest *request) {
  // stopCalibrate = True
  // return new length
  request->send(200, "application/json", "{\"length\":\"Welcome\"}");
};

AsyncCallbackJsonWebHandler* postMotor = new AsyncCallbackJsonWebHandler("/rest/endpoint", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonObject jsonObj = json.as<JsonObject>(); // using ArduinoJson
  // move the motor to specified level
  // request->send(200, "application/json", "{\"level\":\"Welcome\"}");
});

void configureRouting() {
  server.onNotFound(notFound);
  server.on("/hello", HTTP_GET, getHello);

  server.on("/start_calibrate", HTTP_GET, getStartCalibrate);
  server.on("/start_calibrate", HTTP_GET, getStopCalibrate);

  server.addHandler(postMotor);
}

void setup() {
  Serial.begin(921600);
  connectToWifi();
  configureRouting();
  server.begin();
}

void loop() {}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>;
#include <AsyncJson.h>
#include <ArduinoJson.h>

// Initialise variables
const char* ssid = "";
const char* password = "";

// stopCalibrate = False boolean variable
int currentPosition = 0;
int highest = 0;
int lowest = 0;
boolean stopCalibrationMovement = false;

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

void calibrationMoveUp(AsyncWebServerRequest *request) {
   stopCalibrationMovement = false;
  // while (not stopCalibrationMovement) {
  //   move motor up
  //   update currentPosition
  // }
   stopCalibrationMovement = false;
  request->send(200, "application/json", "{\"message\":\"Moving up\"}");
};

void calibrationMoveDown(AsyncWebServerRequest *request) {
   stopCalibrationMovement = false;
  // while (not stopCalibrationMovement) {
  //   move motor up
  //   update currentPosition
  // }
   stopCalibrationMovement = false;
  request->send(200, "application/json", "{\"message\":\"Moving up\"}");
};

void calibrationSetHighest(AsyncWebServerRequest *request) {
  // return the current encoder step reading which represents 
  // the highest point that the motor should reach
  highest = currentPosition;
  request->send(200, "application/json", "{\"highest\":\"highest\"}");
};

void calibrationSetLowest(AsyncWebServerRequest *request) {
  // return the current encoder step reading which represents 
  // the lowest point that the motor should reach
  request->send(200, "application/json", "{\"message\":\"lowest\"}");
};

void calibrationStopMoving(AsyncWebServerRequest *request) {
  stopCalibrationMovement = true;
  request->send(200);
};

void moveMotor(AsyncWebServerRequest *request) {

  if (request->hasParam("steps")) {
    String steps = request->getParam("steps")->value();
    
    // call motor function that moves the given number of steps
    // note steps may be a negative value, indicating the motor should move backwards
 
    request->send(200, "application/json", "{\"position\":\"{motor encoder reading}\"}");
  } else {
    request->send(400, "application/json", "{\"msg\":\"Invalid request - must provide steps parameter!\"}");
  }
};

//AsyncCallbackJsonWebHandler* postMotor = new AsyncCallbackJsonWebHandler("/rest/endpoint", [](AsyncWebServerRequest *request, JsonVariant &json) {
//  JsonObject jsonObj = json.as<JsonObject>(); // using ArduinoJson
//  // move the motor to specified level
//  // request->send(200, "application/json", "{\"level\":\"Welcome\"}");
//});

void configureRouting() {
  server.onNotFound(notFound);
  server.on("/hello", HTTP_GET, getHello);

  server.on("/calibration_move_up", HTTP_GET, calibrationMoveUp);
  server.on("/calibration_move_down", HTTP_GET, calibrationMoveDown);
  server.on("/calibration_stop_moving", HTTP_GET, calibrationStopMoving);
  server.on("/calibration_set_highest", HTTP_GET, calibrationSetHighest);
  server.on("/calibration_set_lowest", HTTP_GET, calibrationSetLowest);
  server.on("/move", HTTP_GET, moveMotor);

//  server.addHandler(postMotor); // to handle a post request
}

void setup() {
  Serial.begin(9600); // note this may need changing...'upload speed' should be 921600 https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/using-with-arduino-ide
  connectToWifi();
  configureRouting();
  server.begin();
}

void loop() {}

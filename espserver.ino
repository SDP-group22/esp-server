#include <WiFi.h>
#include <ESPAsyncWebServer.h>;
//#include "SDPArduino.h" // for motor
#include <Wire.h> // i2c bus communication
#include "Arduino.h" //this one will be interesting
#include "SI114X.h" // sunlight sensor

// Initialise variables
#define ROTARY_SLAVE_ADDRESS 5 // address of encoder board
#define ROTARY_COUNT 6 // how many motors' encoder values needs to be read
#define PRINT_DELAY 200
#define PIR_MOTION_SENSOR 32 //Use pin 2 to receive the signal from motion sensor

// variables for motor calibration and control
int currentPosition = 0;
boolean stopCalibrationMovement = false;

const char* ssid = "SDProbots";
const char* password = "robotsRus";
IPAddress ip;
SI114X SI1145 = SI114X();
// server running on port 4310
AsyncWebServer server(4310);


void setupSun(){
  Serial.println("Beginning Si1145!");

    while (!SI1145.Begin()) {
        Serial.println("Si1145 is not ready!");
        delay(1000);
    }
    Serial.println("Si1145 is ready!");
  }

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
  ip = WiFi.localIP();
  delay(1000);
}

void getHello(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{\"message\":\"Welcome\"}");
};

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"message\":\"Resource not found\"}");
}


void calibrationSetHighest(AsyncWebServerRequest *request) {
  // return the current encoder step reading which represents 
  // the highest point that the motor should reach
//  stopCalibrationMovement = true;
  //  do we need delay here?
  
  String highestString = String(currentPosition);

  request->send(200, "application/json", "{\"highest\":" + highestString + "}");
};

void calibrationSetLowest(AsyncWebServerRequest *request) {
  // return the current encoder step reading which represents 
  // the lowest point that the motor should reach
 
//  stopCalibrationMovement = true; not needed because expect stop moving request first?
  // delay here?
  String lowestString = String(currentPosition);
  request->send(200, "application/json", "{\"lowest\":" + lowestString + "}");
};

void calibrationStopMoving(AsyncWebServerRequest *request) {
  stopCalibrationMovement = true;
  request->send(200, "application/json", "{\"msg\":\"calibration stop moving\"}");
};

void moveMotor(AsyncWebServerRequest *request) {

  if (request->hasParam("steps")) {
    String stepsString = request->getParam("steps")->value();
    int steps = stepsString.toInt();
    String newStepsString = String(steps + 1);
    Serial.println(newStepsString);
//    if(currentPosition < steps) {
//    moveUp(speed,steps);
    Serial.println("move :" + newStepsString + " steps");
    request->send(200, "application/json", "{\"position\":" + newStepsString + "}");
  } else {
    request->send(400, "application/json", "{\"msg\":\"Invalid request - must provide steps parameter!\"}");
  }
}

void get_motion(AsyncWebServerRequest *request){
    boolean pirval = digitalRead(PIR_MOTION_SENSOR);
    request->send(200, "application/json", "{\"motion\":" +  String(pirval) + "}");
    delay(200);
}

void get_sun(AsyncWebServerRequest *request){
    Serial.print("Vis: "); 
    float vs=SI1145.ReadVisible();
    Serial.println(vs);
    Serial.print("IR: "); 
    float ir = SI1145.ReadIR();
    Serial.println(ir);
    //the real UV value must be div 100 from the reg value , datasheet for more information.
    Serial.print("UV: ");  
    float uv = (float)SI1145.ReadUV() / 100;
    Serial.println(uv);
    request->send(200, "application/json", "{\"uv\":" +  String(uv) + ",ir:"+ String(ir)+",visible:"+ String(vs)+"}");
    delay(1000);
}
void pinSetup(){
  pinMode(PIR_MOTION_SENSOR, INPUT);  
}
void configureRouting() {
  server.onNotFound(notFound);
  server.on("/hello", HTTP_GET, getHello);

//  server.on("/calibration_move_up", HTTP_GET, calibrationMoveUp);
//  server.on("/calibration_move_down", HTTP_GET, calibrationMoveDown);
  server.on("/calibration_stop_moving", HTTP_GET, calibrationStopMoving);
  server.on("/calibration_set_highest", HTTP_GET, calibrationSetHighest);
  server.on("/calibration_set_lowest", HTTP_GET, calibrationSetLowest);
  server.on("/move", HTTP_GET, moveMotor);
  server.on("/get_motion", HTTP_GET, get_motion);
  server.on("/get_sun", HTTP_GET, get_sun);

}
void setup() {
  Serial.begin(115200);
  pinSetup();
  setupSun();
  connectToWifi();
  configureRouting();
  Serial.println("Routing configured");
  server.begin();
  Serial.println("Server is live!");
}

void loop() {
  Serial.println(ip);
  delay(10000);
  // put your main code here, to run repeatedly:

}

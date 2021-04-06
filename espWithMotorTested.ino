#include <WiFi.h>
#include <ESPAsyncWebServer.h>;
#include "SDPArduino.h" // for motor
#include <Wire.h> // i2c bus communication
#include "Arduino.h" //this one will be interesting
#include "SI114X.h" // sunlight sensor
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

// Initialise variables
#define ROTARY_SLAVE_ADDRESS 5 // address of encoder board
#define ROTARY_COUNT 6 // how many motors' encoder values needs to be read
#define PRINT_DELAY 200
#define PIR_MOTION_SENSOR 32 //Use pin 2 to receive the signal from motion sensor


// variables for motor calibration and control
int currentPosition = 0;
int speed = 30; // motor speed
int motorPort = 0;
int positions[ROTARY_COUNT] = {0};
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
void updateMotorPositions() {

  // Request motor position deltas from rotary slave board
  Wire.requestFrom(ROTARY_SLAVE_ADDRESS, ROTARY_COUNT);

  // Update the recorded motor positions
  for (int i = 0; i < ROTARY_COUNT; i++) {
    positions[i] += (int8_t) Wire.read();  // Must cast to signed 8-bit type
  }
  currentPosition = positions[motorPort]; // we use motor on port 1 in lab

};

void printMotorPosition() {
  Serial.print("Motor position: ");
  Serial.print(currentPosition);
  Serial.println();
  delay(PRINT_DELAY);  // Delay to avoid flooding serial out
};
void getHello(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{\"message\":\"Welcome\"}");
};

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"message\":\"Resource not found\"}");
}


void calibrationMoveUp(AsyncWebServerRequest *request) {
  int calibrationSpeed = 30;
  updateMotorPositions();
  int prevpos = currentPosition;

   while(currentPosition < prevpos+100){
    motorForward(motorPort,calibrationSpeed);
    updateMotorPositions();
   }
   motorStop(motorPort);
   delay(300);
   updateMotorPositions();
   Serial.println("motor stop");
   printMotorPosition();
  request->send(200, "application/json", "{\"currentpos\":" + String(currentPosition) + "}");
 
};
void calibrationMoveDown(AsyncWebServerRequest *request) {
int calibrationSpeed = 30;
  updateMotorPositions();
  int prevpos = currentPosition;

   while(currentPosition > (prevpos-100)){
    motorBackward(motorPort,calibrationSpeed);
    updateMotorPositions();
   }
   motorStop(motorPort);
   delay(300);
   updateMotorPositions();
   Serial.println("motor stop");
   printMotorPosition();
   request->send(200, "application/json", "{\"currentpos\":" + String(currentPosition) + "}");
};
void calibrationSetHighest(AsyncWebServerRequest *request) {
 String highestString = String(currentPosition);
  Serial.println("sending highest");
  request->send(200, "application/json", "{\"highest\":" + highestString + "}");
};

void calibrationSetLowest(AsyncWebServerRequest *request) {
   String lowestString = String(currentPosition);
  Serial.println("sending lowest");
  request->send(200, "application/json", "{\"lowest\":" + lowestString + "}");
};
void moveUp(int encoderValue){

  while(currentPosition < encoderValue){
    motorForward(motorPort, speed);
    updateMotorPositions();
  }
  motorStop(motorPort);
  delay(500);
  updateMotorPositions();
};

//moves the motor down to the given encoder value
void moveDown(int encoderValue){

  while(currentPosition > encoderValue){
    motorBackward(motorPort, speed);
    updateMotorPositions();
  }
  motorStop(motorPort);
  delay(500);
  updateMotorPositions();
};


void moveMotor(AsyncWebServerRequest *request) {
int encoderValue = 0;
  if (request->hasParam("encoderValue")) {
    String encoderValueString = request->getParam("encoderValue")->value();
    encoderValue = encoderValueString.toInt();
  }
  else {
    request->send(400, "application/json", "{\"msg\":\"Invalid request - must provide encoderValue parameter!\"}");
  }
    String newPosition;
    Serial.println(encoderValue);
    if(currentPosition < encoderValue) {
    moveUp(encoderValue);
    Serial.println("moved to :" + String(currentPosition));
    newPosition = String(currentPosition);
    request->send(200, "application/json", "{\"position\":" + newPosition + "}");
    }
    else{
    moveDown(encoderValue);
    Serial.println("moved to :" + String(currentPosition));
    newPosition = String(currentPosition);
    request->send(200, "application/json", "{\"position\":" + newPosition + "}");
    }
 }; 
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

  server.on("/calibration_move_up", HTTP_GET, calibrationMoveUp);
  server.on("/calibration_move_down", HTTP_GET, calibrationMoveDown);
  //server.on("/calibration_stop_moving", HTTP_GET, calibrationStopMoving);
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
  //Serial.println(ip);
  //delay(10000);
  // put your main code here, to run repeatedly:
  //TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  //TIMERG0.wdt_feed=1;
  //IMERG0.wdt_wprotect=0;
  //yield();
  //delay(500);
}
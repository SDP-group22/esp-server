#include <WiFi.h>
#include <ESPAsyncWebServer.h>;
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "SDPArduino.h" // for motor
#include <Wire.h> // i2c bus communication
#include "Arduino.h" //this one will be interesting
#include "SI114X.h" // sunlight sensor

#define ROTARY_SLAVE_ADDRESS 5 // address of encoder board
#define ROTARY_COUNT 6 // how many motors' encoder values needs to be read
#define PRINT_DELAY 200
#define PIR_MOTION_SENSOR 2 //Use pin 2 to receive the signal from motion sensor

// Initialise variables
const char* ssid = "";
const char* password = "";
int positions[ROTARY_COUNT] = {};
SI114X SI1145 = SI114X(); // sunlight sensor

// stopCalibrate = False boolean variable
int currentPosition = 0;
int highest = 0;
int lowest = 0;
boolean stopCalibrationMovement = false;

// to store sunlight readings
struct Sunlight {
   float uv;
   int visible;
   int ir;
};
struct Sunlight sunlightReadings;

void setup() {
  Serial.begin(9600); // note this may need changing...'upload speed' should be 921600 https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/using-with-arduino-ide
  connectToWifi();
  configureRouting();
  server.begin();

  // Motor setUp
  SDPsetup();
  digitalWrite(8, HIGH);  // Radio on
  Wire.begin();  // 	Master of the I2C bus
  motorStop(1); // If motor is already running
  delay(500);
  updateMotorPositions(); // Because of the momentum of motor read encoder values since it is stopped
  currentPosition = 0; // Initialize motor position to 0 before calibration
	
  // Sunlight Sensor setUp
  // To check if sensor's address is found
  while (!SI1145.Begin()) { 
        Serial.println("Si1145 is not ready!");
        delay(500);
    }
    Serial.println("Si1145 is ready!");

// motion sensor SetUp
  pinMode(PIR_MOTION_SENSOR, INPUT);
}

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
   while (not stopCalibrationMovement) {
     	motorForward(1, 30);
	updateMotorPositions();
   }
   motorStop(1);
   delay(500);
   updateMotorPositions();
        
   stopCalibrationMovement = false;
   request->send(200, "application/json", "{\"message\":\"Moving up\"}");
};

void calibrationMoveDown(AsyncWebServerRequest *request) {
   stopCalibrationMovement = false;
   while (not stopCalibrationMovement) {
     	motorBackward(1, 30);
	updateMotorPositions();
   }
   motorStop(1);
   delay(500);
   updateMotorPositions();

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

    if(currentPosition < steps)
	moveUp(speed,steps);
 
    request->send(200, "application/json", "{\"position\":\"{motor encoder reading}\"}");
  } else {
    request->send(400, "application/json", "{\"msg\":\"Invalid request - must provide steps parameter!\"}");
  }
};

//moves the motor up to the given encoder value
void moveUp(int speed, int encoderValue){

  while(currentPosition < encoderValue){
    motorForward(1, speed);
    updateMotorPositions();
  }
  motorStop(1);
  delay(500);
  updateMotorPositions();
}

//moves the motor down to the given encoder value
void moveDown(int speed, int encoderValue){

  while(currentPosition > encoderValue){
    motorBackward(1, speed);
    updateMotorPositions();
  }
  motorStop(1);
  delay(500);
  updateMotorPositions();
}

void updateMotorPositions() {

  // Request motor position deltas from rotary slave board
  Wire.requestFrom(ROTARY_SLAVE_ADDRESS, ROTARY_COUNT);

  // Update the recorded motor positions
  for (int i = 0; i < ROTARY_COUNT; i++) {
    positions[i] += (int8_t) Wire.read();  // Must cast to signed 8-bit type
  }
  currentPosition = positions[1]; // we currently use motor on port 1 in lab

}

// in case we want to see what's happening
void printMotorPosition() {
  Serial.print("Motor positions: ");
  Serial.print(currentPosition);
  Serial.println();
  delay(PRINT_DELAY);  // Delay to avoid flooding serial out
}

// Read sunlight sensor
struct Sunlight readSunlight(){

//the real UV value must be div 100 from the reg value
sunlightReadings.ur = (float)SI1145.ReadUV() / 100 ;
sunlightReadings.visible = SI1145.ReadVisible();
sunlightReadings.ir = SI1145.ReadIR(); 

return sunlightReadings;
}

// Read motion sensor
bool readMotion(){

  if(digitalRead(PIR_MOTION_SENSOR)) //if it detects movement
	return true;
  else
        return false;
}

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


//void loop() {}

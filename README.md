# helio-motor

helio-motor contains the sketch that will run the motor component's HTTP server and control the motor hardware

## Setup
1. [Install the ESP32 Board manager for Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-mac-and-linux-instructions/)
2. [Install ESPAsyncWebServer & AsyncTCP libraries manually](https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/). _This requires extracting zip files in the Arduino libraries folder (on my PC this was found /home/{user}/snap/arduino/current/Arduino/libraries)_. Otherwise you could try Sketch > Include Library > Add .zip Library
3. To test the sketch is working, you may want to comment out all code except the functions that connectToWifi. Remember to edit ssid and password at the top of the script.
4. Next, try to get the {ESP32_IP}:4310/hello endpoint to work.

## Notes

## Links
 - [Async Webserver Tutorial](https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/)
 - [How to handle post request body](https://github.com/me-no-dev/ESPAsyncWebServer/issues/594)
 - [Handling Json using ArduinoJson](https://arduinojson.org/v6/api/jsonobject/)
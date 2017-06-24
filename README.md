#Introduction

This is an Arduino Sketch for ESP8266 to control a Pan/Tilt kit equiped with 2 servos.

You can control it with an IR remote or with the Blynk application.

#Configuration

## Wifi

Because the sketch use WifiManager, during the first run you will be able to connect to a Wifi AP to configure the ESP8266 to connect to the Wifi of your choice.

## Blynk server, port and token

During this phase, you will be able to provide the Blynk server address and port and a Blynk token.
The params are stored in a file called config.json in the SPIFFS partition of the ESP8266.
You can directly upload your config file to SPIFFS using the arduino IDE plugin esp8266fs (https://github.com/esp8266/arduino-esp8266fs-plugin).

## IR remote

You will have to edit the sketch and replace to existing codes with your own to get it to work.
You can you use the dumpv2 exemple from the library to learn the values corresponding to your remote.

#OTA

The sketch use the arduino OTA library, so after first upload using serial, you should be able to see the device along with his ip address in the "Tools>Ports" of the arduino IDE.
If the device is correctly detected, you will be able to directly send updates using this "port" without a wired serial connection.
For this, your computer need to be connected to the same wifi network as the esp8266.

#Libraries used

* https://github.com/tzapu/WiFiManager
* https://github.com/bblanchon/ArduinoJson
* https://github.com/markszabo/IRremoteESP8266
* https://github.com/blynkkk/blynk-library

#External links

* Arduino Core : https://github.com/esp8266/Arduino
* Blynk : http://www.blynk.cc

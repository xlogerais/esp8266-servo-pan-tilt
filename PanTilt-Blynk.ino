/***************************************************

 Sketch for ESP8266 with WifiManager and OTA
 to control a Pan/Tilt kit via Blynk

****************************************************/

/*********************/
/*     Pin Mapping   */
/*     for D1 mini   */
/*********************/

// Digital Pin to GPIO mapping for Wemos D1 mini :

#define D0  16  // D0  => GPIO16
#define D1   5  // D1  => GPIO05 (SCL)
#define D2   4  // D2  => GPIO04 (SDA)
#define D3   0  // D3  => GPIO00
#define D4   2  // D4  => GPIO02 (LED builtin)
#define D5  14  // D5  => GPIO14
#define D6  12  // D6  => GPIO12
#define D7  13  // D7  => GPIO13
#define D8  15  // D8  => GPIO15
#define D9   3  // D9  => GPIO03 (RX)
#define D10  1  // D10 => GPIO01 (TX)


/*********************/
/*     Libraries     */
/*********************/

// Config file
#include <FS.h>          //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

// WifiManager
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

// OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Blynk
#include <BlynkSimpleEsp8266.h>   // http://www.blynk.cc

// InfraRed      // https://github.com/markszabo/IRremoteESP8266.git
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// Servo
#include <Servo.h>

/*********************/
/*     Parameters    */
/*********************/

#define SERIAL_SPEED    115200

/************************* Module Name ***************************************/

#define MODULE_NAME     "PanTilt"

/************************* WiFi Access Point default values ******************/

#define WLAN_SSID       "AutoConnectAP"
#define WLAN_PASS       "password"

/************************* Blynk *********************************************/

#define BLYNK_PRINT Serial
// #define BLYNK_DEBUG Serial

/************************* IR *****************************************/

#define IR_RECEIVER_PIN   D4

/************************* Servo *********************************************/
#define PAN  D1
#define TILT D2

#define PAN_MIN   0
#define PAN_MAX   180
#define PAN_STEP  5

#define TILT_MIN  90
#define TILT_MAX  180
#define TILT_STEP 5

/************ Global State (you don't need to change this!) ******************/

char module_name[40] = MODULE_NAME;
char blynk_server[34] = "rpilab";
char blynk_port[6]    = "8442";
char blynk_token[34]  = "YOUR_BLYNK_TOKEN";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
    shouldSaveConfig = true;
}

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient wifi;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// IR
IRrecv irrecv(IR_RECEIVER_PIN);
irparams_t save;         // A place to copy the interrupt state while decoding.
decode_results results;  // Somewhere to store the results

// Servo
Servo pan;
Servo tilt;

/*************************** Sketch Code ************************************/

void setup() {

  // Initialize serial port
  Serial.begin(SERIAL_SPEED);

  // Display welcome message
  Serial.println("");
  Serial.println("ESP8266 with Arduino Core");
  Serial.println("with following features :");
  Serial.println("  * WifiManager");
  Serial.println("  * mDNS");
  Serial.println("  * OTA");
  Serial.println("  * Blynk");
  Serial.println("  * IRremote");
  Serial.println("");

  // Read configuration file config.json from SPIFFS
  Serial.println("Mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");

    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("Reading config file config.json");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nParsed json");

          strcpy(module_name, json["module_name"]);
          strcpy(blynk_server, json["blynk_server"]);
          strcpy(blynk_port, json["blynk_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // Set hostname
  Serial.print("Hello, my name is "); Serial.println(MODULE_NAME);
  WiFi.hostname(MODULE_NAME);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //wifiManager.resetSettings();

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_module_name("name", "module name", module_name, 40);
  WiFiManagerParameter custom_blynk_server("server", "blynk server", blynk_server, 40);
  WiFiManagerParameter custom_blynk_port("port", "blynk port", blynk_port, 5);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33);

  //add all your parameters here
  wifiManager.addParameter(&custom_module_name);
  wifiManager.addParameter(&custom_blynk_server);
  wifiManager.addParameter(&custom_blynk_port);
  wifiManager.addParameter(&custom_blynk_token);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(WLAN_SSID,WLAN_PASS)) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connected...yeey :)");

  //read updated parameters
  strcpy(module_name,  custom_module_name.getValue());
  strcpy(blynk_server, custom_blynk_server.getValue());
  strcpy(blynk_port,   custom_blynk_port.getValue());
  strcpy(blynk_token,  custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["module_name"] = module_name;
    json["blynk_server"] = blynk_server;
    json["blynk_port"]   = blynk_port;
    json["blynk_token"]  = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  /* Serial.print("local ip : "); */
  /* Serial.println(WiFi.localIP()); */

  // OTA
  ota_init(module_name);

  // Blynk
  //Blynk.begin(blynk_token, WLAN_SSID, WLAN_PASS, blynk_server, String(blynk_port).toInt());
  Blynk.config(blynk_token,blynk_server,atoi(blynk_port));
  Blynk.connect();

  // IR
  irrecv.enableIRIn();  // Start the receiver

  // Servo
  pan.attach(PAN);
  tilt.attach(TILT);

}

void loop() {

  if (WiFi.status() == WL_CONNECTED)
  {

  // IR
  if (irrecv.decode(&results, &save)) {

    //dumpInfo(&results);           // Output the results
    //dumpRaw(&results);            // Output the results in RAW format
    //dumpCode(&results);           // Output the results as source code
    //Serial.println("");           // Blank line between entries

    /*
       0xE2E4 => Power
       0xE298 => Play/Pause
       0xE288 => Stop
       0xE2C4 => Green  (music)
       0xE244 => Orange (movies)
       0xE284 => Blue   (photos)
       0xE204 => Yellow (tv)
     */
    switch(results.value)
    {
      case 0xE2E4: // Power
        Serial.println("IR received : Power");
        break;
      case 0xE298: // Play/Pause
        Serial.println("IR received : Play/Pause");
        break;
      case 0xE288: // Stop
        Serial.println("IR received : Stop");
        break;
      case 0xE2C4: // Green
        Serial.println("IR received : Green/Music");
        break;
      case 0xE244: // Orange
        Serial.println("IR received : Orange/Movie");
        break;
      case 0xE284: // Blue
        Serial.println("IR received : Blue/Photo");
        break;
      case 0xE204: // Yellow
        Serial.println("IR received : Yellow/TV");
        break;
      case 0xE230: // Right
        Serial.println("IR received : Right");
        if ( pan.read() > PAN_MIN + PAN_STEP ) { pan.write( pan.read() - PAN_STEP ); }
        break;
      case 0xE2B0: // Left
        Serial.println("IR received : Left");
        if ( pan.read() < PAN_MAX - PAN_STEP ) { pan.write( pan.read() + PAN_STEP ); }
        break;
      case 0xE280: // Up
        Serial.println("IR received : Up");
        if ( tilt.read() > TILT_MIN + TILT_STEP ) { tilt.write( tilt.read() - TILT_STEP ); }
        break;
      case 0xE240: // Down
        Serial.println("IR received : Down");
        if ( tilt.read() < TILT_MAX - TILT_STEP ) { tilt.write( tilt.read() + TILT_STEP ); }
        break;
      default:
        Serial.println("IR received : Unknown");
        // print() & println() can't handle printing long longs. (uint64_t)
        // So we have to print the top and bottom halves separately.
        if (results.value >> 32) Serial.print((uint32_t) (results.value >> 32), HEX);
                                 Serial.println((uint32_t) (results.value & 0xFFFFFFFF), HEX);
        break;
    }

    irrecv.resume();  // Receive the next value

  }
    // OTA
    ArduinoOTA.handle();

    // Blynk
    if(Blynk.connected()) {
      Blynk.run();
    }
    else {
      Serial.println("Not connected to blynk");
      Blynk.connect();
    }

  }
  else
  {
    Serial.println("Not connected to wifi");
    //TODO : handle timeout to reset device
  }

}

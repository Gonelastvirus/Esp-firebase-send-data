#include<Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyAqxrTtpuAmvtjLCMvxkkSyJoeaFcZnRqI "

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://carbon-gecko-293516-default-rtdb.firebaseio.com" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;


//-----------------------------------------------------------
#include<ESP8266WiFi.h>
#include<DNSServer.h>
#include<ESP8266WebServer.h>
#include<ESP8266HTTPClient.h>
#include<WiFiManager.h>
#include<ArduinoJson.h>
#include<WiFiClient.h>
//---------------------------------------------------------------------this block is for wifi setup-----------------------
WiFiManager wifiManager;
char Device_ID[20];
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
//---------------------------------------------------------------                               --------------------------

void setup() {
//--------------------------------------------------------------Wifi setup-------------------------------------------------
 Serial.begin(115200);
 Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(Device_ID, json["Device_ID"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
 if ( WiFi.status() != WL_CONNECTED ) {
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
 }
 //wifiManager.resetSettings();
 WiFiManagerParameter custom_output("Device_ID", "Device_ID",Device_ID , 20);
 wifiManager.setSaveConfigCallback(saveConfigCallback);
 wifiManager.addParameter(&custom_output);
 wifiManager.autoConnect("AdvanceTech");
 strcpy(Device_ID, custom_output.getValue());
//if you get here you have connected to the WiFi
  Serial.println(F("WIFIManager connected!"));
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Device_ID"] = Device_ID;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  pinMode(BUILTIN_LED,OUTPUT);
  pinMode(A0, INPUT);
  Serial.print(F("IP --> "));
  Serial.println(WiFi.localIP());
  Serial.print(F("GW --> "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("SM --> "));
  Serial.println(WiFi.subnetMask());
 
  Serial.print(F("DNS 1 --> "));
  Serial.println(WiFi.dnsIP(0));
 
  Serial.print(F("DNS 2 --> "));
  Serial.println(WiFi.dnsIP(1));
  //-------------------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------firebase begin----------------------------------------------
 
 /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  //anonymous user only 
  if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("ok");
  signupOK = true;
}
else{
  Serial.printf("%s\n", config.signer.signupError.message.c_str());
}
   /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
void loop() {

 if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
  
   	data();
    }
}
void data(){
float smoke_data=analogRead(A0);

  // Write an float number on the database path test/int
 if (Firebase.RTDB.setFloat(&fbdo, "test/float",(smoke_data)/1024)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
}


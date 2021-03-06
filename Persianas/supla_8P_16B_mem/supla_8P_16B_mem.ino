#include <FS.h>       // ---- esp board manager 2.4.2 --- iwip Variant V2 higher Bandwidth
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include "SuplaDevice.h"
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> //--------- https://github.com/tzapu/WiFiManager/tree/0.14 -------------
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <DoubleResetDetector.h> 
#include <Ticker.h>      //for LED status
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>  
extern "C"
{
#include "user_interface.h"
}

#define status_led 16 //D0 
#define DRD_TIMEOUT 3 // Number of seconds after reset during which a  subseqent reset will be considered a double reset.
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

bool pr_wifi = true;
byte input=0;
byte btnlast_val=0;
unsigned long btnlast_time;
byte input2=0;
byte btnlast_val2=0;
unsigned long btnlast_time2;
int s;                   //             ---------------- Status ------------------
unsigned long wifi_checkDelay = 60000;  // wifi reconect delay
unsigned long wifimilis;
bool tikOn = false;
#define BEGIN_PIN 100
Adafruit_MCP23017 mcp;
Adafruit_MCP23017 mcp2;
int inttmp0 = -1;
bool savers0 = false;
int inttmp1 = -1;
bool savers1 = false;
int inttmp2 = -1;
//bool savert0 =false; 
bool savers2 = false;
int inttmp3 = -1;
bool savers3 = false;
int inttmp4 = -1;
bool savers4 = false;
int inttmp5 = -1;
bool savers5 = false;
int inttmp6 = -1;
bool savers6 = false;
int inttmp7 = -1;
bool savers7 = false;
WiFiClient client;
Ticker ticker;
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;

char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
char Supla_name[51];
char update_username[21];
char update_password[21];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout           = 180; // seconds to run for wifi config

void supla_rs_SavePosition(int channelNumber, int position) {
   
          switch(channelNumber)
         {   
   case 0:
    if ((!mcp.digitalRead(0))&&(!mcp.digitalRead(8))){
      inttmp0 = position;
      savers0 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp0 = position;
      savers0 = true;   
     }
    break;
   case 1:
    if ((!mcp.digitalRead(1))&&(!mcp.digitalRead(9))){
      inttmp1 = position;
      savers1 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp1 = position;
      savers1 = true;   
     }
   break;
  case 2:
    if ((!mcp.digitalRead(2))&&(!mcp.digitalRead(10))){
      inttmp2 = position;
      savers2 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp2 = position;
      savers2 = true;   
     } 
   break;
   case 3:
    if ((!mcp.digitalRead(3))&&(!mcp.digitalRead(11))){
      inttmp3 = position;
      savers3 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp3 = position;
      savers3 = true;   
     }
   break;
   case 4:
    if ((!mcp.digitalRead(4))&&(!mcp.digitalRead(12))){
      inttmp4 = position;
      savers4 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp4 = position;
      savers4 = true;   
     }
    break;
   case 5:
    if ((!mcp.digitalRead(5))&&(!mcp.digitalRead(13))){
      inttmp5 = position;
      savers5 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp5 = position;
      savers5 = true;   
     }
   break;
  case 6:
    if ((!mcp.digitalRead(6))&&(!mcp.digitalRead(14))){
      inttmp6 = position;
      savers6 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp6 = position;
      savers6 = true;   
     } 
   break;
   case 7:
    if ((!mcp.digitalRead(7))&&(!mcp.digitalRead(15))){
      inttmp7 = position;
      savers7 = true;
     }
    if (position <= 100 || position >= 10100){
      inttmp7 = position;
      savers7 = true;   
     }
   break;
   } 
}

void supla_rs_LoadPosition(int channelNumber, int *position) {
  unsigned int R_pos = 0;  
  switch(channelNumber)
         {   
   case 0: 
      EEPROM.get(1, R_pos);
      *position = R_pos; 
      Serial.print("read position 0: ");       
      Serial.println(*position);
    break;
    case 1:
      EEPROM.get(5, R_pos);
      *position = R_pos; 
      Serial.print("read position 1: ");       
      Serial.println(*position);   
    break;
    case 2:
      EEPROM.get(11, R_pos);
      *position = R_pos; 
      Serial.print("read position 2: ");       
      Serial.println(*position);  
    break;
    case 3: 
      EEPROM.get(15, R_pos);
      *position = R_pos; 
      Serial.print("read position 3: ");       
      Serial.println(*position);  
    break;
    case 4:                    
      EEPROM.get(21, R_pos);
      *position = R_pos; 
      Serial.print("read position 4: ");       
      Serial.println(*position);
    break;
    case 5:     
      EEPROM.get(25, R_pos);
      *position = R_pos; 
      Serial.print("read position 5: ");       
      Serial.println(*position);  
    break;
    case 6:     
      EEPROM.get(31, R_pos);
      *position = R_pos;
      Serial.print("read position 6: ");       
      Serial.println(*position);  
    break;
    case 7:      
      EEPROM.get(35, R_pos);
      *position = R_pos; 
      Serial.print("read position 7: ");       
      Serial.println(*position);  
    break;
         }    
}

void supla_rs_SaveSettings(int channelNumber, unsigned int full_opening_time, unsigned int full_closing_time) {
  switch(channelNumber)
         {   
   case 0:
       EEPROM.put(50, full_opening_time);
       Serial.println("R0 full opening time Saved"); 
       EEPROM.put(55, full_closing_time);
       Serial.println("R0 full closing time Saved");
       EEPROM.commit(); 
  break;
  case 1:
       EEPROM.put(60, full_opening_time);
       Serial.println("R1 full opening time Saved"); 
       EEPROM.put(65, full_closing_time);
       Serial.println("R1 full closing time Saved");
       EEPROM.commit();
  break;
  case 2:
       EEPROM.put(70, full_opening_time);
       Serial.println("R2 full opening time Saved"); 
       EEPROM.put(75, full_closing_time);
       Serial.println("R2 full closing time Saved");
       EEPROM.commit();
  break;
  case 3:
       EEPROM.put(80, full_opening_time);
       Serial.println("R3 full opening time Saved"); 
       EEPROM.put(85, full_closing_time);
       Serial.println("R3 full closing time Saved");
       EEPROM.commit();       
  break;
  case 4:
       EEPROM.put(90, full_opening_time);
       Serial.println("R4 full opening time Saved"); 
       EEPROM.put(95, full_closing_time);
       Serial.println("R4 full closing time Saved");
       EEPROM.commit(); 
  break;
  case 5:
       EEPROM.put(100, full_opening_time);
       Serial.println("R5 full opening time Saved"); 
       EEPROM.put(105, full_closing_time);
       Serial.println("R5 full closing time Saved");
       EEPROM.commit();
  break;
  case 6:
       EEPROM.put(110, full_opening_time);
       Serial.println("R6 full opening time Saved"); 
       EEPROM.put(115, full_closing_time);
       Serial.println("R6 full closing time Saved");
       EEPROM.commit();
  break;
  case 7:
       EEPROM.put(120, full_opening_time);
       Serial.println("R7 full opening time Saved"); 
       EEPROM.put(125, full_closing_time);
       Serial.println("R7 full closing time Saved");
       EEPROM.commit();      
  break;
  }   
}

void supla_rs_LoadSettings(int channelNumber, unsigned int *full_opening_time, unsigned int *full_closing_time) {
  unsigned int R_opening_time = 0;
  unsigned int R_closing_time = 0;  
   switch(channelNumber)
         {   
  case 0: 
   EEPROM.get(50, R_opening_time);
    EEPROM.get(55, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R0--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   

  break;
  case 1:
   EEPROM.get(60, R_opening_time);
    EEPROM.get(65, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R1--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
     
  break;
  case 2:
   EEPROM.get(70, R_opening_time);
    EEPROM.get(75, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R2--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
    
  break;
  case 3:
   EEPROM.get(80, R_opening_time);
    EEPROM.get(85, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R3--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
    
  break;
  case 4: 
   EEPROM.get(90, R_opening_time);
    EEPROM.get(95, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R4--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
  
  break;
  case 5:
   EEPROM.get(100, R_opening_time);
    EEPROM.get(105, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R5--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
     
  break;
  case 6:
   EEPROM.get(110, R_opening_time);
    EEPROM.get(115, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read 61--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
     
  break;
  case 7:
   EEPROM.get(120, R_opening_time);
    EEPROM.get(125, R_closing_time);
    *full_opening_time = R_opening_time;
    *full_closing_time = R_closing_time;
    Serial.print("read R7--channel: ");Serial.print((channelNumber));Serial.print(" opening_time ");Serial.print((*full_opening_time));Serial.print(" closing_time ");Serial.println((*full_closing_time));   
   
  break;
         }             
}

void tick(){
  int state = digitalRead(status_led);  
  digitalWrite(status_led, !state);     
}
void saveConfigCallback () {                
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
  ticker.attach(0.2, tick);
  
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);
  WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
  WiFiManagerParameter custom_update_username("updateUsername", "update username", update_username, 21,"required");
  WiFiManagerParameter custom_update_password("updatePassword", "update password", update_password, 21,"required");


  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);
  wifiManager.addParameter(&custom_Supla_name);
  wifiManager.addParameter(&custom_update_username);
  wifiManager.addParameter(&custom_update_password);

  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");

  wifiManager.setMinimumSignalQuality();

    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla 8P")) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {      
      Serial.println("connected...yeey :)");    //if you get here you have connected to the WiFi
    }   
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(update_username, custom_update_username.getValue());
    strcpy(update_password, custom_update_password.getValue());
   
    if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;
    json["Supla_name"] = Supla_name;
    json["update_username"] = update_username;
    json["update_password"] = update_password;

    File configFile = SPIFFS.open("/configV2.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    WiFi.softAPdisconnect(true);   //  close AP
    WiFi.mode(WIFI_STA);
    initialConfig = false; 
    ticker.detach();
    digitalWrite(status_led, HIGH);
    ESP.restart();
    delay(5000); 
  }    
  WiFi.softAPdisconnect(true);   //  close AP
}
void supla_timer() {

  unsigned long now = millis();  {
   
        input= mcp2.readGPIO(0);
        if (input != btnlast_val && now - btnlast_time ) {
           btnlast_val = input;
           btnlast_time = now+100;
  
    if (input < 255) {
      Serial.print("Mcp 2A input ");  Serial.println(input);
      delay(200);
      input= mcp2.readGPIO(0);
      if (input < 255) {
      
      if ( input == 254 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            SuplaDevice.rollerShutterStop(0);
        } else {
          Serial.print("0 Reveal");
            SuplaDevice.rollerShutterReveal(0);
        }        
     } 
      
     if ( input == 253 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(1) ) {
          Serial.print("1 Stop");
            SuplaDevice.rollerShutterStop(1);
        } else {
          Serial.print("1 Reveal");
            SuplaDevice.rollerShutterReveal(1);
        }        
     } 

     if ( input == 251 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(2) ) {
          Serial.print("2 Stop");
            SuplaDevice.rollerShutterStop(2);
        } else {
          Serial.print("2 Reveal");
            SuplaDevice.rollerShutterReveal(2);
        }        
     } 

     if ( input == 247 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(3) ) {
          Serial.print("3 Stop");
            SuplaDevice.rollerShutterStop(3);
        } else {
          Serial.print("3 Reveal");
            SuplaDevice.rollerShutterReveal(3);
        }        
     } 
      if ( input == 239 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(4) ) {
          Serial.print("4 Stop");
            SuplaDevice.rollerShutterStop(4);
        } else {
          Serial.print("4 Reveal");
            SuplaDevice.rollerShutterReveal(4);
        }        
     }
     if ( input == 223 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(5) ) {
          Serial.print("5 Stop");
            SuplaDevice.rollerShutterStop(5);
        } else {
          Serial.print("5 Reveal");
            SuplaDevice.rollerShutterReveal(5);
        }        
     } 
     if ( input == 191 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(6) ) {
          Serial.print("6 Stop");
            SuplaDevice.rollerShutterStop(6);
        } else {
          Serial.print("6 Reveal");
            SuplaDevice.rollerShutterReveal(6);
        }        
     } 
     if ( input == 127 ) {      
        if ( SuplaDevice.rollerShutterMotorIsOn(7) ) {
          Serial.print("7 Stop");
            SuplaDevice.rollerShutterStop(7);
        } else {
          Serial.print("7 Reveal");
            SuplaDevice.rollerShutterReveal(7);
        }        
     }       
     }       
    }
   }
   input2= mcp2.readGPIO(1);
        if (input2 != btnlast_val2 && now - btnlast_time2 ) {
           btnlast_val2 = input2;
           btnlast_time2 = now+100;
  
    if (input2 < 255) {
      Serial.print("Mcp 2B input ");  Serial.println(input2);
      delay(200);
      input2= mcp2.readGPIO(1);
      if (input2 < 255) {
      
       if ( input2 == 254 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(0) ) {
          Serial.print("0 Stop");
            SuplaDevice.rollerShutterStop(0);
        } else {
          Serial.print("0 Shut");
            SuplaDevice.rollerShutterShut(0);
        }        
     }

       if ( input2 == 253 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(1) ) {
          Serial.print("1 Stop");
            SuplaDevice.rollerShutterStop(1);
        } else {
          Serial.print("1 Shut");
            SuplaDevice.rollerShutterShut(1);
        }        
     }

      if ( input2 == 251 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(2) ) {
          Serial.print("2 Stop");
            SuplaDevice.rollerShutterStop(2);
        } else {
          Serial.print("2 Shut");
            SuplaDevice.rollerShutterShut(2);
        }        
     }

       if ( input2 == 247 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(3) ) {
          Serial.print("3 Stop");
            SuplaDevice.rollerShutterStop(3);
        } else {
          Serial.print("3 Shut");
            SuplaDevice.rollerShutterShut(3);
        }
      } 
 
      if ( input2 == 239 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(4) ) {
          Serial.print("4 Stop");
            SuplaDevice.rollerShutterStop(4);
        } else {
          Serial.print("4 Shut");
            SuplaDevice.rollerShutterShut(4);
        }        
     }

      if ( input2 == 223 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(5) ) {
          Serial.print("5 Stop");
            SuplaDevice.rollerShutterStop(5);
        } else {
          Serial.print("5 Shut");
            SuplaDevice.rollerShutterShut(5);
        }        
     }

      if ( input2 == 191 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(6) ) {
          Serial.print("6 Stop");
            SuplaDevice.rollerShutterStop(6);
        } else {
          Serial.print("6 Shut");
            SuplaDevice.rollerShutterShut(6);
        }        
     }

      if ( input2 == 127 ) {
        if ( SuplaDevice.rollerShutterMotorIsOn(7) ) {
          Serial.print("7 Stop");
            SuplaDevice.rollerShutterStop(7);
        } else {
          Serial.print("7 Shut");
            SuplaDevice.rollerShutterShut(7);
        }
      }        
     }       
    }
   }
  }    
}
int supla_DigitalRead(int channelNumber, uint8_t pin) {
 switch(channelNumber)
         {  
  case 0:
    if (pin == 101) {
    return mcp.digitalRead(0);
    }
    if (pin == 102) {
    return mcp.digitalRead(8);
    }
    break;
  case 1:
    if (pin == 103) {
    return mcp.digitalRead(1);
    }
    if (pin == 104) {
    return mcp.digitalRead(9);
    }
    break;
  case 2:
    if (pin == 105) {
    return mcp.digitalRead(2);
    }
    if (pin == 106) {
    return mcp.digitalRead(10);
    }
    break;
  case 3:
    if (pin == 107) {
    return mcp.digitalRead(3);
    }
    if (pin == 108) {
    return mcp.digitalRead(11);
    }
    break;
  case 4:
    if (pin == 109) {
    return mcp.digitalRead(4);
    }
    if (pin == 110) {
    return mcp.digitalRead(12);
    }
    break;
  case 5:
    if (pin == 111) {
    return mcp.digitalRead(5);
    }
    if (pin == 112) {
    return mcp.digitalRead(13);
    }
    break;
  case 6:
    if (pin == 113) {
    return mcp.digitalRead(6);
    }
    if (pin == 114) {
    return mcp.digitalRead(14);
    }
    break;
  case 7:
    if (pin == 115) {
    return mcp.digitalRead(7);
    }
    if (pin == 116) {
    return mcp.digitalRead(15);
    }
    break;
         }  
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {  //------------------------------------------------  Virtual ----------------------------
  
  Serial.print("Pin:"); Serial.println(pin);
  Serial.print("Value:"); Serial.println(val);

    if (channelNumber == 0) {
     if (pin == 101){
       mcp.digitalWrite(0, val);
       return;
     }
     if (pin == 102){
       mcp.digitalWrite(8, val);
       return;
     }}
    if (channelNumber == 1) {
     if (pin == 103){
       mcp.digitalWrite(1, val);
       return;
     }
     if (pin == 104){
       mcp.digitalWrite(9, val);
       return;
     }}
    if (channelNumber == 2) {
     if (pin == 105){
       mcp.digitalWrite(2, val);
       return;
     }
     if (pin == 106){
       mcp.digitalWrite(10, val);
       return;
     }}
    if (channelNumber == 3) {
     if (pin == 107){
       mcp.digitalWrite(3, val);
       return;
     }
     if (pin == 108){
       mcp.digitalWrite(11, val);
       return;
     }}
     if (channelNumber == 4) {
     if (pin == 109){
       mcp.digitalWrite(4, val);
       return;
     }
     if (pin == 110){
       mcp.digitalWrite(12, val);
       return;
     }}
    if (channelNumber == 5) {
     if (pin == 111){
       mcp.digitalWrite(5, val);
       return;
     }
     if (pin == 112){
       mcp.digitalWrite(13, val);
       return;
     }}
    if (channelNumber == 6) {
     if (pin == 113){
       mcp.digitalWrite(6, val);
       return;
     }
     if (pin == 114){
       mcp.digitalWrite(14, val);
       return;
     }}
    if (channelNumber == 7) {
     if (pin == 115){
       mcp.digitalWrite(7, val);
       return;
     }
     if (pin == 116){
       mcp.digitalWrite(15, val);
       return;
     }}
     
  return;
        
}

void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  wifi_set_sleep_type(NONE_SLEEP_T);
  EEPROM.begin(512);
  Serial.begin(115200);
  SPIFFS.begin();
  pinMode(status_led,OUTPUT); 
  ticker.attach(0.8, tick);
  Wire.begin();
  Wire.setClock(100000L);

  mcp.begin();      // use default address 0 
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(8, OUTPUT);
  mcp.pinMode(9, OUTPUT);
  mcp.pinMode(10, OUTPUT);
  mcp.pinMode(11, OUTPUT);
  mcp.pinMode(12, OUTPUT);
  mcp.pinMode(13, OUTPUT);
  mcp.pinMode(14, OUTPUT);
  mcp.pinMode(15, OUTPUT);


  mcp2.begin(4);      // use address 4
  mcp2.pinMode(0, INPUT);
  mcp2.pullUp(0, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(1, INPUT);
  mcp2.pullUp(1, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(2, INPUT);
  mcp2.pullUp(2, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(3, INPUT);
  mcp2.pullUp(3, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(4, INPUT);
  mcp2.pullUp(4, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(5, INPUT);
  mcp2.pullUp(5, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(6, INPUT);
  mcp2.pullUp(6, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(7, INPUT);
  mcp2.pullUp(7, HIGH);  // turn on a 100K pullup internally   
  mcp2.pinMode(8, INPUT);
  mcp2.pullUp(8, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(9, INPUT);
  mcp2.pullUp(9, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(10, INPUT);
  mcp2.pullUp(10, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(11, INPUT);
  mcp2.pullUp(11, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(12, INPUT);
  mcp2.pullUp(12, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(13, INPUT);
  mcp2.pullUp(13, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(14, INPUT);
  mcp2.pullUp(14, HIGH);  // turn on a 100K pullup internally
  mcp2.pinMode(15, INPUT);
  mcp2.pullUp(15, HIGH);  // turn on a 100K pullup internally

  if (WiFi.SSID()==""){  
    initialConfig = true;
  } 
    Serial.println("mounting Fs...");
    if (SPIFFS.exists("/configV2.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/configV2.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
          Serial.println("\nparsed json");
          const char* configSupla_server = json["Supla_server"];if (configSupla_server != nullptr){strcpy(Supla_server, json["Supla_server"]); }else{initialConfig = true;}
          const char* configLocation_id = json["Location_id"];if (configLocation_id != nullptr){strcpy(Location_id, json["Location_id"]); }else{initialConfig = true;}
          const char* configLocation_Pass = json["Location_Pass"];if (configLocation_Pass != nullptr){strcpy(Location_Pass, json["Location_Pass"]); }else{initialConfig = true;}
          const char* configSupla_name = json["Supla_name"];if (configSupla_name != nullptr){strcpy(Supla_name, json["Supla_name"]); }else{initialConfig = true;}
          const char* configupdate_username = json["update_username"];if (configupdate_username != nullptr){strcpy(update_username, json["update_username"]); }else{initialConfig = true;}
          const char* configupdate_password = json["update_password"];if (configupdate_password != nullptr){strcpy(update_password, json["update_password"]); }else{initialConfig = true;}
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  
  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    ondemandwifiCallback ();
  } else {
    Serial.println("No Double Reset Detected");
  }
  
  WiFi.mode(WIFI_STA);

   uint8_t mac[WL_MAC_ADDR_LENGTH];
   WiFi.macAddress(mac);
   char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 3],                                
                                 mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 1], mac[WL_MAC_ADDR_LENGTH - 2], 
                                 mac[WL_MAC_ADDR_LENGTH - 3], mac[WL_MAC_ADDR_LENGTH - 4], mac[WL_MAC_ADDR_LENGTH - 5], mac[WL_MAC_ADDR_LENGTH - 6]};
          
     // CHANNEL0 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(101,     // Pin number where the 1st relay is connected
                                     102, false);    // Pin number where the 2nd relay is connected

   // CHANNEL1 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(103,     // Pin number where the 1st relay is connected
                                     104, false);    // Pin number where the 2nd relay is connected

      // CHANNEL2 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(105,     // Pin number where the 1st relay is connected
                                     106, false);    // Pin number where the 2nd relay is connected

      // CHANNEL3 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(107,     // Pin number where the 1st relay is connected
                                     108, false);    // Pin number where the 2nd relay is connected
                                  
     // CHANNEL0 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(109,     // Pin number where the 1st relay is connected
                                     110, false);    // Pin number where the 2nd relay is connected

   // CHANNEL1 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(111,     // Pin number where the 1st relay is connected
                                     112, false);    // Pin number where the 2nd relay is connected

      // CHANNEL2 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(113,     // Pin number where the 1st relay is connected
                                     114, false);    // Pin number where the 2nd relay is connected

      // CHANNEL3 - TWO RELAYS (Roller shutter operation)
  SuplaDevice.addRollerShutterRelays(115,     // Pin number where the 1st relay is connected
                                     116, false);    // Pin number where the 2nd relay is connected

  
  SuplaDevice.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  SuplaDevice.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setRollerShutterFuncImpl(&supla_rs_SavePosition, &supla_rs_LoadPosition, &supla_rs_SaveSettings, &supla_rs_LoadSettings);
  SuplaDevice.setName(Supla_name);

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
        
}

void loop() {

  drd.loop();

  if (initialConfig){
    ondemandwifiCallback () ;
  } 

      if (savers0){
       Serial.print("write position 0:");       
       Serial.println(inttmp0);       
       EEPROM.put(1, inttmp0); 
       EEPROM.commit();       
       savers0 = false;
      }
      if (savers1){
       Serial.print("write position 1:");       
       Serial.println(inttmp1);               
       EEPROM.put(5, inttmp1); 
       EEPROM.commit();       
       savers1 = false;
      }
      if (savers2){
       Serial.print("write position 2:");       
       Serial.println(inttmp2);        
       EEPROM.put(11, inttmp2); 
       EEPROM.commit();       
       savers2 = false;;
      }
      if (savers3){
       Serial.print("write position 3:");       
       Serial.println(inttmp3);             
       EEPROM.put(15, inttmp3); 
       EEPROM.commit();      
       savers3 = false;
      }      
       if (savers4){
       Serial.print("write position 4:");       
       Serial.println(inttmp4);        
       EEPROM.put(21, inttmp4); 
       EEPROM.commit();       
       savers4 = false;
      }
      if (savers5){
       Serial.print("write position 5:");       
       Serial.println(inttmp5);               
       EEPROM.put(25, inttmp5); 
       EEPROM.commit();       
       savers5 = false;
      }
      if (savers6){
       Serial.print("write position 6:");       
       Serial.println(inttmp6);        
       EEPROM.put(31, inttmp6); 
       EEPROM.commit();       
       savers6 = false;
      }
      if (savers7){
       Serial.print("write position 7:");       
       Serial.println(inttmp7);             
       EEPROM.put(35, inttmp7); 
       EEPROM.commit(); ;      
       savers7 = false;
      }      
       
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi_up();
    pr_wifi = true;
  }
  else if ((WiFi.status() == WL_CONNECTED)  && (pr_wifi)){
    Serial.println("");
    Serial.println("CONNECTED");
    Serial.print("local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("subnetMask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("gatewayIP: ");
    Serial.println(WiFi.gatewayIP());
    long rssi = WiFi.RSSI();
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm"); 
    pr_wifi = false;
    ticker.detach();
    digitalWrite(status_led, HIGH);
    
  httpUpdater.setup(&httpServer, "/update", update_username, update_password);
  httpServer.begin();
  }  
  
  SuplaDevice.iterate();
  delay(25);
  httpServer.handleClient();
 
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
    case 17:      // -----     STATUS_REGISTERED_AND_READY
      if (tikOn){            
      ticker.detach();
      digitalWrite(status_led, HIGH);
      tikOn = false;}
    break;

    case 9:      // --------------------- DISCONNECTED  ----------------------
    if (!tikOn){
      ticker.attach(0.8, tick);
      tikOn = true;
    }
   
    break;
  }
}


// Supla.org ethernet layer
    int supla_arduino_tcp_read(void *buf, int count) {
        _supla_int_t size = client.available();
       
        if ( size > 0 ) {
            if ( size > count ) size = count;
            return client.read((uint8_t *)buf, size);
        };
    
        return -1;
    };
    
    int supla_arduino_tcp_write(void *buf, int count) {
        return client.write((const uint8_t *)buf, count);
    };
    
    bool supla_arduino_svr_connect(const char *server, int port) {
          return client.connect(server, 2015);
    }
    
    bool supla_arduino_svr_connected(void) {
          return client.connected();
    }
    
    void supla_arduino_svr_disconnect(void) {
         client.stop();
    }
    
    void supla_arduino_eth_setup(uint8_t mac[6], IPAddress *ip) {

    }

SuplaDeviceCallbacks supla_arduino_get_callbacks(void) {
          SuplaDeviceCallbacks cb;
          
          cb.tcp_read = &supla_arduino_tcp_read;
          cb.tcp_write = &supla_arduino_tcp_write;
          cb.eth_setup = &supla_arduino_eth_setup;
          cb.svr_connected = &supla_arduino_svr_connected;
          cb.svr_connect = &supla_arduino_svr_connect;
          cb.svr_disconnect = &supla_arduino_svr_disconnect;
          cb.get_temperature = NULL;
          cb.get_temperature_and_humidity = NULL;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;
          
          return cb;
}

void WiFi_up(){ // conect to wifi
 unsigned long ahora = millis();
  if (ahora >= wifimilis)  {
  Serial.println("CONNECTING WIFI"); 
  WiFi.begin(); 
  }
  wifimilis = (millis() + wifi_checkDelay) ;
}

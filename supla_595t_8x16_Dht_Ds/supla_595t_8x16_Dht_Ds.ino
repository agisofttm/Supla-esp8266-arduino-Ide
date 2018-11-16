#include <FS.h>  
#include <ShiftRegister74HC595.h>   //https://github.com/Simsso/ShiftRegister74HC595
#include <ESP8266WiFi.h>
#define SUPLADEVICE_CPP
#include <SuplaDevice.h>
// create shift register object (number of shift registers, data pin 14 on 74595, clock pin 11 on 74595, latch pin 12 on 74595)
ShiftRegister74HC595 sr (2, 15, 16, 0); 
#include <math.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <DoubleResetDetector.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 3 // GPIO 3-RX
#define DHTTYPE DHT22 

#define DRD_TIMEOUT 30 // Number of seconds after reset during which a  subseqent reset will be considered a double reset.
#define DRD_ADDRESS 0 // RTC Memory Address for the DoubleResetDetector to use
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

bool eep = LOW;          //             ---------------- Eeprom ------------------
bool startEeprom = true; //             ---------------- Eeprom ------------------
int epr = 0;             //             ----------- Eepron read loops ------------
int s;                   //             ---------------- Status ------------------
unsigned long svr_update = 1800000; //mean time between update  30 minutes
unsigned long svr_update_lasttime;   //last time update
unsigned long wifi_checkDelay = 60000;  // wifi reconect delay
unsigned long wifimilis;
#define BEGIN_PIN 100

// Setup a DHT instance
DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance
OneWire oneWire(01); // GPIO 01

// Pass oneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

unsigned int Rs = 100000;
double Vcc = 3.3;

#define BTN_COUNT 16

WiFiClient client;
//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char Supla_server[40];
char Location_id[15];
char Location_Pass[20];
byte mac[6];

//flag for saving data
bool shouldSaveConfig = false;
bool initialConfig = false;
int timeout           = 120; // seconds to run for wifi config

void saveConfigCallback () {                 //callback notifying us of the need to save config
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
// The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 40);
  WiFiManagerParameter custom_Location_id("ID", "Location_id", Location_id, 15);
  WiFiManagerParameter custom_Location_Pass("Password", "Location_Pass", Location_Pass, 20);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_Supla_server);
  wifiManager.addParameter(&custom_Location_id);
  wifiManager.addParameter(&custom_Location_Pass);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();

  // set configportal timeout
    wifiManager.setConfigPortalTimeout(timeout);

    if (!wifiManager.startConfigPortal("Supla 595")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    
    //read updated parameters
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Location_id, custom_Location_id.getValue());
    strcpy(Location_Pass, custom_Location_Pass.getValue());
    
  WiFi.softAPdisconnect(true);   //  close AP
}
double get_temperature(int channelNumber, double last_val) {

    double t = -275;
    
    if ( sensors.getDeviceCount() > 0 )
      {
         sensors.requestTemperatures();
         t = sensors.getTempCByIndex(0);
      };
           Eeprom_save() ;   //        ------------------------------Eepron save callback -----------------------------
                             //        ---- this happens every few seconds
    return t;  
}
void get_temperature_and_humidity(int channelNumber, double *temp, double *humidity) {

    *temp = dht.readTemperature();
    *humidity = dht.readHumidity();

    if ( isnan(*temp) || isnan(*humidity) ) {
      *temp = -275;
      *humidity = -1;
    }

}
typedef struct {  //------------------------------------------- BTN ----------------------------------------------------
  int pin;
  int relay_pin;
  int channel;
  char last_val;
  int ms;
  unsigned long last_time;
  bool mem;
} _btn_t;

_btn_t btn[BTN_COUNT];

void supla_timer() {
  char v;
  unsigned long now = millis();
  {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        v = digitalRead(btn[a].pin);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           if (v==0)
             {
              if ( btn[a].ms > 0 ) {
                     SuplaDevice.relayOn(btn[a].channel, btn[a].ms);
                     Serial.println(" monostable");
                 } else {
                 if ( (btn[a].mem) == 1 ) {   //   ----------------- == 1 if channel is false... == 0 if channel is true -----------------------
                  SuplaDevice.relayOff(btn[a].channel);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(btn[a].relay_pin);
                 } else {
                  SuplaDevice.relayOn(btn[a].channel, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(btn[a].relay_pin);
                 }        
             }
             }}
      }
    }
    
}
void supla_btn_init() {
  for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        pinMode(btn[a].pin, INPUT_PULLUP);
        btn[a].last_val = digitalRead(btn[a].pin);
        btn[a].last_time = millis();
    }
}
int supla_DigitalRead(int channelNumber, uint8_t pin) {
   
      if (channelNumber == 0){
        if (btn[0].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 1){
        if (btn[1].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 2){
        if (btn[2].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 3){
        if (btn[3].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 4){
        if (btn[4].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 5){
        if (btn[5].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 6){
        if (btn[6].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 7){
        if (btn[7].mem ==0) return 0;      
        else return 1;      
      }
      if (channelNumber == 8){
        if (btn[8].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 9){
        if (btn[9].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 10){
        if (btn[10].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 11){
        if (btn[11].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 12){
        if (btn[12].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 13){
        if (btn[13].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 14){
        if (btn[14].mem ==0) return 0;      
        else return 1;
      }
      if (channelNumber == 15){
        if (btn[15].mem ==0) return 0;      
        else return 1;      
      }
   
}
void suplaDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {  //------------------------------------------------  Virtual ----------------------------
  
     if (channelNumber == 0){
       btn[0].mem =val;
       sr.set(0, val); // set single pin 
     }
     if (channelNumber == 1){
       btn[1].mem =val;
       sr.set(1, val); // set single pin 
     }
     if (channelNumber == 2){
       btn[2].mem =val;
       sr.set(2, val); // set single pin 
     }
     if (channelNumber == 3){
       btn[3].mem =val;
       sr.set(3, val); // set single pin 
     }
     if (channelNumber == 4){
       btn[4].mem =val;
       sr.set(4, val); // set single pin 
     }
     if (channelNumber == 5){
       btn[5].mem =val;
       sr.set(5, val); // set single pin 
     }
     if (channelNumber == 6){
       btn[6].mem =val;
       sr.set(6, val); // set single pin 
     }
     if (channelNumber == 7){
       btn[7].mem =val;
       sr.set(7, val); // set single pin 
     }
     if (channelNumber == 8){
       btn[8].mem =val;
       sr.set(8, val); // set single pin 
     }
      if (channelNumber == 9){
       btn[9].mem =val;
       sr.set(9, val); // set single pin 
     }
     if (channelNumber == 10){
       btn[10].mem =val;
       sr.set(10, val); // set single pin 
     }
     if (channelNumber == 11){
       btn[11].mem =val;
       sr.set(11, val); // set single pin 
     }
     if (channelNumber == 12){
       btn[12].mem =val;
       sr.set(12, val); // set single pin 
     }
     if (channelNumber == 13){
       btn[13].mem =val;
       sr.set(13, val); // set single pin 
     }
     if (channelNumber == 14){
       btn[14].mem =val;
       sr.set(14, val); // set single pin 
     }
     if (channelNumber == 15){
       btn[15].mem =val;
       sr.set(15, val); // set single pin 
     }
     
  return;
    
  
  
}
void Eeprom_save() {                  //----------EEPROM write  ---------------------- EEprom
    if (startEeprom == true){             // ----- don't change memorized state until connected and restored all the channels

      return;
      }                        
      for(int i=0;i<BTN_COUNT;i++) {  //  ---check relay except it have delay (staircase)
         if ( btn[i].ms > 0 ) {
                     continue;
         } else {
        eep = (btn[i].mem);                    //  --- read relay state
        if (eep != EEPROM.read(i)){            //  --- compare relay state with memorized state
         EEPROM.write(i,eep);                  //  --- if different write memory
         Serial.print("EEPROM.");
         Serial.print(i);
         Serial.print(" write.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[i].channel));
         EEPROM.commit();
        }
      }
    }
}
void Eepron_read() {                  //----------EEPROM read  ---------------------- EEprom
                                
   // for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
     //if ( (btn[i].ms) > 0 ) {
                //     continue;
        // } else {
        eep = EEPROM.read(epr);               //  ---read relay state
       
         Serial.print("EEPROM.");
         Serial.print(epr);
         Serial.print(" read.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[epr].channel));
       
        if (eep == HIGH){                    //  --- if 1 send relay on
          SuplaDevice.relayOn(epr, 0);       //  --- only one channel in each pass
          }
        
    if (epr == (BTN_COUNT-1)) {
      startEeprom = false;                //  --- once finished we do not need more
      }     
}
void status_func(int status, const char *msg) {     //    ------------------------ Status --------------------------
 s=status;                                          //    -- to check if we are registered and ready before restore from memory
}
void setup() {  //------------------------------------------------ Setup ----------------------------------------------

  dht.begin(); // Init DHT library   
  sensors.begin();// Init DS18B20 library 
   
  EEPROM.begin(512);

   if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    ondemandwifiCallback ();
  } else {
    Serial.println("No Double Reset Detected");
  }
  if (WiFi.SSID()==""){
    //Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  } 
  //read configuration from FS json
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
        //json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Location_id, json["Location_id"]);
          strcpy(Location_Pass, json["Location_Pass"]);

        } else {
          Serial.println("failed to load json config");
          
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

   uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  char GUID[SUPLA_GUID_SIZE] = {mac[WL_MAC_ADDR_LENGTH - 6], 
                                mac[WL_MAC_ADDR_LENGTH - 5], 
                                mac[WL_MAC_ADDR_LENGTH - 4], 
                                mac[WL_MAC_ADDR_LENGTH - 3], 
                                mac[WL_MAC_ADDR_LENGTH - 2], 
                                mac[WL_MAC_ADDR_LENGTH - 1]};
   
   
          
  SuplaDevice.addRelay(101, false);   
  SuplaDevice.addRelay(102, false);  
  SuplaDevice.addRelay(103, false); 
  SuplaDevice.addRelay(104, false);   
  SuplaDevice.addRelay(105, false);  
  SuplaDevice.addRelay(106, false); 
  SuplaDevice.addRelay(107, false);   
  SuplaDevice.addRelay(108, false);
  SuplaDevice.addRelay(109, false);   
  SuplaDevice.addRelay(110, false);  
  SuplaDevice.addRelay(111, false); 
  SuplaDevice.addRelay(112, false);   
  SuplaDevice.addRelay(113, false);  
  SuplaDevice.addRelay(114, false); 
  SuplaDevice.addRelay(115, false);   
  SuplaDevice.addRelay(116, false);  
  SuplaDevice.addDS18B20Thermometer();// CHANNEL8 - Thermometer DS18B20
  SuplaDevice.addDHT22();             // CHANNEL9 - Thermometer DHT22

  memset(btn, 0, sizeof(btn));
  btn[0].pin =5;          // pin gpio buton  0 = no buton
  btn[0].relay_pin =101;  // pin gpio Relay
  btn[0].channel =0;      // channel
  btn[0].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[0].mem =0;
  btn[1].pin =4;          // pin gpio buton  0 = no buton
  btn[1].relay_pin =102;  // pin gpio Relay
  btn[1].channel =1;      // channel
  btn[1].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[1].mem =0;
  btn[2].pin =2;          // pin gpio buton  0 = no buton
  btn[2].relay_pin =103;  // pin gpio Relay
  btn[2].channel =2;      // channel
  btn[2].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[2].mem =0;
  btn[3].pin =14;         // pin gpio buton  0 = no buton 
  btn[3].relay_pin =104;  // pin gpio Relay
  btn[3].channel =3;      // channel
  btn[3].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[3].mem =0;
  btn[4].pin =12;         // pin gpio buton  0 = no buton
  btn[4].relay_pin =105;  // pin gpio Relay
  btn[4].channel =4;      // channel
  btn[4].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[4].mem =0;
  btn[5].pin =13;         // pin gpio buton  0 = no buton
  btn[5].relay_pin =106;  // pin gpio Relay
  btn[5].channel =5;      // channel
  btn[5].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[5].mem =0;
  btn[6].pin =22;         // pin gpio buton  0 = no buton
  btn[6].relay_pin =107;    // pin gpio Relay
  btn[6].channel =6;      // channel
  btn[6].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[6].mem =0;
  btn[7].pin =22;          // pin gpio buton  0 = no buton
  btn[7].relay_pin =108;  // pin gpio Relay
  btn[7].channel =7;      // channel
  btn[7].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[7].mem =0;
  btn[8].pin =22;          // pin gpio buton  0 = no buton
  btn[8].relay_pin =109;  // pin gpio Relay
  btn[8].channel =8;      // channel
  btn[8].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[8].mem =0;
  btn[9].pin =22;          // pin gpio buton  0 = no buton
  btn[9].relay_pin =110;  // pin gpio Relay
  btn[9].channel =9;      // channel
  btn[9].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[9].mem =0;
  btn[10].pin =22;          // pin gpio buton  0 = no buton
  btn[10].relay_pin =111;  // pin gpio Relay
  btn[10].channel =10;      // channel
  btn[10].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[10].mem =0;
  btn[11].pin =22;         // pin gpio buton  0 = no buton 
  btn[11].relay_pin =112;  // pin gpio Relay
  btn[11].channel =11;      // channel
  btn[11].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[11].mem =0;
  btn[12].pin =22;         // pin gpio buton  0 = no buton
  btn[12].relay_pin =113;  // pin gpio Relay
  btn[12].channel =12;      // channel
  btn[12].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[12].mem =0;
  btn[13].pin =22;         // pin gpio buton  0 = no buton
  btn[13].relay_pin =114;  // pin gpio Relay
  btn[13].channel =13;      // channel
  btn[13].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[13].mem =0;
  btn[14].pin =22;         // pin gpio buton  0 = no buton
  btn[14].relay_pin =115;    // pin gpio Relay
  btn[14].channel =14;      // channel
  btn[14].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[14].mem =0;
  btn[15].pin =22;          // pin gpio buton  0 = no buton
  btn[15].relay_pin =116;  // pin gpio Relay
  btn[15].channel =15;      // channel
  btn[15].ms =0;           //  if = 0 Bistable -- if > 0 Monostable for X ms
  btn[15].mem =0;
  supla_btn_init();

  SuplaDevice.setTemperatureHumidityCallback(&get_temperature_and_humidity);// Set temperature/humidity callback
  SuplaDevice.setDigitalReadFuncImpl(&supla_DigitalRead);    //            ------Send Value to server -------
  SuplaDevice.setDigitalWriteFuncImpl(&suplaDigitalWrite);   //        -------  Read Value from server   -------
  SuplaDevice.setTimerFuncImpl(&supla_timer);
  SuplaDevice.setStatusFuncImpl(&status_func);    //   ----------------------------- Status -----------------------------
  SuplaDevice.setName("Supla 8x16");

  int LocationID = atoi(Location_id);
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    mac,               // Ethernet MAC address
                    Supla_server,      // SUPLA server address
                    LocationID,        // Location ID 
                    Location_Pass);    // Location Password
// read_initial_relay_state();
    for(int i=0;i<BTN_COUNT;i++){      //  ---check relay except der have delay (staircase)
     if ( (btn[i].ms) > 0 ) {
                     continue;
         } else {
        eep = EEPROM.read(i);               //  ---read relay state
       
         Serial.print("Recover.");
         Serial.print(i);
         Serial.print(" read.");
         Serial.println((eep));
         Serial.print(" channel ");
         Serial.println((btn[i].channel));
       
       // if (eep == HIGH){                    //  --- if 1 send relay on
          sr.set(btn[i].channel, eep); // set single pin 
          //SuplaDevice.relayOn(epr, 0);       //  --- only one channel in each pass
         // }
         }
    }         
}

void loop() {

  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd.loop();

  
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Location_id"] = Location_id;
    json["Location_Pass"] = Location_Pass;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");
    shouldSaveConfig = false;
    //end save
  }
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi_up();
    } 
  
  SuplaDevice.iterate();
  SuplaDevice.setTemperatureCallback(&get_temperature);
  switch (s) {    //    ------------------------------------------------ Status ------------------------------------
  case 17:      // -----     STATUS_REGISTERED_AND_READY
  if (epr<BTN_COUNT){
  Serial.print("Eepron_read...");
  Serial.println(epr);
   Eepron_read() ;                // ------------------------------- Eeprom read callback -------------------------------
   epr = epr+1;                   // -------- 1 loop for each output 8 in total ----------
   
  }
  if (millis() > svr_update_lasttime + svr_update)  {
    epr = 0 ;
     startEeprom = true;
      Serial.println("svr update");
        svr_update = 1800000;
            svr_update_lasttime = millis(); 
    }
    break;
    case 9:      // --------------------- DISCONNECTED  ----------------------
    svr_update = 5000;
    break;
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
    svr_update = 5000;
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

       WiFi_up();
    }

SuplaDeviceCallbacks supla_arduino_get_callbacks(void) {
          SuplaDeviceCallbacks cb;
          
          cb.tcp_read = &supla_arduino_tcp_read;
          cb.tcp_write = &supla_arduino_tcp_write;
          cb.eth_setup = &supla_arduino_eth_setup;
          cb.svr_connected = &supla_arduino_svr_connected;
          cb.svr_connect = &supla_arduino_svr_connect;
          cb.svr_disconnect = &supla_arduino_svr_disconnect;
          cb.get_temperature = &get_temperature;
          cb.get_temperature_and_humidity = &get_temperature_and_humidity;
          cb.get_rgbw_value = NULL;
          cb.set_rgbw_value = NULL;
          
          return cb;
}

void WiFi_up() // conect to wifi
{ 
  if (millis() > wifimilis)  {
  WiFi.begin(); 
  for (int x = 10; x > 0; x--) 
  {
    if (x == 1){
    wifimilis = (millis() + wifi_checkDelay) ; 
    }
    if (WiFi.status() == WL_CONNECTED) 
    {    
     return; // break;                           
    }
    else                              
    {
     yield();
     delay(500);   
      Serial.print(".");                
                         
    }
    
  }

  if (WiFi.status() == WL_CONNECTED)
  {
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
  }
  else    
  {
    Serial.println("");
    Serial.println("not connected");
  }
  }
}

#include <PZEM004Tv30.h>
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif
#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif
#if defined(ESP32)
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
#else
PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

#include "WiFi.h"
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <AsyncElegantOTA.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ThingSpeak.h"

#include "WiFiCredentials.h"
WiFiClient  client;
String myStatus = "";
 // obiekty UDP i NTP (czas)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
AsyncWebServer server(80);

//DEKLARACJE
String readFileS(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void getEData();
String getNTPTime();
unsigned long getEpoch();

/* ASYNC WS REQUESTS                      - #PARAM           #RETURN
server.on("/", HTTP_GET, [])              - NIC,             /NIC/
server.on("/test", HTTP_GET, []           - NIC,             ENERGY
server.on("/getCurrData", HTTP_GET, []    - NIC,             JSON WSZYSTKO
server.on("/resetEnergy", HTTP_GET, []    - BOOL (reset),    EPOCH
server.on("/setECost", HTTP_GET, []       - FLOAT (cost),    COST
server.on("/setNTPoffset", HTTP_GET, []   - INT (offset),    /NIC/
*/

#include "filesystem.h"

float ECost = 0.95;
bool spiffsOK = false;
long nptOffset = 7200;

typedef struct energyVals
{
  float voltage, current, power, energy, frequency, pf;
} energyVals;
energyVals EV;

typedef struct energyValsAvg1Min
{
  float voltage=0, current=0, power=0, energy=0, frequency=0, pf=0;
} energyValsAvg1Min;
energyValsAvg1Min EVA1m;

void startWIFI()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Lacze z WiFi..");
    }
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {Serial.println("Nie udalo sie skonfigurowac WiFi STA");}

  String IP = WiFi.localIP().toString().c_str();
  int RSI = WiFi.RSSI();
  Serial.println("IP ESP: "+IP+" , sygnal: "+String(RSI));
}

String getNTPTime()
{
  // Czas z NTP
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  //konkatenacja daty i czasu (SQL format)
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  //Serial.println("CzasNTP: "+currentDate+" "+formattedTime);
  String dateTime = currentDate+" "+formattedTime;
  return dateTime;
}

unsigned long getEpoch()
{
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  epochTime = epochTime-nptOffset;
  return epochTime;
}

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)) {Serial.println("Blad przy montowaniu systemu plikow SPIFFS!");}
  else spiffsOK = true;

  startWIFI();
  // Inicjalizuj klienta NTP
  timeClient.begin();
  timeClient.setTimeOffset(nptOffset); // offset dla GMT+1 (Polska) +1G zmiana czasu

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  #include "asyncws.h"
  ThingSpeak.begin(client);
}

float cost = 0;
void getEData()
{
  EV.voltage = pzem.voltage();
  EV.current = pzem.current();
  EV.power = pzem.power();
  EV.energy = pzem.energy();
  EV.frequency = pzem.frequency();
  EV.pf = pzem.pf();
}

unsigned long eTimerSec = 0;
unsigned long eTimerSecD = 1000;

unsigned long minCounter = 0;
bool okFlag = 1;

void loop() {
    // Print the custom address of the PZEM
    //Serial.print("Custom Address:");
    //Serial.println(pzem.readAddress(), HEX);

    // Zbierz dane energi co 1 sek
    if (millis() - eTimerSec > eTimerSecD)
    {
      getEData();
      okFlag = 1;
      if (isnan(EV.voltage))    { EV.voltage = -1; okFlag = 0;} 
      if (isnan(EV.current))   { EV.current = -1; okFlag = 0;}
      if (isnan(EV.power))     { EV.power = -1; okFlag = 0;}
      if (isnan(EV.energy))    { EV.energy = -1; okFlag = 0;}
      if (isnan(EV.frequency)) { EV.frequency = -1; okFlag = 0;}
      if (isnan(EV.pf))        { EV.pf = -1; okFlag = 0;}

      if (okFlag = 1)
      {
        minCounter++;
        
        EVA1m.voltage+=EV.voltage;
        EVA1m.current+=EV.current;
        EVA1m.power+=EV.power;
        EVA1m.frequency+=EV.frequency;
        EVA1m.pf+=EV.pf;
        cost = ECost*EV.energy;
      }
      if (minCounter==600)
      {
        EVA1m.voltage/=600;
        EVA1m.current/=600;
        EVA1m.power/=600;
        EVA1m.frequency/=600;
        EVA1m.pf/=600;

        ThingSpeak.setField(1, EVA1m.voltage );
        ThingSpeak.setField(2, EVA1m.current );
        ThingSpeak.setField(3, EVA1m.power );
        ThingSpeak.setField(4, EV.energy );
        ThingSpeak.setField(5, EVA1m.frequency );
        ThingSpeak.setField(6, EVA1m.pf );
        ThingSpeak.setField(7, cost );
        
        int code = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if(code == 200){ Serial.println("Zaaktualizowano kanal ThingSpeak"); }
        else{ Serial.println("Blad podczas aktualizicja kanalu ThingSpeak!"); }

        EVA1m.voltage = 0;
        EVA1m.current = 0;
        EVA1m.power = 0;
        EVA1m.frequency = 0;
        EVA1m.pf = 0;
        
        minCounter = 0;
      }
         
      eTimerSec = millis();
    }



}

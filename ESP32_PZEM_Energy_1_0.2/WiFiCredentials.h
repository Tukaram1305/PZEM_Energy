#ifndef WIFICREDENTIALS_H
#define WIFICREDENTIALS_H
//  Parametry lokalnej sieci WWIFI
const char* ssid = "xxxx";
const char* password =  "xxxx";
// spobuje ustawic statyczne IP
IPAddress local_IP(192, 168, 1, 45);
IPAddress gateway(192, 168, 1 ,1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8); 
IPAddress secondaryDNS(8, 8, 4, 4);

//  Dane uwierzytelnienia kanalu na serwerze ThingSpeak
unsigned long myChannelNumber = 2104181;
const char * myWriteAPIKey = "xxxx";

//  Adres lokalnego serwera WWW na maszynie stacjonarnej
String ServerAddress = "192.168.1.55/smartesp";

#endif

/*
 *             // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(EV.voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(EV.current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(EV.power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(EV.energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(EV.frequency, 1); Serial.println("Hz");
        Serial.print("PF: "); Serial.println(EV.pf);
 */

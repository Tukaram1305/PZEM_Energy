#ifndef ASYNCWS_H
#define ASYNCWS_H
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String mess = "AWS OK, PZEM Addr.: "+String(pzem.readAddress(), HEX);
    request->send(200, "text/plain", mess.c_str());
  });

  server.on("/test", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "application/json", String(EV.power,2).c_str());
  });

  // Pobierz aktualne dane
  server.on("/getCurrData", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String dateTime = getNTPTime();
    String LE = readFileS(SPIFFS, "/lastEnergy.txt");
    if (LE=="") LE = "BRAK";
    String form = "{\"V\":"+String(EV.voltage, 3)+
                  ",\"C\":"+String(EV.current, 4)+
                  ",\"P\":"+String(EV.power, 4)+
                  ",\"E\":"+String(EV.energy, 5)+
                  ",\"F\":"+String(EV.frequency, 2)+
                  ",\"X\":"+String(EV.pf, 3)+
                  ",\"O\":"+"\""+LE+"\""+
                  ",\"S\":"+String(spiffsOK)+
                  ",\"D\":"+"\""+dateTime+"\""+"}"+"\n";
            
    request->send(200, "application/json",form.c_str());
    Serial.println("GET: podaje energie");
  });

  server.on("/resetEnergy", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("reset")) 
    {
      String inputMessage = request->getParam("reset")->value();
      int state = inputMessage.toInt();
      if (state==1) 
      {
        pzem.resetEnergy();
        unsigned long epoch = getEpoch();
        String sEpoch = String(epoch)+"\n";
        writeFile(SPIFFS, "/lastEnergy.txt", sEpoch.c_str() );

        String message = "Reset E, epoch: "+String(epoch);
        request->send(200, "text/plain", message.c_str());
      }
      else request->send(200, "text/plain", String("Nic nie robie").c_str());
    }
    });

  server.on("/setECost", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("cost")) 
    {
      String inputMessage = request->getParam("cost")->value();
      float val = inputMessage.toFloat();
      ECost = val;
      request->send(200, "text/plain", String(ECost).c_str());
    }
    else request->send(200, "text/plain", String(ECost).c_str());
    });    

  server.on("/setNTPoffset", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("offset")) 
    {
      String inputMessage = request->getParam("offset")->value();
      int val = inputMessage.toInt();
      timeClient.setTimeOffset(val);
      nptOffset = val;
      request->send(200, "text/plain", String(val).c_str());
    }
    else request->send(200, "text/plain", String("Offset NTP staly").c_str());
    });  

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
#endif

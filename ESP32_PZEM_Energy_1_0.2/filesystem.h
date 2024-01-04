#ifndef FILESYSTEM_H
#define FILESYSTEM_H

String readFileS(fs::FS &fs, const char * path){
    Serial.printf("Czytam plik: %s\r\n", path);
    String buff = "";
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- Blad przy otwieraniu pliku do odczytu!");
        return "";
    }

    Serial.println("- czytam z pliku:");
    while(file.available()){
        buff = file.readStringUntil('\n');
        Serial.println(buff);
    }
    file.close();
    return buff;
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Czytam plik: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- Blad przy otwieraniu pliku do odczytu!");
        return;
    }

    Serial.println("- czytam z pliku:");
    String buff = "";
    uint16_t iter = 0;
    while(file.available()){
        //Serial.write(file.read());
        buff = file.readStringUntil('\n');
        iter++;
        Serial.print(String(iter)+"------");
        Serial.println(buff);
        //if (buff=="\n") break;
        //else if(buff=="") break;
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Zapisuje/nadpisuje plik: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- blad przy otwierniu pliku do zapisu!");
        return;
    }
    if(file.print(message)){
        Serial.println("- zapisano plik");
    } else {
        Serial.println("Plik wyczyszczony");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Dopisuje do pliku: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- blad przy otwieraniu pliku!");
        return;
    }
    if(file.print(message)){
        Serial.println("- dane dolaczone");
    } else {
        Serial.println("- blad przy dolaczaniu do pliku!");
    }
    file.close();
}

#endif

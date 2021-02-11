

#include <SD.h>
#include <Streaming.h>
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson/issues/223, https://github.com/bblanchon/ArduinoJson
#include <EEPROM.h>

#include "SerialNumber.h"

char snFileName[20] = "SN.CFG";


void readSerialNumberConfigurationFromFile(void) {
  char json[512];
  char buf [256];
  File snCfgFile = SD.open(snFileName, FILE_READ);

  if ( snCfgFile == 0 ) {
    // unable to open config file/does not exist
    Serial << endl << "  No " << snFileName << " found. No change to S/N." << endl;
  }
  else {
    Serial << endl << "  Reading... " << snFileName;
    snCfgFile.readBytes(json, sizeof(json));
    snCfgFile.close();
    StaticJsonBuffer<JSON_OBJECT_SIZE(14)> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    Serial << " OK." << endl;
    Serial << "  Parsing S/N config file... " << endl;

    String snstr = root["SN"];
    char snstrc[7];
    snstr.toCharArray(snstrc, 7);
    
    Serial << snstr << endl;

    int go    = root["GO"];
    Serial << "  Go-Flag... " << go << endl;
    if ( go == 1 ) {
      /* set the serial number */
      int addr = 0;
      for ( int i = 0; i < 7; i++) {
        EEPROM.write(addr, snstr[i]);
        addr++;
      }
      /* reset the "GO" flag */
      SD.remove("SN.cfg");
      snCfgFile = SD.open(snFileName, FILE_WRITE);
      sprintf(buf, "{\"SN\":\"%s\",\"GO\":\"0\"}", snstrc);
      snCfgFile.println(buf);
      snCfgFile.close();
      asm volatile ("  jmp 0");  // ebichris, 2019-02-04
    }
    else {
      Serial <<  "  No change to SN" << endl;
    }
  }
}

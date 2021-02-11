/*! \Configuration.cpp
 *  \Configuration file utilities 
 *
 *   _____    _____    ___    __    ___  _____      _____
 *  /  _  \  |__    \  \  \  /  \  /  / |__    \   /  _  \
 * |  /_\  |  __| _  |  \  \/ __ \/  /   __| _  | |  |_|  |
 * |  |____  |   |_| |   \   /  \   /   |   |_| |  \____  |
 *  \______| |_______|    \_/    \_/    |_______|   _   | |
 * Eawag                                           | \__/ |
 * Department Urban Water Management                \____/
 * Ueberlandstrasse 133
 * CH-8600 Duebendorf
 *
 * and kofatec GmbH, CH-5735 Pfeffikon, www.kofatec.ch 
 *
 * The SQUID hardware and software work is licensed under a Creative
 * Commons Attribution-ShareAlike 2.0 Generic License and this program is 
 * free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 2.1 of the License, or (at your option) any 
 * later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Design and Implementation:	Adrian Koller, Christian Ebi
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson/issues/223, https://github.com/bblanchon/ArduinoJson

#include "Data.h"
#include "Configuration.h"


void setDefaultConfiguration(Configuration& cfg){
   cfg.phEna = 1.0;
   cfg.phDly = 0;
   cfg.ecEna = 1.0;
   cfg.ecFrequency = 40000;
   cfg.ecFrequencyIncrement = 0;
   cfg.ecNumberOfIncrements = 0;
   cfg.orpEna = 1.0;
   cfg.orpDly = 0;
   cfg.cycleT = 5;
   cfg.buzzerOff = 0;
   cfg.ledOff = 0;   
   cfg.samplingDelay = 0; //milliseconds
   cfg.filterSize = 10;

   cfg.tempSensor.scale = 0;
   cfg.tempSensor.offset = 0;
   cfg.pHref4 = 4.0;
   cfg.pHref7 = 7.0;
   cfg.pHref10 = 10.0;
   cfg.pHcal4 = 0.174;
   cfg.pHcal7 = 0.0;
   cfg.pHcal10 = -0.174;
   cfg.pHcalT = 22.0;
   cfg.orpcalNeg220mV = -0.220 ;
   cfg.orpcal0 = 0.0;
   cfg.orpcalPos220mV = 0.220;
   cfg.orpcalT = 22.0;
   cfg.ecCalRef = 1413e-6; 
   cfg.ecCalRawValue = 575.06;
   cfg.ecCalT = 25.0; //leave at 25degC for Kcell calculation without temperature compensation part.
   cfg.ecCalTalpha = 2.0; //leave at 2

}


// pass in the default configuration, if a good config can be read from file it will be overwritten 
void readConfigurationFromFile(Configuration& cfg) {
   char buf[128];
   char json[1024];

   // open the file
   File sensorCfgFile = SD.open(cfg.filename, FILE_READ);
   if( sensorCfgFile == 0 ) {
      // no config file found, create default configuration 

      Serial << "  No " << cfg.filename << " found, using default config." << endl;

      Serial << "  pH enabled: " << cfg.phEna << endl;
      Serial << "  pH delay (ms): " << cfg.phDly << endl;
      Serial << "  EC enabled: " << cfg.ecEna << endl;
      Serial << "  ORP enabled: " << cfg.orpEna << endl;
      Serial << "  ORP delay (ms): " << cfg.orpDly << endl;
      Serial << "  Cycle Time (s): " << cfg.cycleT << endl;

   } 
   else {
      // valid cfg file was found, read in the configuration
      sensorCfgFile.readBytes(json, sizeof(json));
      sensorCfgFile.close();

      StaticJsonBuffer<JSON_OBJECT_SIZE(27)> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(json);

      sprintf(buf, "  %s found. \r\nSettings & Calibrations:", cfg.filename);
      Serial.println(buf);

      cfg.phEna = root["phEna"];
      cfg.phDly = root["phDly"];
      cfg.ecEna = root["ecEna"];
      cfg.orpEna = root["orpEna"];
      cfg.orpDly = root["orpDly"];
      cfg.cycleT = root["cycleT"];
      if (root.containsKey("buzzerOff"))  cfg.buzzerOff = root["buzzerOff"];
      if (root.containsKey("ledOff")) cfg.ledOff = root["ledOff"];
      if (root.containsKey("samplingDelay")) cfg.samplingDelay = root["samplingDelay"];    
      if (root.containsKey("filterSize")) cfg.filterSize = root["filterSize"];            
      cfg.tempSensor.scale = root["aT"];
      cfg.tempSensor.offset = root["bT"];
      cfg.pHref4 = root["pHref4"];
      cfg.pHref7 = root["pHref7"];
      cfg.pHref10 = root["pHref10"];
      cfg.pHcal4 = root["pHcal4"];
      cfg.pHcal7 = root["pHcal7"];
      cfg.pHcal10 = root["pHcal10"];
      cfg.pHcalT = root["pHcalT"];
      cfg.orpcalNeg220mV = root["orpcalNeg220mV"];
      cfg.orpcal0 = root["orpcal0"];
      cfg.orpcalPos220mV = root["orpcalPos220mV"];
      cfg.orpcalT = root["orpcalT"];
      cfg.ecCalRef = root["ecCalRef"];
      cfg.ecCalRawValue = root["ecCalRawValue"];
      cfg.ecCalT = root["ecCalT"];
      cfg.ecCalTalpha = root["ecCalTalpha"];

      if(cfg.ecCalTalpha == 0 ){
        cfg.ecCalTalpha = 2.0;
      }

      Serial << "  pH enabled: " << cfg.phEna,0 << endl;
      Serial << "  pH delay (ms): " << cfg.phDly << endl;
      Serial << "  EC enabled: " << cfg.ecEna << endl;
      Serial << "  ORP enabled: " << cfg.orpEna << endl;
      Serial << "  ORP delay (ms): " << cfg.orpDly << endl;
      Serial << "  Cycle Time (s): " << cfg.cycleT << endl;

      Serial << "  Temp: aT: " << _FLOAT(cfg.tempSensor.scale,3);
      Serial << ", bT: " << _FLOAT(cfg.tempSensor.offset,3) << endl;

      Serial << "  pH: pHref4: " << cfg.pHref4;
      Serial << ", pHref7: " << cfg.pHref7;
      Serial << ", pHref10: " << cfg.pHref10;
      Serial << ", pHcal4: " << _FLOAT(cfg.pHcal4,3);
      Serial << ", pHcal7: " << _FLOAT(cfg.pHcal7,3);
      Serial << ", pHcal10: " << _FLOAT(cfg.pHcal10,3);
      Serial << ", pHcalT: " << cfg.pHcalT  << endl;

      Serial << "  ORP: orpcalNeg220mV: " << _FLOAT(cfg.orpcalNeg220mV,3);
      Serial << ", orpcal0: " << _FLOAT(cfg.orpcal0,3);
      Serial << ", orpcalPos220mV: " << _FLOAT(cfg.orpcalPos220mV,3);
      Serial << ", orpcalT: " << cfg.orpcalT << endl;

      Serial << "  EC: ecCalRef: " << _FLOAT(cfg.ecCalRef,7);
      Serial << ", ecCalRawValue: " << cfg.ecCalRawValue;
      Serial << ", ecCalT: " << cfg.ecCalT;
      Serial << ", ecCalTalpha: " << cfg.ecCalTalpha << endl;
   }
}                      

// Saves the configuration to a file
void saveConfiguration(const Configuration& cfg) {
  // Delete existing file, otherwise the configuration is appended to the file
  SD.remove(cfg.filename);

  // Open file for writing
  File file = SD.open(cfg.filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate the memory pool on the stack
  // Don't forget to change the capacity to match your JSON document.
  // Use https://arduinojson.org/assistant/ to compute the capacity.
  StaticJsonBuffer<512> jsonBuffer;

  // Parse the root object
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["phEna"] = cfg.phEna;
  root["phDly"] = cfg.phDly;
  root["ecEna"] = cfg.ecEna;
  root["orpEna"] =  cfg.orpEna;
  root["orpDly"] = cfg.orpDly;
  root["cycleT"] = cfg.cycleT;
  root["buzzerOff"] = cfg.buzzerOff;  
  root["ledOff"] = cfg.ledOff;   
  root["samplingDelay"] = cfg.samplingDelay;
  root["filterSize"] = cfg.filterSize;  
  root["aT"] = cfg.tempSensor.scale;
  root["bT"] = cfg.tempSensor.offset;
  root["pHref4"] = cfg.pHref4;
  root["pHref7"] = cfg.pHref7;
  root["pHref10"] = cfg.pHref10;
  root["pHcal4"] = cfg.pHcal4;
  root["pHcal7"] = cfg.pHcal7;
  root["pHcal10"] = cfg.pHcal10;
  root["pHcalT"] = cfg.pHcalT;
  root["orpcalNeg220mV"] = cfg.orpcalNeg220mV;
  root["orpcal0"] = cfg.orpcal0;
  root["orpcalPos220mV"] = cfg.orpcalPos220mV;
  root["orpcalT"] = cfg.orpcalT;
  root["ecCalRef"] = cfg.ecCalRef;
  root["ecCalRawValue"] = cfg.ecCalRawValue;
  root["ecCalT"] = cfg.ecCalT;
  root["ecCalTalpha"] = cfg.ecCalTalpha;

  // Serialize JSON to file
  if (root.printTo(file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

// Prints the content of a file to the Serial
void printFile(const char *filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

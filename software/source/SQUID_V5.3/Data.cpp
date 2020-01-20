/*! \Data.cpp
 *  \Utilities for data logging onto uSD card
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
#include <SD.h>
#include "RTC.h"
#include "Data.h"
#include "Configuration.h"

const int chipSelect = 10;
char datafilename[13] = "dataXXXX.txt";

/* Filename convention:
 *  dataXXXX.txt where XXXX is a sequential number. This function finds the lowest
 *  XXXX index and creates a file name. If XXXX = 9999 exists, filename is set to NULL
 */
bool createValidDataFileName(char* datafilename){
   bool success = true;
   int i = 0;
   for(i = 0; i< 10000; i++){
      sprintf(datafilename, "data%04d.txt", i);
      if( !SD.exists(datafilename) ){
         break;      
      }
   }
   if(i == 9999){
      datafilename = NULL;
      success = false;
      /* error flag */
   }
   return success;
}


bool initializeDataFileWithHeader(char* serialNumber, int majorversion, int minorversion, Configuration cfg ) {
   bool success = true;
   File dataFile = SD.open(datafilename, FILE_WRITE);
   if (dataFile) {
      dataFile.print("SQUID Serial#: ");
      dataFile.print(serialNumber);
      dataFile.println();
      dataFile.print("Software: ");
      dataFile.print(majorversion);
      dataFile.print(".");
      dataFile.print(minorversion);
      dataFile.println();
      dataFile.print("Active Configuration:");
      dataFile.println();
      dataFile.print("pH enabled: ");
      dataFile.print(cfg.phEna);
      dataFile.println();

      dataFile.print("pH delay (ms): ");
      dataFile.print(cfg.phDly);
      dataFile.println();

      dataFile.print("EC enabled: ");
      dataFile.print(cfg.ecEna);
      dataFile.println();
        
      dataFile.print("ORP enabled: ");
      dataFile.print(cfg.orpEna);
      dataFile.println();

      dataFile.print("ORP delay (ms): ");
      dataFile.print(cfg.orpDly);
      dataFile.println();

      dataFile.print("Cycle Time (s): ");
      dataFile.print(cfg.cycleT);
      dataFile.println();
      
      dataFile.print("Sampling delay (ms): ");
      dataFile.print(cfg.samplingDelay);
      dataFile.println();    
      
      dataFile.print("Filter Size (samples): ");
      dataFile.print(cfg.filterSize);
      dataFile.println();          
      
      dataFile.print("Buzzer Off (1 means \"off\"): ");
      dataFile.print(cfg.buzzerOff);
      dataFile.println(); 
      
      dataFile.print("LED Off (1 means \"off\"): ");
      dataFile.print(cfg.ledOff);
      dataFile.println();       

      dataFile.print("Temp: aT: ");
      dataFile.print(cfg.tempSensor.scale,4);
      dataFile.print(", bT: ");
      dataFile.print(cfg.tempSensor.offset,4);
      dataFile.println();

      dataFile.print("pH: pHref4: ");
      dataFile.print(cfg.pHref4,4);
      dataFile.print(", pHref7: ");
      dataFile.print(cfg.pHref7,4);
      dataFile.print(", pHref10: ");
      dataFile.print(cfg.pHref10,4);
      dataFile.print(", pHcal4: ");
      dataFile.print(cfg.pHcal4,4);
      dataFile.print(", pHcal7: ");
      dataFile.print(cfg.pHcal7,4);
      dataFile.print(", pHcal10: ");
      dataFile.print(cfg.pHcal10,4);
      dataFile.print(", pHcalT: ");
      dataFile.print(cfg.pHcalT,4);
      dataFile.println();

      dataFile.print("ORP: orpcalNeg220mV: ");
      dataFile.print(cfg.orpcalNeg220mV,4);
      dataFile.print(", orpcal0: ");
      dataFile.print(cfg.orpcal0,4);
      dataFile.print(", orpcalPos220mV: ");
      dataFile.print(cfg.orpcalPos220mV,4);
      dataFile.print(", orpcalT: ");
      dataFile.print(cfg.orpcalT,4);
      dataFile.println();

      dataFile.print("EC: ecCalRef: ");
      dataFile.print(cfg.ecCalRef,4);
      dataFile.print(", ecCalRawValue: ");
      dataFile.print(cfg.ecCalRawValue,4);
      dataFile.print(", ecCalT: ");
      dataFile.print(cfg.ecCalT,4);
      dataFile.print(", ecCalTalpha: ");
      dataFile.print(cfg.ecCalTalpha,4);
      dataFile.println();

      dataFile.print("Time, Traw, T, pHraw, pHmV, pH, orpRaw, orpmV, orp, ecRRraw, ecIRaw, ec, VBat");
      dataFile.println();
      dataFile.close();
   }  
   else {
      success = false;
   }
   return success;
}

bool saveDataPointToFile(time_t t, SquidData data, float fBatteryVoltage){
   bool success = true;
   char buf[256];
   File dataFile = SD.open(datafilename, FILE_WRITE);
   
   if (dataFile) {
      t=now();
      getCurrentTimeInText(t, buf);
      dataFile.print(buf);
      dataFile.print(",");

      dataFile.print(data.tempRaw);
      dataFile.print(",");
      dataFile.print(data.t,4);
      dataFile.print(",");

      dataFile.print(data.phRaw);
      dataFile.print(",");
      dataFile.print(data.phmV,4);
      dataFile.print(",");    
      dataFile.print(data.ph,4);
      dataFile.print(",");

      dataFile.print(data.orpRaw);
      dataFile.print(",");
      dataFile.print(data.orpmV,4);
      dataFile.print(",");    
      dataFile.print(data.orp,4);
      dataFile.print(",");

      dataFile.print(data.ecRealRaw);
      dataFile.print(",");
      dataFile.print(data.ecImagRaw);
      dataFile.print(",");
      dataFile.print(data.ec,7);
      dataFile.print(",");
      dataFile.print(fBatteryVoltage,2);      
      dataFile.println();
      dataFile.close();
   } 
   else {
      success = false;
   }
   dataFile.close();  
   return success;
}

// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
   time_t currentTime;
   
   currentTime = now();

   // return date using FAT_DATE macro to format fields
   *date = FAT_DATE(year(), month(), day());

   // return time using FAT_TIME macro to format fields
   *time = FAT_TIME(hour(), minute(), second());
}

bool initializeSdCard( void ){
   bool success = true;
   
   SdFile::dateTimeCallback(dateTime); 
   
   // see if the card is present and can be initialized:
   if (!SD.begin(chipSelect)) {
      success = false;
   }
   return success;
}

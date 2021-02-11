/*! \usermenu.cpp
 * \Simple user menu
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
 *
*/
#include <Arduino.h>
#include <Streaming.h>
#include "zmodem.h"
#include "RTC.h"
#include "Data.h"
#include "Configuration.h"
#include "usermenu.h"
#include "pHORPTBatt.h"
#include "EC.h"

extern char serialNumber[];
extern int majorversion;
extern int minorversion;
extern Configuration cfg;
extern SquidData data;


int UM_CheckforCmd() {

   #define BUF_LEN 64
   char inputBuffer [BUF_LEN] = {NULL};
   char * pch;
   unsigned int i=0;
   int iRetValue = 0;

   while (Serial.available() > 0 && i < BUF_LEN  )
   {
      inputBuffer[i++]=Serial.read();
      delay(100);
   }

   if (strstr (inputBuffer,"*****")) {
      iRetValue = -1;
   }
   else if (strstr (inputBuffer,"------")) {
      iRetValue = -2;    
   }
   else {
      iRetValue = 0;
   }

   return iRetValue;
}


void UM_ShowMenu(uint32_t timeout)
{
   uint32_t previous = 0;
   char option;
   int i = 0;
   int iQuit = false;
   
	// get current time
	previous = millis();

	do
	{
      while(Serial.available() > 0){
         Serial.read();		
      }
      Serial.println(F("\r\n------------------------- MENU -------------------------"));
      Serial.println(F("1: Start Filemanager"));
      Serial.println(F("2: Show version"));
      Serial.println(F("3: Set date and time (RTC)"));     
      Serial.println(F("4: Reboot and start new data file"));  
      Serial.println(F("5: Sensor calibration menu"));  
      Serial.println(F("6: System configuration")); 
      Serial.println(F("9: Stop and power off"));                                   
      Serial.println(F("0: Quit"));
      Serial.println(F("--------------------------------------------------------"));

      Serial.print(F("\n==> Enter numeric option:"));
      Serial.flush();

      // wait for incoming data from keyboard
      while (!Serial.available() && (millis()-previous < timeout));

      // parse incoming bytes
      if (Serial.available() > 0)
      {
         previous = millis();

         // get option number
         option = Serial.read();
         Serial.println(option);
         Serial.flush();

         switch (option)
         {
            case '1':
            // ************
            //  ToDo ZModem
               zModemInit();
               zModemRun();
               previous = millis();
               break;
            case '2':
               Serial  << endl << endl;
               Serial << "********************" << endl;
               Serial << "Serial Number: " << serialNumber << endl;
               Serial << "Software Version: " << majorversion << "." << minorversion << endl << endl;
               Serial << "Build date and time: " << __DATE__ << " " << __TIME__ << endl << endl;                  
               previous = millis();
               break;
            case '3':
               RtcSetupMenu();
               previous = millis();               
               break;
            case '4':
               asm volatile ("  jmp 0");
               break;      
            case '5':
               UM_CalibrationMenu(20000);
               break;  
            case '6':
               UM_SystemConfigMenu(20000);
               previous = millis();                  
               break; 
            case '9': 
               Serial.println(F("Power is off"));
               Serial.flush();
               delay(1000);
               pinMode(SHUTDOWN_N, OUTPUT);
               digitalWrite(SHUTDOWN_N, HIGH);   // low means enabled!     
               break;                        
            case '0':
               iQuit = true;
               break;
            default:
               break;
         }
      }

      // Condition to avoid an overflow (DO NOT REMOVE)
      if (millis() < previous) previous = millis();

	} while ((iQuit == false) && (millis()-previous < timeout));

	Serial.println();
}

void UM_CalibrationMenu(uint32_t timeout)
{
   uint32_t previous = 0;
   char option;
   int i = 0;
   int iQuit = false;
   float fLastValue = 0.0;
   
  // get current time
  previous = millis();
  
  digitalWrite(PIN_5V_ENA, HIGH);
  digitalWrite(EN_PH, LOW);

  do
  {
      while(Serial.available() > 0){
         Serial.read();   
      }
      Serial.println(F("\r\n-------------CALIBRATION MENU -------------------------"));
      Serial.println(F("1: pH4 Calibration"));
      Serial.println(F("2: pH7 Calibration"));
      Serial.println(F("3: pH10 Calibration"));
      Serial.println(F("4: EC Calibration"));   
      Serial.println(F("9: Store data to configuration"));                
      Serial.println(F("0: Quit"));
      Serial.println(F("--------------------------------------------------------"));

      Serial.print(F("\n==> Enter numeric option:"));
      Serial.flush();

      // wait for incoming data from keyboard
      while (!Serial.available() && (millis()-previous < timeout));

      // parse incoming bytes
      if (Serial.available() > 0)
      {
         previous = millis();

         // get option number
         option = Serial.read();
         Serial.println(option);
         Serial.flush();

         switch (option)
         {
            case '1':
            // ************
               while(Serial.available() > 0) Serial.read();
               Serial << F("Put SQUID into pH4 buffer solution") << endl;            
               Serial.println(F("Enter 1 to PROCEED or 0 for EXIT "));
               while (!Serial.available());              
               option = Serial.read();        
               if (option == '1') {
                  getTempMeasurement(cfg, data);
                  getPhMeasurement(cfg, data);                   
                  do {
                     delay(500);
                     fLastValue = data.phmV;
                     getTempMeasurement(cfg, data);
                     getPhMeasurement(cfg, data);                    
                     Serial << "\"pHraw\": " << data.phRaw << " , \"pHmV\": " << _FLOAT(data.phmV,6) << " , \"pH\": " << _FLOAT(data.ph,6) << " , "<< endl;
                  } while (fabs(data.phmV - fLastValue) > 0.005);
                  cfg.pHcal4 = data.phmV; // current mV signal
                  cfg.pHcalT = data.t; // current temperature
                  Serial.println(F("Done"));
               }
               previous = millis();
               break;
            case '2':
            // ************
               while(Serial.available() > 0) Serial.read();            
               Serial << F("Put SQUID into pH7 buffer solution") << endl;            
               Serial.println(F("Enter 1 to PROCEED or 0 for EXIT "));
               while (!Serial.available());              
               option = Serial.read();        
               if (option == '1') {
                  getTempMeasurement(cfg, data);
                  getPhMeasurement(cfg, data);                   
                  do {
                     delay(500);
                     fLastValue = data.phmV;
                    getTempMeasurement(cfg, data);
                    getPhMeasurement(cfg, data);                    
                    Serial << "\"pHraw\": " << data.phRaw << " , \"pHmV\": " << _FLOAT(data.phmV,6) << " , \"pH\": " << _FLOAT(data.ph,6) << " , "<< endl;
                  } while (fabs(data.phmV - fLastValue) > 0.005);
                  cfg.pHcal7 = data.phmV;
                  cfg.pHcalT = data.t; // current temperature  
                  Serial.println(F("Done"));                
               }
               previous = millis();                       
               break; 
            case '3':
            // ************
               while(Serial.available() > 0) Serial.read();            
               Serial << F("Put SQUID into pH10 buffer solution") << endl;            
               Serial.println(F("Enter 1 to PROCEED or 0 for EXIT "));
               while (!Serial.available());              
               option = Serial.read();        
               if (option == '1') {
                 getTempMeasurement(cfg, data);
                 getPhMeasurement(cfg, data);                   
                  do {
                     delay(500);
                     fLastValue = data.phmV;
                    getTempMeasurement(cfg, data);
                    getPhMeasurement(cfg, data);                    
                    Serial << "\"pHraw\": " << data.phRaw << " , \"pHmV\": " << _FLOAT(data.phmV,6) << " , \"pH\": " << _FLOAT(data.ph,6) << " , "<< endl;
                  } while (fabs(data.phmV - fLastValue) > 0.005);
                  cfg.pHcal10 = data.phmV;
                  cfg.pHcalT = data.t; // current temperature   
                  Serial.println(F("Done"));               
               } 
               previous = millis();                    
               break;
            case '4':
            // ************
               while(Serial.available() > 0) Serial.read();            
               Serial << F("Put SQUID into 1413uS/cm @25degC Conductivity Standard Solution ") << endl;            
               Serial.println(F("Enter 1 to PROCEED or 0 for EXIT "));
               while (!Serial.available());              
               option = Serial.read();        
               if (option == '1') {
                  getEcMeasurement(cfg, data);                 
                  do {
                     delay(500);
                     fLastValue = data.ecRealRaw;
                    getEcMeasurement(cfg, data);                   
                    Serial << "\"ECraw\": [" << _FLOAT(data.ecRealRaw, 7) << ", " << _FLOAT(data.ecImagRaw, 7) <<  "] , \"ec\": " <<  _FLOAT(data.ec,7) << endl;
                  } while (fabs(data.ecRealRaw - fLastValue) > 10.0);
                  cfg.ecCalRawValue = data.ecRealRaw;
                  cfg.ecCalT = data.t; // current temperature    
                  initializeEC(cfg, data);
                  Serial.println(F("Done"));             
               } 
               previous = millis();                    
               break;               
            case '9':
               saveConfiguration(cfg);
               Serial.println(F("Configuration saved")); 
               iQuit = true;
               break;               
            case '0':
               iQuit = true;
               break;
            default:
               break;
         }
      }

      // Condition to avoid an overflow (DO NOT REMOVE)
      if (millis() < previous) previous = millis();

  } while ((iQuit == false) && (millis()-previous < timeout));
  
  digitalWrite(PIN_5V_ENA, LOW);
  digitalWrite(EN_PH, HIGH);
  
  Serial.println();
}

void UM_SystemConfigMenu(uint32_t timeout)
{
   uint32_t previous = 0;
   char option;
   int i = 0;
   int iQuit = false;
   float fLastValue = 0.0;
   unsigned int duration = 0;
   
  // get current time
  previous = millis();

  do
  {
      while(Serial.available() > 0){
         Serial.read();   
      }
      Serial.println(F("\r\n------- SYSTEM CONFIGURATION -------------------------"));
      Serial.println(F("1: Loop duration"));
      Serial.println(F("2: Buzzer on/off"));    
      Serial.println(F("3: LED on/off"));       
      Serial.println(F("4: Sampling delay"));                 
      Serial.println(F("0: Quit"));
      Serial.println(F("--------------------------------------------------------"));

      Serial.print(F("\n==> Enter numeric option:"));
      Serial.flush();

      // wait for incoming data from keyboard
      while (!Serial.available() && (millis()-previous < timeout));

      // parse incoming bytes
      if (Serial.available() > 0)
      {
         previous = millis();

         // get option number
         option = Serial.read();
         Serial.println(option);
         Serial.flush();

         switch (option)
         {
            case '1':
            // ************
               while(Serial.available() > 0) Serial.read();
               Serial << F("Enter loop duration in seconds, valid range between 1..59") << endl;            
               Serial.println(F("or press any key "));
               while (!Serial.available());              
               duration = Serial.parseInt();        
               if (duration >= 1 && duration < 60) {
                  cfg.cycleT = duration;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }
               else {
                  Serial.println(F("value out of range"));   
               }
               previous = millis();
               break;
            case '2':
            // ************
               while(Serial.available() > 0) Serial.read();
               Serial << F("Set buzzer ON/OFF") << endl;   
               Serial.println(F("0: OFF - 1: ON"));         
               Serial.println(F("or press any key "));
               while (!Serial.available());              
               option = Serial.parseInt();        
               if (option == 0) {
                  cfg.buzzerOff = 1;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }
               else if (option == 1) {
                  cfg.buzzerOff = 0;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }               
               else {
                  Serial.println(F("value out of range"));   
               }
               previous = millis();
               break;
            case '3':
               // ************
               while(Serial.available() > 0) Serial.read();
               Serial << F("Set LED ON/OFF") << endl;
               Serial.println(F("0: OFF - 1: ON"));
               Serial.println(F("or press any key "));
               while (!Serial.available());
               option = Serial.parseInt();
               if (option == 0) {
                  cfg.ledOff = 1;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }
               else if (option == 1) {
                  cfg.ledOff = 0;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }
               else {
                  Serial.println(F("value out of range"));
               }
               previous = millis();
               break;
            case '4':
            // ************
               while(Serial.available() > 0) Serial.read();
               Serial << F("Enter sampling delay between measurements in milliseconds, valid range between 0..100") << endl;            
               Serial.println(F("or press any key "));
               while (!Serial.available());              
               duration = Serial.parseInt();        
               if (duration >= 0 && duration <= 1006) {
                  cfg.samplingDelay = duration;
                  saveConfiguration(cfg);
                  Serial.println(F("Done"));
               }
               else {
                  Serial.println(F("value out of range"));   
               }
               previous = millis();
               break;    
            case '9':
               saveConfiguration(cfg);
               Serial.println(F("Configuration saved")); 
               iQuit = true;
               break;               
            case '0':
               iQuit = true;
               break;
            default:
               break;
         }
      }

      // Condition to avoid an overflow (DO NOT REMOVE)
      if (millis() < previous) previous = millis();

  } while ((iQuit == false) && (millis()-previous < timeout));

  Serial.println();
}

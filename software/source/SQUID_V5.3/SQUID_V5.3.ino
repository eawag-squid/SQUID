/*! \SQUID_V5.3.ino
 *  \Main program, initializes the SQUID and contains the control 
 *  \finite state machine
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
  * For this part we used a libraries from 
 *  https://github.com/MrLight/DualOptiboot/tree/master/optiboot
 *  https://github.com/LowPowerLab/DualOptiboot 
 *  http://github.com/JChristensen/DS3232RTC
 *  http://adafruit.com
*/
#include <Arduino.h>

#include <Wire.h>
#include <SPI.h>

#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <Adafruit_ADS1015.h>
#include <Streaming.h>

#include "squidpins.h"
#include "Data.h"
#include "LEDs.h"
#include "pHORPTBatt.h"
#include "Configuration.h"
#include "RTC.h"
#include "EC.h"
#include "usermenu.h"


// types
enum SquidState {
   INIT = 0,
   NO_SD_CARD,
   FILE_NAME_ERROR,
   FILE_OPEN_ERROR,
   BATTERY_LOW,
   RTC_INIT_FAIL,
   RTC_CONFIG_FAIL,
   EC_INIT_FAIL,
   UNKNOWN_ERROR,
   SQUID_RUN
};

// local variables
char buf[512];
char serialNumber[7];
Configuration cfg;
SquidData data;
bool firstLoop = true;
int loopCounter = 0;
SquidState squidstate = INIT;
bool rtcOk = false;
// software version
int majorversion = 5;
int minorversion = 3;

// function prototypes
void getSerialNumber( void );
int SerialWaitResponse (char * pBuffer, int iMaxlen, uint32_t timeout);
int InitializeBluetooth (void);


/**********************************************************************************/
void setup() {

   pinMode(SHUTDOWN_N, OUTPUT);
   digitalWrite(SHUTDOWN_N, LOW);   // low means enabled!
   squidstate = SQUID_RUN; // set the default state, gets overwritten, if something goes wrong during init

   // init serial comm.
   Serial.begin(57600);
   while (!Serial) {
      delay(1);
   }
   
   InitializeBluetooth();

   // display the start message and the serial number
   Serial << endl << "***************  SQUID ***************" << endl;
   getSerialNumber();
   Serial << "Serial Number: " << serialNumber << endl; 
   Serial << "Software Version: " << majorversion << "." << minorversion << endl;
   Serial << "Build date and time: " << __DATE__ << " " << __TIME__ << endl << endl;   
   Serial << "**************************************" << endl;

   Serial << "Initializing LED/Buzzer...  ";  
   stripA.begin();
   pinMode(PIN_5V_ENA, OUTPUT);
   pinMode(BUZ_PIN, OUTPUT);
   pinMode(LED_PIN, OUTPUT);
   runSignalInitSequence();
   Serial << "DONE." << endl;

   Serial << "Checking battery...  ";  
   if( (showBatteryStatus() >= 3) ) {
      Serial << "DONE." << endl;
   } else {
      squidstate = BATTERY_LOW;
      Serial << "too low, charge battery!" << endl;
      return;
   }

   Serial << "Initializing SD card...  ";
   if( initializeSdCard() ) {
      Serial << "DONE." << endl;
   } else {
      squidstate = NO_SD_CARD;
      Serial << "NOK. No SD card detected." << endl;
      return;
   }

   Serial << "Initializing RTC... "; 
   if( initializeRtc() ) {
      rtcOk = true;
      Serial << " DONE." << endl;
      t = now();
      getCurrentTimeInText(t, buf);
      Serial << "  Current time: " << buf << endl;
   } else {
      squidstate = RTC_INIT_FAIL;
      Serial << " NOK. RTC cannot initialize." << endl;
   }

   Serial << "Reading RTC configuration... ";
   readRtcConfigurationFromFile();
   Serial << " DONE." << endl;

   Serial << "Setting SQUID default config... ";
   setDefaultConfiguration(cfg);
   Serial << "DONE." << endl;

   Serial << "Reading the SQUID config file... ";
   readConfigurationFromFile(cfg);
   Serial << "DONE." << endl;

   Serial << "Creating valid data file name... ";
   if( createValidDataFileName( datafilename ) ) {
      Serial << "OK. File name: " << datafilename << endl;
   } else {
      squidstate = FILE_NAME_ERROR;
      Serial << "NOK. Cannot create valid file name" << endl;
      return;    
   }

   Serial << "Initializing the data file... ";    
   if( initializeDataFileWithHeader(serialNumber, majorversion, minorversion, cfg) ) {
      Serial << "OK. Software version written: " << majorversion << "." << minorversion << endl;
   } else {
      Serial << "NOK." << endl;
      squidstate =  FILE_OPEN_ERROR;
      return;
   }

   Serial << "Initializing T... ";
   // nothing to do here
   Serial << "OK. " << endl;

   Serial << "Initializing pH... " ;
   initializePh();
   Serial << "OK. " << endl;
   digitalWrite(EN_PH, HIGH);   // stop pH

   Serial << "Initializing ORP... ";
   initializeOrp();
   Serial << "OK. " << endl;
   digitalWrite(EN_ORP, HIGH);   // stop ORP

   Serial.print("Initializing EC... ");
   if ( initializeEC(cfg, data) ) {
      Serial << "OK." << endl; 
   } else {
      Serial << "NOK." << endl;
      squidstate = EC_INIT_FAIL;
      return;
   }
   
   UM_ShowMenu(3000);

   /* print the header to Console */
   Serial << "Time, Traw, pHraw, ORPraw, EC_Rraw, EC_Iraw, T, pH, ORP, EC" << endl;
}

void loop() {
   t = now();
   int iCmd =0;
   float batteryVoltage = 0.0;

   switch(squidstate) {
      case NO_SD_CARD:
         showNoCardSignal();
         delay(3000);
         Serial << "NO MicroSD Card" << endl;
         break;

      case BATTERY_LOW:
         showBatteryStatus();
         if (showBatteryStatus() >= 3) {
            asm volatile ("  jmp 0");  // restart
         }            
         cfg.cycleT = 5;
         Serial << "Battery Low" << endl;
         break;

      case RTC_INIT_FAIL:
         showRtcInitFailSignal();
         cfg.cycleT = 5;
         Serial << "RTC Init Fail" << endl;
         break;

      case FILE_NAME_ERROR:
         showUnknownErrorSignal();
         cfg.cycleT = 5;
         Serial << "File name fail." << endl;
         break;

      case FILE_OPEN_ERROR:
         showUnknownErrorSignal();
         cfg.cycleT = 5;
         Serial << "File open fail." << endl;
         break;

      case EC_INIT_FAIL:
         showEcInitFailSignal();
         cfg.cycleT = 5;
         Serial << "File open fail." << endl;
         break;

      case UNKNOWN_ERROR:
         showUnknownErrorSignal();
         cfg.cycleT = 5;
         Serial << "Unknown Error" << endl;
         break;

   case SQUID_RUN:
      showStartMeasurementCycleSignal(cfg); 
      getCurrentTimeInText(t, buf);
      Serial << "\"Time\": \"" << buf  << "\" , ";

      digitalWrite(PIN_5V_ENA, HIGH);
      getTempMeasurement(cfg, data);
    digitalWrite(EN_PH, LOW);
      Serial << "\"Traw\": " << data.tempRaw << " , \"T\": " << data.t << " , ";

      if( cfg.phEna == 1 ) {
         getPhMeasurement(cfg, data);
         Serial << "\"pHraw\": " << data.phRaw << " , \"pHmV\": " << _FLOAT(data.phmV,6) << " , \"pH\": " << _FLOAT(data.ph,6) << " , ";
      } 
      else {
         data.phRaw = -9999;
         data.phmV = 0.0;   
         data.ph = data.phRaw;
         Serial << "\"pHraw\": off , \"pH\": off, ";
      }
    digitalWrite(EN_PH, HIGH);
    delay(10);  
    digitalWrite(EN_ORP, LOW);
      if( cfg.orpEna == 1 ) {
         getOrpMeasurement(cfg, data);
         Serial << "\"ORPraw\": " << data.orpRaw << " , \"orpmV\": " << _FLOAT(data.orpmV,6) << " , \"orp\": " << data.orp << " , ";
      } 
      else {
         data.orpRaw = -9999;
         data.orpmV = 0.0;
         data.orp = data.orpRaw;
         Serial << "\"orpRaw\": off , \"orp\": off, ";
      }
    digitalWrite(EN_ORP, HIGH);
    
    digitalWrite(PIN_5V_ENA, LOW);

      if( cfg.ecEna == 1 ) {
         if( !getEcMeasurement(cfg, data) ) {
            data.ecRealRaw = -2;
            data.ecImagRaw = -2;
            Serial << "\"ECraw\": [" << data.ecRealRaw << ", " << data.ecImagRaw <<  "] , \"ec\": nok" << " , ";
         } 
         else {
            Serial << "\"ECraw\": [" << _FLOAT(data.ecRealRaw, 7) << ", " << _FLOAT(data.ecImagRaw, 7) <<  "] , \"ec\": " <<  _FLOAT(data.ec,7) << " , ";
         }
      } 
      else {
        data.ecRealRaw = -1;
        data.ecImagRaw = -1;
        Serial << "\"ECraw\": [" << data.ecRealRaw << ", " << data.ecImagRaw <<  "] , \"ec\": off" << " , ";
      }
      batteryVoltage = getBatteryVoltage();
      Serial << "\"VBat\": \"" << _FLOAT(batteryVoltage,2);
      Serial << endl;
      Serial.flush();
      saveDataPointToFile(t, data, batteryVoltage);
      break;

   default:
      delay(500);
      break;    

   }
  
   if( rtcOk ) {
      /* go to sleep */
      if( firstLoop ) {
         t = t + 2; // two extra seconds on first loop, otherwise loop timing misses one minute
         firstLoop = false;
      }
      if( squidstate == SQUID_RUN ) {
         computeNextWakeUp(t, cfg, true );
      } 
      else {
         computeNextWakeUp(t, cfg, false );
      }
      setRtcWakeUp();
      detachInterrupt(0);      // disables interrupt 0 on pin 2 so the

    // wakeUpNow code will not be executed    
   }
  
    iCmd = UM_CheckforCmd();
   switch (iCmd) {
      case -1:
         UM_ShowMenu(30000);
         firstLoop = true;  // two extra seconds on first loop, otherwise loop timing misses one minute
         break;
      case -2:
         //squidstate = SQUID_RUN;
         break;             
      default:
         break;
   }
  
} // main loop 


/* get the serial number from EEPROM */
void getSerialNumber() {
   int addr = 0;
   for(int i = 0; i<6; i++) {
      serialNumber[i] = EEPROM.read(addr);
      addr++;    
   }
   serialNumber[6] =  '\0';
}

int SerialWaitResponse (char * pBuffer, int iMaxlen, uint32_t timeout) {
   
   uint32_t previous;
   uint8_t i = 0;
   
   previous = millis();
   while (!Serial.available() && (millis()-previous < timeout))
   {
      if (millis() < previous) previous = millis();
   }
   while (Serial.available() > 0 && i < iMaxlen  ) {
      pBuffer[i++]=Serial.read();
      delay(10);
   }
   return i;
}

int InitializeBluetooth (void) {
   
   uint32_t previous;
   #define BUF_LEN  64
   
   char inputBuffer [BUF_LEN] = {NULL};
   unsigned int i=0;
   volatile int iRetValue = 0;
   
   // init serial comm.
   pinMode(A5, OUTPUT);
   pinMode(A4, INPUT_PULLUP);
   digitalWrite(A5, LOW);  // low means enabled!
   delay(5);
   int iBTConfigRequest = digitalRead(A4);
   
   // check if configuration is requested. This is done by setting a jumper from pin A4 to A5
   if (iBTConfigRequest == 0) {
      Serial.flush();
      Serial.begin(115200);
      while (!Serial) {
         delay(1);
      }
      while (Serial.available()) { Serial.read();}  // empty input buffer
      Serial.print("---\r\n"); Serial.flush();
      delay(500);
      Serial.print("$$$");
      Serial.flush();    
      iRetValue = SerialWaitResponse (inputBuffer, BUF_LEN, 1000);
      
      // device maybe configured to different baudrate, try again to connect
      if (!iRetValue) {
         Serial.flush();
         Serial.begin(57600);
         while (!Serial) {
            delay(1);
         }
         Serial.println("---"); Serial.flush();
         delay(100);
         Serial.print("$$$");
         Serial.flush();   
         iRetValue = SerialWaitResponse (inputBuffer, BUF_LEN, 1000);    
      }
      
      // when connected, force configuration
      if (iRetValue) {
         Serial.println("S~,3"); Serial.flush();
         SerialWaitResponse (inputBuffer, BUF_LEN, 1000);
         delay(3000);
         Serial.println("SU,57"); Serial.flush();
         SerialWaitResponse (inputBuffer, BUF_LEN, 1000);
         delay(3000);
         getSerialNumber();
         Serial << "SN,SQUID " << serialNumber << "\r\n";  Serial.flush();
         SerialWaitResponse (inputBuffer, BUF_LEN, 1000);
         Serial.println("R,1"); Serial.flush();
         delay(2000);
         Serial.print("---"); Serial.flush();
      }
      
      while (Serial.available()) { Serial.read();}  // empty input buffer
      Serial.flush();
      Serial.begin(57600);
      while (!Serial) {
         delay(1);
      }
   }
}

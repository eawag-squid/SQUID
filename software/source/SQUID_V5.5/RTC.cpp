/*! \RTC.cpp
 *  \Timestamping measurements and sleep/wakeUp handling is 
 *  \realized with a DS3231SN RTC from Maxim integrated
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
  * For this part we used a library from 
 *  http://github.com/JChristensen/DS3232RTC
 */
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <avr/sleep.h>
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson/issues/223, https://github.com/bblanchon/ArduinoJson
#include <Streaming.h>

#include "RTC.h"
#include "Configuration.h"


char rtcFileName[20] = "RTC.CFG";
unsigned int nextWakeUp = 0;
time_t t;
tmElements_t tm;
char nowDate[20] = "YYYY-MM-DD hh:mm:ss";

void printDateTime(time_t t);
void printTime(time_t t);
void printDate(time_t t);
void printI00(int val, char delim);


bool initializeRtc(void){
   bool success = true;
   //******** Initialize RTC DS3231 **************************************************** 
   //setSyncProvider() causes the Time library to synchronize with the
   //external RTC by calling RTC.get() every five minutes by default.
   setSyncProvider(RTC.get);
   if (timeStatus() != timeSet) {
      success = false;
   }
   else {
      success = true;
   }
   return success;
}


/* convert the time stamp into readable form */
void getCurrentTimeInText(time_t t, char* buf){
   short nowYear = 0;
   byte nowMonth = 0;
   byte nowDay = 0;
   byte nowHour = 0;
   byte nowMinute = 0;
   byte nowSecond = 0;

   nowYear = year(t);
   nowMonth = month(t);
   nowDay = day(t);
   nowHour = hour(t);
   nowMinute = minute(t);
   nowSecond = second(t);      
   sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", nowYear, nowMonth, nowDay, nowHour, nowMinute, nowSecond);
}


/* wake up interrupt service routine 
 *  
 */
void wakeUpNow() {
}


/* computes next wake up call so that we keep a constant cycle time */
void computeNextWakeUp(time_t loopStartTime, Configuration cfg, bool squidIsInRunMode ) {
   float phDlyLocal = 0.0, orpDlyLocal = 0.0;
   int minCycleT = 0;
   int loopTime  = 0;
   
   if(cfg.phEna && squidIsInRunMode ) {
      phDlyLocal = cfg.phDly;
   } 
   else {
      phDlyLocal = 0.0;
   }
   
   if(cfg.orpEna && squidIsInRunMode ) {
      orpDlyLocal = cfg.orpDly;
   } 
   else {
      orpDlyLocal = 0.0;
   }

   if(cfg.ecEna &&  squidIsInRunMode ) {
      cfg.ecDly = 0;
   } 
   else {
      cfg.ecDly = 0;
   }

   minCycleT = (unsigned int) ceil(0.5 + phDlyLocal/1000.0 + orpDlyLocal/1000.0 + cfg.buzzerDly + cfg.ecDly + cfg.filterSize*cfg.samplingDelay/1000.0); 
   if (cfg.cycleT < 1 || minCycleT < 1) {
      loopTime = 1;    
   }
   else if(cfg.cycleT < minCycleT ) {
      loopTime = minCycleT;
   } 
   else {
      loopTime = cfg.cycleT;
   }
   nextWakeUp = second(loopStartTime) + loopTime;
   nextWakeUp = nextWakeUp%60;        // default in run mode

}


/* check if we have received a valid time input to set the RTC */
bool checkTime(tmElements_t tm ){
   bool success = true;
   /* year gets checked outside this function */
   if( tm.Month < 1 || tm.Month > 12 ) {
      success = false;
   } else if( tm.Day < 1 || tm.Day > 31 ) {
      success = false;
   } else if( tm.Hour > 23 ) {
      success = false;
   } else if( tm.Minute > 59 ) {
      success = false;
   } else if( tm.Second> 59 ) {
      success = false;
   }
   return success;
}


void readRtcConfigurationFromFile(void) {
   char json[512];
   char buf [256];
   File rtcCfgFile = SD.open(rtcFileName, FILE_READ);
   
   if( rtcCfgFile == 0 ){
      // unable to open config file/does not exist
      Serial << endl<< "  No " << rtcFileName << " found. No change to RTC." << endl;
   } 
   else {
      Serial << endl << "  Reading... " << rtcFileName;
      rtcCfgFile.readBytes(json, sizeof(json));
      rtcCfgFile.close();
      StaticJsonBuffer<JSON_OBJECT_SIZE(14)> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(json);
      Serial << " OK." << endl;
      Serial << "  Parsing RTC config file... " << endl; 

      tm.Year = root ["YY"];
      if (tm.Year >= 100 && tm.Year < 1000) {
         Serial << F("Error: Year must be two digits or four digits!") << endl;
      }
      else {
         if (tm.Year >= 1000)
            tm.Year = CalendarYrToTm(tm.Year);
         else    //(y < 100)
            tm.Year = y2kYearToTm(tm.Year);
      }       
      Serial << "  Year... " << tmYearToY2k(tm.Year) << endl;

      tm.Month = root["MM"];
      Serial << "  Month ... " << tm.Month << endl;

      tm.Day = root["DD"];
      Serial << "  Day ... " << tm.Day << endl;

      tm.Hour = root["hh"];
      Serial << "  Hour ... " << tm.Hour << endl;

      tm.Minute = root["mm"];
      Serial << "  Minute ... " << tm.Minute << endl;

      tm.Second = root["ss"];
      Serial << "  Second ... " << tm.Second << endl;

      int go    = root["GO"];
      Serial << "  Go-Flag... " << go << endl;   
      if( checkTime(tm) && go == 1 ){ 
         time_t setRtcTime = makeTime(tm);    
         Serial << "  Setting rtc... ";
         RTC.set(setRtcTime);
         setTime(t);       
         Serial << " OK." << endl;   
         /* reset the "GO" flag */
         SD.remove("rtc.cfg");
         rtcCfgFile = SD.open(rtcFileName, FILE_WRITE);
         sprintf(buf, "{\"YY\":\"%d\",\"MM\":\"%d\",\"DD\":\"%d\",\"hh\":\"%d\",\"mm\":\"%d\",\"ss\":\"%d\",\"GO\":\"0\"}", tmYearToY2k(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
         rtcCfgFile.println(buf);  
         rtcCfgFile.close();  
         asm volatile ("  jmp 0");  // ebichris, 2019-02-04
      } 
      else {
      Serial <<  "  No change to RTC" << endl;
      }
   }
}

ISR(PCINT3_vect) {                           
   PCICR = PCICR & B11110111; // disable PCIE3 group    
   PCMSK3 = PCMSK3 & B00000000; //disable PCINT24   
   PCIFR = B00000000; // clear all interrupt flags     
}

/*
 *  put the atmega to sleep, save battery
 */
void sleepNow() {
   sleep_bod_disable();
   //do not go to SLEEP_MODE_PWR_DOWN, otherwise PCINT on RXD0 does work reliable
   set_sleep_mode(SLEEP_MODE_STANDBY);   // sleep mode is set here 
   sleep_enable();   

   // enable PCINT on RXD0 
   PCICR = PCICR | B00001000; // enable PCIE3 group           
   PCMSK3 = PCMSK3 | B00000001; //enable PCINT24
   PCIFR = B00000000; // clear all interrupt flags

   attachInterrupt(digitalPinToInterrupt(PIN_WAKEUP),wakeUpNow, LOW); 
         
   sleep_mode();
   sleep_disable();         // first thing after waking from sleep:
                          // disable sleep...
   detachInterrupt(digitalPinToInterrupt(PIN_WAKEUP));  // disables interrupt 0 on pin 2 so the 
                          // wakeUpNow code will not be executed 
                          // during normal running time.                           
}


void setRtcWakeUp() {
   RTC.setAlarm(ALM1_MATCH_SECONDS, nextWakeUp, 0, 0, 0);
   RTC.alarm(ALARM_1); 
   //RTC.alarmInterrupt(ALARM_1, false);  // for debugging
   RTC.alarmInterrupt(ALARM_1, true); 
   delay(5);   // 100 -> 5, ebichris 20190829
   sleepNow();
   RTC.alarmInterrupt(ALARM_1, false);   
   setSyncProvider(RTC.get);   // the function to get the time from the RTC  
}

void RtcSetupMenu()
{
      while(Serial.available() > 0) Serial.read();// empty RX buffer

      Serial.println(F("******* Set RTC ********"));
      Serial.print(F("Current time: "));
      printDateTime(now()); Serial.println();
      Serial.println(F("set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s"));
      Serial.println(F("or press any key "));
      while (!Serial.available())
      ;
      delay(200);
      //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
      if (Serial.available() >= 12) {
         //note that the tmElements_t Year member is an offset from 1970,
         //but the RTC wants the last two digits of the calendar year.
         //use the convenience macros from Time.h to do the conversions.
         int y = Serial.parseInt();
         if (y >= 100 && y < 1000)
         Serial << F("Error: Year must be two digits or four digits!") << endl;
         else {
            if (y >= 1000)
            tm.Year = CalendarYrToTm(y);
            else    //(y < 100)
            tm.Year = y2kYearToTm(y);
            tm.Month = Serial.parseInt();
            tm.Day = Serial.parseInt();
            tm.Hour = Serial.parseInt();
            tm.Minute = Serial.parseInt();
            tm.Second = Serial.parseInt();
            t = makeTime(tm);
            RTC.set(t);        //use the time_t value to ensure correct weekday is set
            setTime(t);
            Serial << F("RTC set to: ");
            printDateTime(t);
            Serial << endl;
            //dump any extraneous input
            while (Serial.available() > 0) Serial.read();
         }
      }
      else {
         while (Serial.available())
         Serial.read();
         Serial << F("RTC not set ");
         Serial << endl;
      }
}               
               
//print date and time to Serial
void printDateTime(time_t t)
{
   printDate(t);
   Serial << ' ';
   printTime(t);
}

//print time to Serial
void printTime(time_t t)
{
   printI00(hour(t), ':');
   printI00(minute(t), ':');
   printI00(second(t), ' ');
}

//print date to Serial
void printDate(time_t t){
   printI00(day(t), 0);
   Serial << " " << monthShortStr(month(t)) << " " << _DEC(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim){
   if (val < 10) Serial << '0';
   Serial << _DEC(val);
   if (delim > 0) Serial << delim;
   return;
}               

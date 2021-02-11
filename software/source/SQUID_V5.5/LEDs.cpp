/*! \LED.cpp
 *  \Methods for display SQUID states with colour LED 
 *  \This work is based on code from Adafruit (see link below)
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
 * This work is based on code from Adafruit. Check out great products and
 * software libaries here:
 * https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels
 *
 */
#include "LEDs.h"
#include "Configuration.h"


Adafruit_NeoPixel stripA = Adafruit_NeoPixel(NUM_PIXEL, LED_PIN, NEO_GRB + NEO_KHZ800);


/* if there is no SD card, signal to the operator
 *  Signal: LED 0 and 2 flash red, then LED 1 and 3 flash red, total
 *  of three times.
 */
void showNoCardSignal(){
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   for(int i = 0; i<3; i++){
      stripA.setPixelColor(0,255,0,0);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,255,0,0);
      stripA.setPixelColor(3,0,0,0);
      stripA.show();
      delay(500);

      stripA.setPixelColor(0,0,0,0);
      stripA.setPixelColor(1,255,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,255,0,0);
      stripA.show();
      delay(500);
   }
   stripA.setPixelColor(0,0,0,0);
   stripA.setPixelColor(1,0,0,0);
   stripA.setPixelColor(2,0,0,0);
   stripA.setPixelColor(3,0,0,0);
   stripA.show();
   digitalWrite(PIN_5V_ENA, LOW);
   delay(50);
}


/* any error not defined by its own signal is routed here.
 *  Signal: red LEDs running in circle.
 */

void showUnknownErrorSignal(void) {
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   for(int j=0; j<2; j++) {
      for(int i=0;i<NUM_PIXEL;i++){
         stripA.setPixelColor(i,255,0,0);
         stripA.show(); 
         delay(100);
         stripA.setPixelColor(i,0,0,0);
         stripA.show(); 
         delay(100);
      }
   }
   digitalWrite(BUZ_PIN, HIGH);
   delay(500);
   digitalWrite(BUZ_PIN, LOW);
   digitalWrite(PIN_5V_ENA, LOW);
   delay(50);
}

/*
 *  Each measurement cycle is started with a audiovisual signal 
 */
void showStartMeasurementCycleSignal(Configuration cfg){
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   if (cfg.buzzerOff == 0) {
      digitalWrite(BUZ_PIN, HIGH);
   }  
   if (cfg.ledOff == 0) {  
      for(int i=0;i<NUM_PIXEL;i++){
         stripA.setPixelColor(i,255,255,255);
      }
      stripA.show(); 
   }    
   delay(200);   // 400 -> 200, ebichris 20190829 
   digitalWrite(BUZ_PIN, LOW);
   for(int i=0;i<NUM_PIXEL;i++){
      stripA.setPixelColor(i,0,0,0);
   }
   stripA.show(); // Initialize all pixels to 'off'
   digitalWrite(PIN_5V_ENA, LOW);
   // delay(50);  // ebichris 20190829
}


/*
 *  if EC init fails, show signal:
 *  
 *  blink LED 0, 1 red then off, then blink LED 2,3 then off, repeat 2 times
 */
void showEcInitFailSignal(void){
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   for(int i = 0; i<3; i++){
      stripA.setPixelColor(0,255,0,0);
      stripA.setPixelColor(1,255,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
      stripA.show();
      delay(500);

      stripA.setPixelColor(0,0,0,0);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,255,0,0);
      stripA.setPixelColor(3,255,0,0);
      stripA.show();
      delay(500);
   }
   stripA.setPixelColor(0,0,0,0);
   stripA.setPixelColor(1,0,0,0);
   stripA.setPixelColor(2,0,0,0);
   stripA.setPixelColor(3,0,0,0);
   stripA.show();
   digitalWrite(PIN_5V_ENA, LOW);
   delay(50);
}



/* start up lights and buzzer */
void runSignalInitSequence(){
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   for(int j=0; j<2; j++) {
      for(int i=0;i<NUM_PIXEL;i++){
         stripA.setPixelColor(i,0,0,255);
         stripA.show(); // Initialize all pixels to 'off'
         delay(20);
         stripA.setPixelColor(i,0,0,0);
         stripA.show(); // Initialize all pixels to 'off'
         delay(20);
      }
   }
   digitalWrite(BUZ_PIN, HIGH);
   delay(200);
   digitalWrite(BUZ_PIN, LOW);
   digitalWrite(PIN_5V_ENA, LOW);
}


void showRtcInitFailSignal(void){
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   for(int i = 0; i<3; i++){
      stripA.setPixelColor(0,0,0,255);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,0,0,255);
      stripA.setPixelColor(3,0,0,0);
      stripA.show();
      delay(500);

      stripA.setPixelColor(0,0,0,0);
      stripA.setPixelColor(1,0,0,255);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,255);
      stripA.show();
      delay(500);
   }
   stripA.setPixelColor(0,0,0,0);
   stripA.setPixelColor(1,0,0,0);
   stripA.setPixelColor(2,0,0,0);
   stripA.setPixelColor(3,0,0,0);
   stripA.show();
   digitalWrite(PIN_5V_ENA, LOW);
   delay(50);
}

/* displays the battery status.
 * Each LED has three states: green, yellow, red givin with 4 LEDs
 * a total of 12 states. The levels are < 3.3 V  then increments of
 * 0.07V and > 4.1V
 */
int showBatteryStatus(){
   float voltageDividerScale = 0.8;
   float minVoltage = 3.3;
   float maxVoltage = 4.1;
   float deltaVoltage = maxVoltage - minVoltage;
   digitalWrite(PIN_5V_ENA, HIGH);
   delay(50);
   int sensorCount = analogRead(BATT_AI_PORT);
   float sensorAnalog = (float)sensorCount * 3.3 / 1024.0/voltageDividerScale;

   /* compute the battery level */
   int level = (int)(((sensorAnalog - minVoltage)/deltaVoltage)*12);
   if(level <= 1){
      stripA.setPixelColor(0,255,0,0);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 2) {
      stripA.setPixelColor(0,255,255,0);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 3) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 4) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,255,0,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 5) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,255,255,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 6) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,0,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 7) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,255,0,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 8) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,255,255,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 9) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,0,255,0);
      stripA.setPixelColor(3,0,0,0);
   } else if (level == 10) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,0,255,0);
      stripA.setPixelColor(3,255,0,0);
   } else if (level == 11) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,0,255,0);
      stripA.setPixelColor(3,255,255,0);
   } else if (level >= 12) {
      stripA.setPixelColor(0,0,255,0);
      stripA.setPixelColor(1,0,255,0);
      stripA.setPixelColor(2,0,255,0);
      stripA.setPixelColor(3,0,255,0);
   }
   stripA.show();
   delay(2500);
   for(int i=0;i<NUM_PIXEL;i++){
      stripA.setPixelColor(i,0,0,0);
   }
   stripA.show(); 
   digitalWrite(PIN_5V_ENA, LOW);
   delay(50);
   return level;
}

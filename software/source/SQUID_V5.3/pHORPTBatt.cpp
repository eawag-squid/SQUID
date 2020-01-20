/*! \pHORPBatt.cpp
 *  \Methods for measurement of pH, ORP, temperature and battery voltage
 *  \Measurement for pH and ORP uses two 16bit Analog-to-Digital 
 *  \Converters ADS1115 from Texas instruments.
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
  * For this part we used a library from Adafruit (Adafruit_ADS1015.h).
 *  Check out this great tutorial here:
 *  https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/downloads
 */
#include "pHORPTBatt.h"
#include "Configuration.h"
#include "Data.h"


Adafruit_ADS1115 orpADC(I2C_ADDRESS_ORP);
Adafruit_ADS1115 pHADC(I2C_ADDRESS_PH);


void initializePh( void ) {
   pinMode(EN_PH, OUTPUT);
   digitalWrite(EN_PH, LOW);   // low means enabled!
   pHADC.setGain(GAIN_ONE);
   pHADC.begin();
}

void initializeOrp( void ) {
   pinMode(EN_ORP, OUTPUT);
   digitalWrite(EN_ORP, LOW);  // low means enabled! 
   orpADC.setGain(GAIN_ONE);
   orpADC.begin();
}


/*
 * get one pH measurement. Adjust settle time/wait time before value is read.
 */
void getPhMeasurement( Configuration cfg, SquidData& data ){

   float sensitivity;  
   float fVref = 0.0;
   float fVadc = 0.0;
   int value_array[10];

   for(int i = 0; i < cfg.filterSize; i++) {
      //Read from the ADC channel selected
      value_array[i] = pHADC.readADC_SingleEnded(1);
   }
   data.phRaw = medianFilterInt(value_array,cfg.filterSize);
   fVadc = data.phRaw * 4.096 / 32768.0; 

   int iVRef = pHADC.readADC_SingleEnded(0);
   fVref = iVRef * 4.096 / 32768.0;  

   data.phmV = (fVadc - fVref * 2.5) / (-1.5 * -6.618);   // INA116 has negative gain

   // The temperature of the water must be between 0 and 100 degC
   if( (data.t < 0)||(data.t > 100)) {
      data.ph = -1.0;
   }
   // The calibration temperature must be between 0 and 100 degC
   if((cfg.pHcalT < 0)||(cfg.pHcalT > 100)) {
      data.ph = -1.0;
   }
   
   // Two ranges calibration
   if (data.phmV > cfg.pHcal7 ) {
      // The sensitivity is calculated using the other two calibration values
      // Asumme that the pH sensor is lineal in the range.
      // sensitivity = pHVariation / volts
      // Divide by 3 = (pH) 7 -  (pH) 4
      sensitivity = (cfg.pHcal4-cfg.pHcal7)/(cfg.pHref7-cfg.pHref4);
      // Add the change in the pH owed to the change in temperature
      //sensitivity = sensitivity + (data.t - cfg.pHcalT)*0.0001984;
      sensitivity = sensitivity * (data.t + 273.15) / (cfg.pHcalT + 273.15);      

      // The value at pH 7.0 is taken as reference
      // => Units balance => 7 (pH) + (volts) / ((pH) / (volts))
      data.ph = cfg.pHref7 + (cfg.pHcal7-data.phmV) / sensitivity;
      //        |         |
      //      (pH 7 voltage - Measured volts) = Variation from the reference
   } 
   else {
      // The sensitivity is calculated using the other two calibration values
      sensitivity = (cfg.pHcal7-cfg.pHcal10) / (cfg.pHref10-cfg.pHref7);
      // Add the change in the pH owed to the change in temperature
      //sensitivity = sensitivity + (data.t - cfg.pHcalT)*0.0001984;
      sensitivity = sensitivity * (data.t + 273.15) / (cfg.pHcalT + 273.15);    

      data.ph = cfg.pHref7 + (cfg.pHcal7-data.phmV)/sensitivity;
  }  
}


/* obtain a single ORP measurement */
void getOrpMeasurement( Configuration cfg, SquidData& data ) {  

   float fVref = 0.0;
   float fVadc = 0.0;
   int value_array[10];  
   int iVRef = 0;

   for(int i = 0; i < cfg.filterSize; i++) {
      //Read from the ADC channel selected
      value_array[i] = orpADC.readADC_SingleEnded(1);
   }  
   data.orpRaw = medianFilterInt(value_array,cfg.filterSize);
   fVadc = data.orpRaw * 4.096 / 32768.0;  

   iVRef = orpADC.readADC_SingleEnded(0);
   fVref = iVRef * 4.096 / 32768.0; 

   data.orpmV = (fVadc - fVref * 2.5) / (-1.5 * 1.417);

   data.orp = data.orpmV;
}


/* obtain a single Temperature measurement */
void getTempMeasurement( Configuration cfg, SquidData& data ) {
   /* read analog value for temp:
   * take 10 measurements and average out
   */
   int value_array[cfg.filterSize];
   for (int i = 0; i < cfg.filterSize; i++) {
      value_array[i]= analogRead(TEMP_AI_PORT);
      delay(cfg.samplingDelay); // ebichris 20190829
   }
   data.tempRaw = medianFilterInt(value_array,cfg.filterSize);
   //data.tempRaw = data.tempRaw / 10;
   data.t = cfg.tempSensor.scale*data.tempRaw + cfg.tempSensor.offset;
}


int medianFilterInt(int *sample, unsigned int sample_size) {
    
  for (int i = 1; i < sample_size; i++) {
      int j = sample[i];
      int k;

      for (k = i - 1; (k >= 0) && (j < sample[k]); k--) {
         sample[k + 1] = sample[k];
      }
      sample[k + 1] = j;
   }  
  
  return sample[sample_size / 2];
}

/* get battery voltage.
 * returns battery voltage in [V]
 */
float getBatteryVoltage(){
   float sensorAnalog = 0.0;
   float voltageDividerScale = 0.8;
   float minVoltage = 3.3;
   float maxVoltage = 4.1;
   float deltaVoltage = maxVoltage - minVoltage;
   //digitalWrite(PIN_5V_ENA, HIGH);
   //delay(20);
   int sensorCount = analogRead(BATT_AI_PORT);
   sensorAnalog = (float)sensorCount * 3.3 / 1024.0/voltageDividerScale;

   return sensorAnalog;
}

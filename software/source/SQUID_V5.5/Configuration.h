/*! \Configuration.h
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
#ifndef _CONFIGURATION_H_
   #define _CONFIGURATION_H_

#define NBR_JSON_OBJECTS_IN_CFG 26

struct FullQuadraticModel {
   float p00 = 0.0;
   float p10 = 0.0;
   float p01 = 0.0;
   float p20 = 0.0;
   float p11 = 0.0;
   float p02 = 0.0;
};

struct LinearModel {
   float scale = 0.0;
   float offset = 0.0;
};


struct Configuration {
   int   phEna = 1;
   int   phDly = 0; 
   int   ecDly = 0;
   int   ecEna = 1;
   unsigned long	ecFrequency = 40000;
   int   ecFrequencyIncrement = 0;
   int   ecNumberOfIncrements = 0;
   int   orpEna = 1;
   int   orpDly = 0;
   int   cycleT = 5; // 5 seconds default cycle time
   int   buzzerOff = 0; 
   int   ledOff = 0;   
   int   samplingDelay = 0;
   unsigned int   filterSize = 10;     
   int   buzzerDly = 0;
   LinearModel tempSensor;
   float pHref4 = 4.0;
   float pHref7 = 7.0;
   float pHref10 = 10.0;
   float pHcal4 = 0.174;
   float pHcal7 = 0.174;
   float pHcal10 = -0.174;    
   float pHcalT= 22.0;
   float orpcalNeg220mV = -0.22;
   float orpcal0 = 0.0;
   float orpcalPos220mV = 0.22;
   float orpcalT = 22.0;
   float ecCalRef = 1372e-6;
   float ecCalRawValue = 575.06;
   float ecCalT = 25.0; //leave at 25°C for Kcell calculation without temperature compensation part.
   float ecCalTalpha = 2.0; //approximation of the temperature compensation factor for different substances (acids, bases, salts, neutral water), leave at 2

   char filename[20] = "SENS_V5.CFG";
};

void readConfigurationFromFile(Configuration& cfg);
void setDefaultConfiguration(Configuration& cfg);
void saveConfiguration(const Configuration& cfg);


#endif // _CONFIGURATION_H_

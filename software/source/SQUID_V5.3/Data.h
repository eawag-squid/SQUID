/*! \Data.h
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
#ifndef _DATA_H_
#define _DATA_H_

#include <SD.h>
#include <Streaming.h>        //http://arduiniana.org/libraries/streaming/
#include <TimeLib.h>
#include "Configuration.h"

extern char datafilename[13];

struct SquidData {
   int   phRaw   = 0;
   int   tempRaw = 0;
   int   orpRaw  = 0; 
   float ecRealRaw = 0; 
   float ecImagRaw = 0; 
   float kCell = 0.78899;	
   float t = 0.0;
   float phmV = 0.0;	
   float ph = 0.0;
   float orpmV = 0.0;  	
   float orp = 0.0;
   float ec = 0.0;
};

bool createValidDataFileName(char* filename);
bool initializeSdCard( void );
bool initializeDataFileWithHeader(char* serialNumber, int majorversion, int minorversion, Configuration cfg );
bool saveDataPointToFile(time_t t, SquidData data, float fBatteryVoltage);

#endif // _DATA_H_ 

/*! \RTC.h
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
#ifndef _RTC_H_
#define _RTC_H_

#include <DS3232RTC.h>        //http://github.com/JChristensen/DS3232RTC
#include <TimeLib.h>

#include "Configuration.h"

#define PIN_WAKEUP 8
#define WAKE_INT 2
#define INT0_PIN 0

extern time_t t;
extern char nowDate[20];
extern unsigned int nextWakeUp;


bool initializeRtc(void);
void readRtcConfigurationFromFile(void);
void sleepNow(void);
void wakeUpNow(void);
bool checkTime(tmElements_t tm );
void setRtcWakeUp();
void computeNextWakeUp(time_t loopStartTime, Configuration cfg, bool squidIsInRunMode );
void getCurrentTimeInText(time_t t, char* buf);
void RtcSetupMenu(void);



#endif // _RTC_H_

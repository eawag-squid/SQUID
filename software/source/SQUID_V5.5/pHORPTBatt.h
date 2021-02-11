/*! \pHORPBatt.h
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
#ifndef _PHORPTBATT_H_
#define _PHORPTBATT_H_

#include "squidpins.h"
#include "Adafruit_ADS1015.h"
#include "Configuration.h"
#include "Data.h"


#define TEMP_AI_PORT 0  /* Analog Input A0 */
#define BATT_AI_PORT 6  /* Analog Input A6 */
#define I2C_ADDRESS_ORP 0x48
#define I2C_ADDRESS_PH  0x49


void getTempMeasurement( Configuration cfg, SquidData& data );
void getPhMeasurement( Configuration cfg, SquidData& data );
void getOrpMeasurement( Configuration cfg, SquidData& data );
void initializePh( void );
void initializeOrp( void );
int medianFilterInt(int *sample, unsigned int sample_size);
float getBatteryVoltage();


#endif // _PHORPTBATT_H_

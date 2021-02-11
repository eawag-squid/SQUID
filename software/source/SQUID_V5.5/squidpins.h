/*! \squidpins.h
 *  \Pin definitions for microcontroller board APMiniMax
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
#ifndef _SQUIDPINS_H_
#define _SQUIDPINS_H_

// pin assigments
#define PIN_5V_ENA          PIN_C6
#define EN_ORP              7
#define EN_PH               6
#define LED_PIN             2
#define BUZ_PIN             3
#define TEMP_AI_PORT        0  // Analog Input A0 
#define BATT_AI_PORT        6  // Analog Input A6 
#define SD_CARD_CHIPSELECT  10
#define SHUTDOWN_N          4

// Addresses 
#define I2C_ADDRESS_ORP     0x48
#define I2C_ADDRESS_PH      0x49


#endif // _SQUIDPINS_H_ 

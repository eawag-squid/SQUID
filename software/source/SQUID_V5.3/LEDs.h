/*! \LED.h
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
#ifndef _LEDS_H_
#define _LEDS_H_

#include "squidpins.h"
#include <Adafruit_NeoPixel.h>
#include "Configuration.h"


#define NUM_PIXEL 4

extern Adafruit_NeoPixel stripA;

void runSignalInitSequence(void);
int  showBatteryStatus(void);
void showRtcInitFailSignal(void);
void showEcInitFailSignal(void);
void showNoCardSignal(void);
void showUnknownErrorSignal(void);
void showStartMeasurementCycleSignal(Configuration cfg);


#endif // _LEDS_H_

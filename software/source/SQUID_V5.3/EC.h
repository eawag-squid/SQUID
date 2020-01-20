/*! \EC.h
 *  \Methods for measurement of electric conductivity using
 *  \Analog Devices famous AD5933 impedance analyzer
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
#ifndef _EC_H_
#define _EC_H_

#include "Data.h"


/* function prototypes */
bool initializeEC( Configuration cfg, SquidData& data );
bool getEcMeasurement(Configuration cfg, SquidData& data );

#endif // _EC_H_

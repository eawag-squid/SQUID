/*! \zmodem_config.h
 * \Utilities for data transfer
 * The implementation of the zmodem protocol for Arduino has a 
 * long history (see below). 
 * We had to adapt some functions so that they work with standard 
 * Arduino libraries
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
#ifndef ZMODEM_CONFIG_H
#define ZMODEM_CONFIG_H

#define Progname F("Arduino ZModem V2.1.2")

// Dylan (monte_carlo_ecm, bitflipper, etc.) - The SparkFun MP3 shield (which contains an SDCard)
// doesn't operate properly with the SDFat library (SPI related?) unless the MP3 library is
// initialized as well.  If you are using a SparkFun MP3 shield as your SDCard interface,
// the following macro must be defined.  Otherwise, comment it out.

//#define SFMP3_SHIELD

#ifndef SFMP3_SHIELD
// Make sure you set this correctly to define the pin where you have connected your SDCard's
// CS pin!
#define SD_SEL 10
#endif

// Serial output for debugging info
#define DSERIAL Serial

// The Serial port for the Zmodem connection
// must not be the same as DSERIAL unless all
// debugging output to DSERIAL is removed
//#define ZSERIAL Serial3
#define ZSERIAL Serial

#ifdef TEENSYDUINO
  #ifndef SERIAL_TX_BUFFER_SIZE
    #define SERIAL_TX_BUFFER_SIZE 64
  #endif
#endif

// Dylan (monte_carlo_ecm, bitflipper, etc.) - Adjust the baud rate to suit your board and needs
#define ZMODEM_SPEED 57600

// Dylan (monte_carlo_ecm, bitflipper, etc.) - For smaller boards (32K flash, 2K RAM) it may only
// be possible to have only one or some of the following 3 features enabled at a time:  1) File manager
// commands (DEL, MD, RD, etc.), 2) SZ (Send ZModem) or 3) RZ (Receive ZModem).  Large boards
// like the Arduino Mega can handle all 3 features in a single sketch easily, but for smaller boards like
// Uno or Nano, it's very tight.  It seems to work okay, but if you don't need the file manager commands,
// or one of send or receive, comment out the associated macro and it'll slim the sketch down some.

// Uncomment the following macro to build a version with file manipulation commands.

#define ARDUINO_SMALL_MEMORY_INCLUDE_FILE_MGR

// Uncomment the following macro to build a version with SZ enabled.

#define ARDUINO_SMALL_MEMORY_INCLUDE_SZ

// Uncomment the following macro to build a version with RZ enabled

#define ARDUINO_SMALL_MEMORY_INCLUDE_RZ


#endif

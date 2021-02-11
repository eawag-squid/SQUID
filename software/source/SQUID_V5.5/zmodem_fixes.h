/*! \zmodem_fixes.h
 * \Utilities for zmodem protocol implementation
 * The implementation of the zmodem protocol for Arduino has a 
 * long history (zmodem.cpp). 
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
/*
	Keep everything for ANSI prototypes.
From: http://stackoverflow.com/questions/2607853/why-prototype-is-used-header-files
*/

#ifndef ZMODEM_FIXES_H
#define ZMODEM_FIXES_H


////////////////////////////////////////////////////////


#define _PROTOTYPE(function, params) function params

#include <SD.h>


#include <string.h>

// Dylan (monte_carlo_ecm, bitflipper, etc.) - changed serial read/write to macros to try to squeeze 
// out higher speed

#define READCHECK
#define TYPICAL_SERIAL_TIMEOUT 1200

#define readline(timeout) ({ char _c; ZSERIAL.readBytes(&_c, 1) > 0 ? _c : TIMEOUT; })
int zdlread2(int);
#define zdlread(void) ({ int _z; ((_z = readline(Rxtimeout)) & 0140) ? _z : zdlread2(_z); })
//#define sendline(_c) ZSERIAL.write(char(_c))
#define sendline(_c) ({ if (ZSERIAL.availableForWrite() > SERIAL_TX_BUFFER_SIZE / 2) ZSERIAL.flush(); ZSERIAL.write(char(_c)); })
#define zsendline(_z) ({ (_z & 0140 ) ? sendline(_z) : zsendline2(_z); })

void sendzrqinit(void);
int wctxpn(const char *name);
#define ARDUINO_RECV
//int wcrx(void);
int wcreceive(int argc, char **argp);

extern int Filcnt;

#define register int

// If this is not defined the default is 1024.
// It must be a power of 2

#ifdef ARDUINO_SMALL_MEMORY
#define TXBSIZE 1024
#else
#define TXBSIZE 1024
#endif

#define sleep(x) delay((x)*1000L)
#define signal(x,y)

// Handle the calls to exit - one has SS_NORMAL
#define SS_NORMAL 0
#define exit(n)

// For now, evaluate it to zero so that it does not
// enter the "if" statement's clause
#define setjmp(...)

#define printf(s, ... ) DSERIAL.println(s);
#define fprintf(...)

// fseek(in, Rxpos, 0)
//#define fseek(fd, pos, rel) sdfile->seekSet(pos)
//#define fclose(c)

// ignore calls to mode() function in rbsb.cpp
#define mode(a)

#define sendbrk()

//extern int Fromcu;
void purgeline(void);

#ifndef UNSL
#define UNSL unsigned
#endif

void flushmo(void);

#endif

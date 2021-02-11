/*! \zmodem.cpp
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

// Arghhh. These three links have disappeared!
// See this page for the original code:
// http://www.raspberryginger.com/jbailey/minix/html/dir_acf1a49c3b8ff2cb9205e4a19757c0d6.html
// From: http://www.raspberryginger.com/jbailey/minix/html/zm_8c-source.html
// docs at: http://www.raspberryginger.com/jbailey/minix/html/zm_8c.html
// The minix files here might be the same thing:
// http://www.cise.ufl.edu/~cop4600/cgi-bin/lxr/http/source.cgi/commands/zmodem/
// This works with Tera Term Pro Web Version 3.1.3 (2002/10/08)
// (www.ayera.com) but TeraTerm only works on COM1, 2, 3 or 4.

// It DOES NOT handle interruptions of the Tx or Rx lines so it
// will NOT work in a hostile environment.
 
/*
  Originally was an example by fat16lib of reading a directory
  and listing its files by directory entry number.
See: http://forum.arduino.cc/index.php?topic=173562.0

  Heavily modified by Pete (El Supremo) to recursively list the files
  starting at a specified point in the directory structure and then
  use zmodem to transmit them to the PC via the ZSERIAL port

  Further heavy modifications by Dylan (monte_carlo_ecm, bitflipper, etc.)
  to create a user driven "file manager" of sorts.
  Many thanks to Pete (El Supremo) who got this started.  Much work remained
  to get receive (rz) working, mostly due to the need for speed because of the
  very small (64 bytes) Serial buffer in the Arduino.

  I have tested this with an Arduino Mega 2560 R3 interfacing with Windows 10
  using Hyperterminal, Syncterm and TeraTerm.  All of them seem to work, though
  their crash recovery (partial file transfer restart) behaviours vary.
  Syncterm kicks out a couple of non-fatal errors at the beginning of sending
  a file to the Arduino, but appears to always recover and complete the transfer.

  This sketch should work on any board with at least 30K of flash and 2K of RAM.
  Go to zmodem_config.h and disable some of the ARDUINO_SMALL_MEMORY_* macros
  for maximum peace of mind and stability if you don't need all the features
  (send, receive and file management).

V2.1.2
2018-05-11
  - Fixes for Arduino IDE 1.8.5
  - Attempted to patch for use on Teensy

V2.1
2015-03-06
  - Large scale code clean-up, reduction of variable sizes where they were
    unnecessarily large, sharing variables previously unshared between sz and
    rz, and creative use of the send/receive buffer allowed this sketch to
    BARELY fit and run with all features enabled on a board with 30K flash and
    2K of RAM.  Uno & Nano users - enjoy.
  - Some boards were unstable at baud rates above 9600.  I tracked this back
    to overrunning the SERIAL_TX_BUFFER_SIZE to my surprise.  Added a check
    if a flush() is required both in the help and directory listings, as well
    as the sendline() macro.

V2.0
2015-02-23
  - Taken over by Dylan (monte_carlo_ecm, bitflipper, etc.)
  - Added Serial based user interface
  - Added support for SparkFun MP3 shield based SDCard (see zmodem_config.h)
  - Moved CRC tables to PROGMEM to lighten footprint on dynamic memory (zmodem_crc16.cpp)
  - Added ZRQINIT at start of sz.  All terminal applications I tested didn't strictly need it, but it's
    super handy for getting the terminal application to auto start the download
  - Completed adaptation of rz to Arduino
  - Removed directory recursion for sz in favour of single file or entire current directory ("*") for sz
  - Optimized zdlread, readline, zsendline and sendline
      into macros for rz speed - still only up to 57600 baud
  - Enabled "crash recovery" for both sz and rz.  Various terminal applications may respond differently
      to restarting partially completed transfers; experiment with yours to see how it behaves.  This
      feature could be particularly useful if you have an ever growing log file and you just need to
      download the entries since your last download from your Arduino to your computer.

V1.03
140913
  - remove extraneous code such as the entire main() function
    in sz and rz and anything dependent on the vax, etc.
  - moved purgeline, sendline, readline and bttyout from rz to zm
    so that the the zmodem_rz.cpp file is not required when compiling
    sz 
    
V1.02
140912
  - yup, sz transfer still works.
    10 files -- 2853852 bytes
    Time = 265 secs
    
V1.01
140912
This was originally working on a T++2 and now works on T3
  - This works on a T3 using the RTC/GPS/uSD breadboard
    It sent multiple files - see info.h
  - both rz and sz sources compile together here but have not
    yet ensured that transmit still works.
    
V1.00
130630
  - it compiles. It even times out. But it doesn't send anything
    to the PC - the TTYUSB LEDs don't blink at all
  - ARGHH. It does help to open the Serial1 port!!
  - but now it sends something to TTerm but TTerm must be answering
    with a NAK because they just repeat the same thing over
    and over again.

V2.00
130702
  - IT SENT A FILE!!!!
    It should have sent two, but I'll take it!
  - tried sending 2012/09 at 115200 - it sent the first file (138kB!)
    but hangs when it starts on the second one. The file is created
    but is zero length.
    
  - THIS VERSION SENDS MULTIPLE FILES

*/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <SD.h>

#include "zmodem_config.h"
#include "zmodem.h"
#include "zmodem_zm.h"


//function prototypes
void printDirectory(File root, int numTabs);
int count_files(File root, int* file_count, long* byte_count);
size_t DSERIALprint(const __FlashStringHelper* ifsh);
void help(void );
int count_files(int* file_count, long* byte_count);
void loop(void );

#ifdef SFMP3_SHIELD
#include <SFEMP3Shield.h>
#include <SFEMP3ShieldConfig.h>
#include <SFEMP3Shieldmainpage.h>
SFEMP3Shield mp3;
#endif

//#define error(s) sd.errorHalt(s)

extern int Filesleft;
extern long Totalleft;

File fout;
File root;
//extern File fout;

// Dylan (monte_carlo_ecm, bitflipper, etc.) - This function was added because I found
// that SERIAL_TX_BUFFER_SIZE was getting overrun at higher baud rates.  This modified
// Serial.print() function ensures we are not overrunning the buffer by flushing if
// it gets more than half full.

size_t DSERIALprint(const __FlashStringHelper *ifsh)
{
   PGM_P p = reinterpret_cast<PGM_P>(ifsh);
   size_t n = 0;
   while (1) {
      unsigned char c = pgm_read_byte(p++);
      if (c == 0) break;
      if (DSERIAL.availableForWrite() > SERIAL_TX_BUFFER_SIZE / 2) DSERIAL.flush();
      if (DSERIAL.write(c)) n++;
      else break;
   }
   return n;
}

#define DSERIALprintln(_p) ({ DSERIALprint(_p); DSERIAL.write("\r\n"); })

void help(void)
{
   DSERIALprint(Progname);
   DSERIALprint(F(" - Transfer rate: "));
   DSERIAL.flush(); DSERIAL.println(ZMODEM_SPEED); DSERIAL.flush();
   DSERIALprintln(F("Available Commands:")); DSERIAL.flush();
   DSERIALprintln(F("HELP     - Print this list of commands")); DSERIAL.flush();
   DSERIALprintln(F("DIR      - List files in current working directory - alternate LS")); DSERIAL.flush();
   //DSERIALprintln(F("PWD      - Print current working directory")); DSERIAL.flush();
   //DSERIALprintln(F("CD       - Change current working directory")); DSERIAL.flush();
#ifdef ARDUINO_SMALL_MEMORY_INCLUDE_FILE_MGR
   DSERIALprintln(F("DEL file - Delete file - alternate RM")); DSERIAL.flush();
   DSERIALprintln(F("MD  dir  - Create dir - alternate MKDIR")); DSERIAL.flush();
   DSERIALprintln(F("RD  dir  - Delete dir - alternate RMDIR")); DSERIAL.flush();
#endif
#ifdef ARDUINO_SMALL_MEMORY_INCLUDE_SZ
   DSERIALprintln(F("SZ  file - Send file from Arduino to terminal (* = all files)")); DSERIAL.flush();
#endif
#ifdef ARDUINO_SMALL_MEMORY_INCLUDE_RZ  
   DSERIALprintln(F("RZ       - Receive a file from terminal to Arduino (Hyperterminal sends this")); DSERIAL.flush();
   DSERIALprintln(F("              automatically when you select Transfer->Send File...)")); DSERIAL.flush();
#endif
   DSERIALprintln(F("EXIT     - Quit ZModem Mode")); DSERIAL.flush();
}


//dir_t *dir ;

// Dylan (monte_carlo_ecm, bitflipper, etc.) - The way I made this sketch in any way operate on
// a board with only 2K of RAM is to borrow the SZ/RZ buffer for the buffers needed by the main
// loop(), in particular the file name parameter and the SdFat directory entry.  This is very
// unorthodox, but now it works on an Uno.  Please see notes in zmodem_config.h for limitations

//#define name (&oneKbuf[512])
#define dir ((dir_t *)&oneKbuf[256])

void zModemInit()
{
   // NOTE: The following line needs to be uncommented if DSERIAL and ZSERIAL are decoupled again for debugging
   //  DSERIAL.begin(115200);

   //ZSERIAL.begin(ZMODEM_SPEED);   //TODO: do not initialize the serial again here!
   //ZSERIAL.setTimeout(TYPICAL_SERIAL_TIMEOUT);

   //  DSERIALprintln(Progname);
  
   //  DSERIALprint(F("Transfer rate: "));
   //  DSERIALprintln(ZMODEM_SPEED);

   //Initialize the SdCard.
   //if(!SD.begin(SD_SEL)) DSERIALprintln(F("initializing SdCard failed")); 
  
   root = SD.open("/");
  
   #ifdef SFMP3_SHIELD
   mp3.begin();
   #endif

   root.rewindDirectory();

   help();
 
}

void printDirectory(File root, int numTabs) {
    
    while (true) {

        File entry =  root.openNextFile();
        if (! entry) {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++) {
            DSERIAL.print('\t');
        }
        DSERIAL.flush(); DSERIAL.print(entry.name()); DSERIAL.flush();
        if (entry.isDirectory()) {
            DSERIAL.println("/");
            printDirectory(entry, numTabs + 1);
        } 
        else {
           // files have sizes, directories do not
           DSERIAL.print("\t\t");
           DSERIAL.println(entry.size(), DEC);
        }
        entry.close();
    }
    DSERIAL.flush();
}

int count_files(File root, int *file_count, long *byte_count)
{
   while (true) {
      File entry =  root.openNextFile();
      if (! entry) {
          // no more files
          break;
      }
      
      if (entry.isDirectory()) {
          continue;// do only count files in the current directory. instead of all files ->: //count_files(entry, file_count, byte_count);
      }      
      else {
          *file_count = *file_count + 1;
          *byte_count = *byte_count + entry.size();
      }  
      entry.close();   
   }
  
return 0;
}

void zModemRun()
{
   char *cmd = oneKbuf;
   char *param;
   bool exitZmodemLoop = false;
   uint32_t timeout = 20000; 
   uint32_t previous = 0;   
  
   // get current time
   previous = millis();

   while(!exitZmodemLoop){
      char *cmd = oneKbuf;
      char *param;
      char c = 0;           
      *cmd = 0;
 
      while (DSERIAL.available()) DSERIAL.read();

      previous = millis();
    

      while(millis()-previous < timeout) {
         if (DSERIAL.available() > 0) {
            c = DSERIAL.read();
            if ((c == 8 or c == 127) && strlen(cmd) > 0) cmd[strlen(cmd)-1] = 0;
            if (c == '\n' || c == '\r') break;
            DSERIAL.write(c);
            if (c != 8 && c != 127) strncat(cmd, &c, 1);
            previous = millis();
         } 
         else {
            // Dylan (monte_carlo_ecm, bitflipper, etc.) -
            // This delay is required because I found that if I hard loop with DSERIAL.available,
            // in certain circumstances the Arduino never sees a new character.  Various forum posts
            // seem to confirm that a short delay is required when using this style of reading
            // from Serial
            delay(20);
         }
         // Condition to avoid an overflow (DO NOT REMOVE)
         if (millis() < previous) previous = millis();
      }

      // auto EXIT 
      if ((millis()-previous) >= timeout) {
         exitZmodemLoop = true;
      }
        
      param = strchr(cmd, 32);
      if (param > 0) {
         *param = 0;
         param = param + 1;
         } else {
         param = &cmd[strlen(cmd)];
      }

      strupr(cmd);
      DSERIAL.println();
      //  DSERIALprintln(command);
      //  DSERIALprintln(parameter);

      if (!strcmp_P(cmd, PSTR("EXIT"))) {
         exitZmodemLoop = true;
      }

      if (!strcmp_P(cmd, PSTR("HELP"))) {
         
         help();
         
      } 
      else if (!strcmp_P(cmd, PSTR("DIR")) || !strcmp_P(cmd, PSTR("LS"))) {
         DSERIALprintln(F("Directory Listing:"));
         root.rewindDirectory();
         printDirectory(root, 0);
         DSERIALprintln(F("End of Directory"));
         
         //} else if (!strcmp_P(cmd, PSTR("PWD"))) {
         //sd.vwd()->getName(name, 13);
         //DSERIALprint(F("Current working directory is "));
         //DSERIAL.flush(); DSERIAL.println(name); DSERIAL.flush();
         //
         //} else if (!strcmp_P(cmd, PSTR("CD"))) {
         //if(!sd.chdir(param, true)) {
         //DSERIALprint(F("Directory "));
         //DSERIAL.flush(); DSERIAL.print(param); DSERIAL.flush();
         //DSERIALprintln(F(" not found"));
         //} else {
         //DSERIALprint(F("Current directory changed to "));
         //DSERIAL.flush(); DSERIAL.println(param); DSERIAL.flush();
         //}
         #ifdef ARDUINO_SMALL_MEMORY_INCLUDE_FILE_MGR
      } 
      else if (!strcmp_P(cmd, PSTR("DEL")) || !strcmp_P(cmd, PSTR("RM"))) {
         if (!SD.remove(param)) {
             DSERIALprint(F("Failed to delete file "));
             DSERIAL.flush(); DSERIAL.println(param); DSERIAL.flush();
         } 
         else {
            DSERIALprint(F("File "));
            DSERIAL.flush(); DSERIAL.print(param); DSERIAL.flush();
            DSERIALprintln(F(" deleted"));
         }
      } 
      else if (!strcmp_P(cmd, PSTR("MD")) || !strcmp_P(cmd, PSTR("MKDIR"))) {       //unusable if no CD function is available...
         if (!SD.mkdir(param)) {
             DSERIALprint(F("Failed to create directory "));
             DSERIAL.flush(); DSERIAL.println(param); DSERIAL.flush();
         } 
         else {
            DSERIALprint(F("Directory "));
            DSERIAL.flush(); DSERIAL.print(param); DSERIAL.flush();
            DSERIALprintln(F(" created"));
         }
      } 
      else if (!strcmp_P(cmd, PSTR("RD")) || !strcmp_P(cmd, PSTR("RMDIR"))) {       //unusable if no CD function is available...
         if (!SD.rmdir(param)) {
             DSERIALprint(F("Failed to remove directory "));
             DSERIAL.flush(); DSERIAL.println(param); DSERIAL.flush();
         } 
         else {
             DSERIALprint(F("Directory "));
             DSERIAL.flush(); DSERIAL.print(param); DSERIAL.flush();
             DSERIALprintln(F(" removed"));
         }
         #endif
         #ifdef ARDUINO_SMALL_MEMORY_INCLUDE_SZ
      } 
      else if (!strcmp_P(cmd, PSTR("SZ"))) {
         //    Filcnt = 0;
         if (!strcmp_P(param, PSTR("*"))) {
            Filesleft = 0;
            Totalleft = 0L;
            uint8_t retries = 200;
             
            root.rewindDirectory();
            count_files(root, &Filesleft, &Totalleft);
            root.rewindDirectory();

            if (Filesleft > 0) {
               ZSERIAL.print(F("rz\r"));
               sendzrqinit();
               delay(200);
                 
               while (Filesleft > 0) {
                  // read next directory entry in current working directory
                  
                  // open file
                  fout =  root.openNextFile();
                  
                  // format file name
                  //SdFile::dirName(dir, name);
                  
                  
                  if (!fout){
                      DSERIALprintln(F("file.open failed"));
                  }
                  else if (!fout.isDirectory()) {
                      if (wcs(fout.name()) == ERROR) {
                          delay(500);
                          fout.close();
                          break;
                      }
                      else {
                          delay(500);
                      }
                  }
                  
                  fout.close();
               }
               saybibi();
            } 
            else {
              DSERIALprintln(F("No files found to send"));
            }
         } 
         else if (!(fout = SD.open(param, FILE_READ))) {
             DSERIALprintln(F("file.open failed"));
         } 
         else {
             // Start the ZMODEM transfer
             Filesleft = 1;
             Totalleft = fout.size();
             ZSERIAL.print(F("rz\r"));
             sendzrqinit();
             delay(200);
             wcs(param);
             saybibi();
             fout.close();
         }
         #endif
         #ifdef ARDUINO_SMALL_MEMORY_INCLUDE_RZ
      } 
      else if (!strcmp_P(cmd, PSTR("RZ"))) {
         //    DSERIALprintln(F("Receiving file..."));
         if (wcreceive(0, 0)) {
             DSERIALprintln(F("zmodem transfer failed"));
             } else {
             DSERIALprintln(F("zmodem transfer successful"));
         }
         fout.flush();
         //fout.sync();
         fout.close();
         #endif
      }
   } // end overall while loop
   root.close();
}

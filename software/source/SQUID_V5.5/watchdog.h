#ifndef _WDG_SQUID_WATCHDOG_H_ 
#define _WDG_SQUID_WATCHDOG_H_ 

#include <EEPROM.h>
#include <avr/wdt.h>

#define EEPROM_ADDR_WATCHDOG_COUNTER 1023

uint8_t  watchdog_counter;

void watchdog_init() {
  // Restore the watchdog counter
  watchdog_counter = EEPROM.read(EEPROM_ADDR_WATCHDOG_COUNTER);
  
  // Disable all interrupts
  cli();
  
  // Reset the WDT timer
  wdt_reset();
  
  /*
  WDTCSR configuration:
  WDIE = 1: Interrupt Enable
  WDE = 1: Reset Enable
  WDP3 = 1: For 2000ms Time-out
  WDP2 = 0: For 2000ms Time-out
  WDP1 = 0: For 2000ms Time-out
  WDP0 = 1: For 2000ms Time-out
  */
  
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  
  // Set Watchdog settings:
  WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
  
  // Enable interrupts
  sei();
}

void setwatchdog_for_sleep(){
  // Disable all interrupts
  cli();
  
  // Reset the WDT timer
  wdt_reset();
  
  /*
  WDTCSR configuration:
  WDIE = 1: Interrupt Enable
  WDE = 1: Reset Enable
  WDP3 = 1: For 2000ms Time-out
  WDP2 = 0: For 2000ms Time-out
  WDP1 = 0: For 2000ms Time-out
  WDP0 = 1: For 2000ms Time-out
  */
  
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  
  // Set Watchdog settings:
  WDTCSR = (1<<WDIE) | (0<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0);
  
  // Enable interrupts
  sei();  
}

void watchdog_feed() {
  wdt_reset();
}

void watchdog_delay(unsigned long milliseconds) {
  unsigned long remaining_ms = milliseconds;
  unsigned long current_ms;
  
  do {
    current_ms = (remaining_ms > 100) ? 100 : remaining_ms;
    delay(current_ms);
    watchdog_feed();
    remaining_ms -= current_ms;
  } while(remaining_ms > 0);
}

// The watchdog Interrupt Service Routine
ISR(WDT_vect) {
    watchdog_counter++;
    Serial.println(F("\n\nWATCHDOG!! WATCHDOG!! WATCHDOG!! WAU WAU!!\n\n"));
    EEPROM.write(EEPROM_ADDR_WATCHDOG_COUNTER, watchdog_counter);
}


#endif //_WDG_SQUID_WATCHDOG_H_ 

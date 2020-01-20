#include <Arduino.h>
#include <avr/sleep.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
/* #include "SdFat.h" */
#include <DS3232RTC.h>        //http://github.com/JChristensen/DS3232RTC
#include <Streaming.h>        //http://arduiniana.org/libraries/streaming/
#include <avr/pgmspace.h>
#include "AD5933.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson/issues/223, https://github.com/bblanchon/ArduinoJson
#include <EEPROM.h>


//Beginning of Auto generated function prototypes by Atmel Studio
const int chipSelect = 10;

/* power / pin assigments */
#define PIN_5V_ENA PIN_C6
#define LED_PIN 2
#define BUZ_PIN 3

/* LEDs */
#define NUM_PIXEL 4
Adafruit_NeoPixel stripA = Adafruit_NeoPixel(NUM_PIXEL, LED_PIN, NEO_GRB + NEO_KHZ800);

int addr = 0;

void setup() {

  //******** Power Up VCCIO     **************************************************** 
  //digitalWrite(PIN_VCCIO_ON, LOW);   // 
  //pinMode(PIN_VCCIO_ON, OUTPUT);    
  //digitalWrite(PIN_VCCIO_ON, LOW);   // 
  delay(1000);
  
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only 
  }

  char serial[7] = "17-020";
  for( int i = 0; i< 7; i++) {
      EEPROM.write(addr, serial[i]);
      addr++;
  }
  Serial.println("serial set to EEPROM");

  stripA.begin();
  pinMode(PIN_5V_ENA, OUTPUT);
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  
}

void loop() {
  while(1){
    digitalWrite(PIN_5V_ENA, HIGH);
    delay(100);
    /* two short beeps */
    digitalWrite(BUZ_PIN, HIGH);
    delay(250);
    digitalWrite(BUZ_PIN, LOW);
    delay(250);
    digitalWrite(BUZ_PIN, HIGH);
    delay(250);
    digitalWrite(BUZ_PIN, LOW);

    /* set all pixels to green */
    for(int i=0;i<NUM_PIXEL;i++){
      stripA.setPixelColor(i,0,255,0);
    }
    stripA.show(); 
    delay(2000);
    /* ... and turn them off */
    for(int i=0;i<NUM_PIXEL;i++){
      stripA.setPixelColor(i,0,0,0);
    }
    stripA.show(); // Initialize all pixels to 'off'
    digitalWrite(PIN_5V_ENA, LOW);
    delay(2000);  
  }
}

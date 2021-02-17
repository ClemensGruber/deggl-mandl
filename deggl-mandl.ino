/* 
  DegglMandl
  ----------
  Copyright (C) 2021 by Marc Junker
  
  Den Code kann jeder frei verwenden, ändern und hochladen wo er will, solange er nicht 
  seinen eigenen Namen drüber setzt, oder diesen kommerziell verwertet, beispielsweise
  indem Glas-Zuschraub-Maschinen mit diesem Code versehen und verkauft werden.
                            
  2021-01-27 Marc Junker    | Version 0.1 
                              - initiale Version
  2021-01-29 Marc Junker    | Version 0.1a 
                              - Auslagerung von Variablen-Definitionen in deggl-mandl.h
                                - Definition des Drehmomentes über konstanten max. Strom und 
                                - variablen Nachlauf: afterburner in ms
                                - Konfiguration für 0.96" und 1.3" OLED
  2021-02-05 Clemens Gruber | - kleinere Umformatierungen in den Kommentaren und weitere Kommentare 
  2021-02-06 Marc Junker    | Version 0.1b
                               - unbounced StartSwitch
                               - Temperatursemsoren  
  2021-02-10 Marc Junker    | Version 0.1pio2
                               - Migration zum platfomIO Projekt
                               - Rotary zur Einstellung des Drehmomentes
                               - Drehmoment im EEprom gespeichert/geladen
                               - Auslagerung aller Funktionen in tools.h
                               - Temperatursensor im Gehäuse und am Motor
                      
  

  Todo
  ----
  
  - splash screen logo
  - ev. Einstellung des Drehmomentes von 0-Xms auf 1-100% umstellen 
  - Rotary Switch nutzen.. ;-) -> Menu ?
  
*/

const char versionTag[] = "ver 0.1pio3";

// Größe des Oled Displays. Not defined = 0.96" / defined = 1.3"
#define DISPLAY_BIG      

// Bibliotheken 
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <U8g2lib.h>            // aus dem Arduino-Bibliotheksverwalter, bitte die "U8g2" (ohne Adafruit) von Oliver Kraus installieren
#include <INA219.h>             // aus dem Arduino-Bibliotheksverwalter, bitte die "ArduinoINA219" von John De Cristofaro, ... installieren
#include <DallasTemperature.h>  // aus dem Arduino-Bibliotheksverwalter, bitte die "DallasTemperature" von Miles Burton, ... installieren

#include "vars.h"               // in dieser Datei bitte individuelle Anpassungen vornehmen!
#include "tools.h"

// Display-Parameter 
#ifdef DISPLAY_BIG
  // grosses 1.3" OLED mit SH1106 display driver, 128x64 px, ueber I2C angeschlossen
  U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
#else
  // kleines 0.96" OLED mit SSD1306 display driver, 128x64 px, ueber I2C angeschlossen
  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
#endif

INA219 monitor;
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);



void setup() {
  // Definition der pins
  pinMode(pwmEngine, OUTPUT);  
  analogWrite(pwmEngine, 0);
  pinMode(startSwitch, INPUT_PULLUP);
  pinMode (encoderPinA, INPUT_PULLUP);
  pinMode (encoderPinB, INPUT_PULLUP);
  // INA initialisieren 
  monitor.begin();
  // Temperatursensoren initialisieren
  sensors.begin();
  // OLED initialisieren
  u8x8.begin();
  u8x8.clear();  
  // splash screen fuer Arme ;-)
  u8x8.setCursor(1,1);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f); 
  u8x8.print("DegglMandl");
  u8x8.setCursor(1,4);
  u8x8.print(versionTag);
  attachInterrupt(0, doEncoderA, CHANGE); // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(1, doEncoderB, CHANGE); // encoder pin on interrupt 1 (pin 3)
  afterburner = EEPROM.read(0); 
  encoderPos = afterburner;
  afterburnerOld = afterburner;
  delay(3000);  // 3 Sekunden anzeigen  
  u8x8.clear();
  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  buttonState = HIGH;
  lastDebounceTime = millis();
}


void loop() {
  //rotating = true;  // reset the debouncer
  // Hebel runtergezogen //////////////////////////////////////////////////////////////////
  if (unbouncedStartSwitch() == LOW) { 
    // Motor einschalten
    analogWrite(pwmEngine, rpmPWM); 
    if (afterburner != afterburnerOld) {
       EEPROM.write(0,afterburner);
       afterburnerOld = afterburner;
    }
    // Display-Ausgabe Status
    u8x8.setCursor(1,3);
    u8x8.print("Working");    
    delay(50);    // hohen Anlaufstrom für 50ms ignorieren
        
    // volle Fahrt bis maximaler Strom gemessen wird oder Hebel in Null-Stellung kommt /////
    current = (int)(monitor.shuntCurrent() * 1000);
    while ( (current < torqCurrent) && (unbouncedStartSwitch() == LOW)) {
      current = (int)(monitor.shuntCurrent() * 1000);
    }    
    delay(afterburner);  // Nachlauf in ms, bevor der Motor abgeschaltet wird
    // Motor ausschalten
    currentAfterburner = (int)(monitor.shuntCurrent() * 1000);
    analogWrite(pwmEngine, 0);
    
    // max. Strom wurde nicht erreicht, passiert z.B. bei einem zu schwachen Netzteil! /////
    if (current < torqCurrent) {
      // Display-Ausgabe Fehlermeldung
      u8x8.setCursor(1,3);
      u8x8.print("Broke !");     
    }
    
    // max. Strom wurde erreicht, Deckel sollte straff sitzen! /////////////////////////////    
    else {
      // Display-Ausgabe Status
      u8x8.clear();      
      u8x8.inverse();
      u8x8.fillDisplay(); 
      u8x8.setCursor(2,3);
      u8x8.print("Closed ");
      u8x8.setCursor(1,7);
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.print(max(current, currentAfterburner));
      u8x8.print("mA reached");
      u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
 
    }    
    delay(200);
    
    // Bildschirminhalt fuer ca. 3 Sekunden belassen
    while (unbouncedStartSwitch() == LOW) {};
    timeStamp = millis();
    while ( (millis() -2800 ) < timeStamp ) {
      if (unbouncedStartSwitch() == LOW) {
        break;  // Aussteigen, falls der Hebel schon früher gezogen wird 
      }
    }
    u8x8.noInverse();  // Display zuruecksetzen 
    u8x8.clear();    
  }
  else {
  // Warteposition ...  //////////////////////////////////////////////////////////////////    
    // Motor ausschalten
    analogWrite(pwmEngine, 0);
    // Display-Ausgabe Status und aktuelle Einstellungen 
    u8x8.setCursor(1,3);
    u8x8.print(" Ready  ");
    u8x8.setCursor(0,7);
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
    //u8x8.print("torq= ");
    u8x8.print(torqCurrent);
    u8x8.print("mA     ");
    afterburner = encoderPos;
    if (afterburner <10) {u8x8.print("  ");}
    else if (afterburner <100) {u8x8.print(" ");}
    u8x8.print(afterburner);
    //u8x8.print(encoderPos);
    
    u8x8.print("ms");
    
    sensors.requestTemperatures(); 
    u8x8.setCursor(0,0);
    u8x8.print("B ");
    u8x8.print(round(sensors.getTempCByIndex(1)));
    u8x8.print("C      M ");
    u8x8.print(round(sensors.getTempCByIndex(0)));
    u8x8.print("C");
    u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
    
  }
}
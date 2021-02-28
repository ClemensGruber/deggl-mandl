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
  2021-02-16 Marc Junker    | Version 0.1pio3
                               - Einstellung um afterburner erweitert     
  2021-02-22 Marc Junker    | Version 0.1pio4                             
                               - Rotary-Switch: Ready -> config current -> config afterburner -> Ready
  2021-02-22 Marc Junker    | Version 0.2a
                               - lastfreie Motordrehzahl wird nach dem Einschalten kalibriert
                               - Motor fährt mit extrm langsamer Drehzahl an und bschleunigt erst, wenn der Deckel anliegt 
  2021-02-27 Marc Junker    | Version 0.2b
                               - WICHTIG : ############################################
                                           #   Motor PWM Pin von D6 auf D11 umgelegt  #
                                           #   StartSwitch auf D10 umgelegt           #
                                           # ##########################################
                               - PWM Frequenz über Register TCCR2B zwischen 30Hz und 31KHz einstellbar
                               - Minor bugfixes,  torqCurrent und afterburner Min/Max,                                  
  2021-02-28 Marc Junker    | Version 0.2c
                               - Anzahl und Reihenfolge der Temperatursensoren konfigurierbar 


  Todo
  ----
  
  - splash screen logo
  - ev. Einstellung des Drehmomentes von 0-Xms auf 1-100% umstellen 
  - Warung / Abschaltung bei zu hoher Temperatur
  
*/

const char versionTag[] = "ver 0.2c";

// Größe des Oled Displays. Not defined = 0.96" / defined = 1.3"
#define DISPLAY_BIG     

// Positionierung der Temperatursensoren: 0 = aus, 1 der Erste, 2 der Zweite.. ;-) 
// BSP: keine Temperatursensoren ->  {0,0}
//      ein Temperatursensor auf PCB -> {1,0}
//      ein Temperatursensor im Motor -> {0,1}
//      zwei Temperatursensoren, erster auf dem Board, zweiter im Motor -> {1,2}
//      zwei Temperatursensoren, zweiter auf dem Board, erster im Motor -> {2,1}
const int tempSensors[2] = {2,1};

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

  // PWM Frequenz der Motorsteuerung
  //
  // TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  // Definition der pins
  pinMode(pwmEngine, OUTPUT);  
  analogWrite(pwmEngine, 0);
  pinMode(startSwitch, INPUT_PULLUP);
  pinMode (encoderPinA, INPUT_PULLUP);
  pinMode (encoderPinB, INPUT_PULLUP);
  pinMode (outputSW, INPUT_PULLUP);
  pinMode(tacho, INPUT_PULLUP);
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
  torqCurrent = ((EEPROM.read(1) << 0) & 0xFF) + ((EEPROM.read(2) << 8) & 0xFF00);
  if (torqCurrent <1000 || torqCurrent >3000) {torqCurrent = 1200;}
  encoderPos = afterburner;
  afterburnerOld = afterburner;
  u8x8.setCursor(0,7);
  for (int i= 1; i<=rpmVoid; i++) {
    analogWrite(pwmEngine, i);
    delay(10);
  }
  while (( pulseLength< tachoMin) || (pulseLength >tachoMax)) {    /////////////////////////// lastfreie Drehzahl einstellen
    analogWrite(pwmEngine, rpmVoid); 
    delay(10);
    pulseLength = pulseIn(tacho,HIGH);
    u8x8.setCursor(0,7);
    u8x8.print("calibrating");
    //sprintf(tmpBuf, "%3d", (int)pulseLength);
    //u8x8.setCursor(7,7);
    //u8x8.drawString(7, 7, tmpBuf);
    //u8x8.print(tmpBuf);
    //u8x8.print(" ");
    sprintf(tmpBuf, "%3d", rpmVoid);
    //u8x8.setCursor(12,7);
    //u8x8.print(tmpBuf);
    u8x8.drawString(12, 7, tmpBuf);
    delay(200);
    if ((pulseLength >= tachoMax) ||  (pulseLength <= 0)) {
      rpmVoid +=1;
    }
    else if (pulseLength <= tachoMin ) {
      rpmVoid -=1;  
    }
 }
  u8x8.setCursor(0,7);
  u8x8.print("..done !    ");
  analogWrite(pwmEngine, 0); 
  delay(2000);  // 2 Sekunden anzeigen  
  u8x8.clear();
  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  buttonStateStart = HIGH;
  lastDebounceTimeStart = millis();
}


void loop() {
  //rotating = true;  // reset the debouncer
  // Hebel runtergezogen //////////////////////////////////////////////////////////////////
  if ((unbouncedStartSwitch() == LOW) && (configActive == theReady)) { 
    // Motor einschalten
    u8x8.setCursor(1,3);
    u8x8.print("feel..");    
    analogWrite(pwmEngine, rpmVoid); 
    delay(30);  
    do {
      pulseTMP = pulseIn(tacho,HIGH, (pulseLength * 10));
    } while ((pulseTMP < (pulseLength * 3.0)) && (pulseTMP != 0) && (unbouncedStartSwitch() == LOW));
    if (unbouncedStartSwitch() == LOW) {analogWrite(pwmEngine, rpmPWM);} 
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
    timeStampStart = millis();
    while ( (millis() -2800 ) < timeStampStart ) {
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
    switch (configActive) {
      case theCurrent:
        u8x8.print("Config ");
        u8x8.setCursor(0,7);
        u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
        u8x8.inverse();
        u8x8.print(torqCurrent);
        u8x8.print("mA");
        u8x8.noInverse();
        u8x8.print("     ");
        if (afterburner <10) {u8x8.print("  ");}
        else if (afterburner <100) {u8x8.print(" ");}
        u8x8.print(afterburner);
        u8x8.print("ms");
        break;
      case theAfterburner:
        u8x8.print("Config ");
        u8x8.setCursor(0,7);
        u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
        u8x8.print(torqCurrent);
        u8x8.print("mA");
        u8x8.inverse();
        sprintf(tmpBuf, "%3d", afterburner);
        u8x8.drawString(11, 7, tmpBuf);
        u8x8.drawString(14,7,"ms");
        u8x8.noInverse();
        break;
      case theReady:
        u8x8.print(" Ready");
        u8x8.setCursor(0,7);
        u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
        u8x8.print(torqCurrent);
        u8x8.print("mA");
        sprintf(tmpBuf, "%3d", afterburner);
        u8x8.drawString(11, 7, tmpBuf);
        u8x8.drawString(14,7,"ms");
        break;
    }    
    sensors.requestTemperatures(); 
    if (tempSensors[0] !=0) {
      u8x8.setCursor(0,0);
      u8x8.print("B ");
      u8x8.print(round(sensors.getTempCByIndex((tempSensors[0]-1))));
      u8x8.print("C");
    }
    if (tempSensors[1] !=0) {
      u8x8.setCursor(5,0);
      u8x8.print("      M ");
      u8x8.print(round(sensors.getTempCByIndex((tempSensors[1]-1))));
      u8x8.print("C");
    }
    u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
    if (unbouncedRotarySwitch() == LOW) {
      switch (configActive) {
        case theCurrent: configActive = theAfterburner; break;
        case theAfterburner: 
          configActive = theReady; 
          if (afterburner != afterburnerOld) {
            EEPROM.write(0,afterburner);
            afterburnerOld = afterburner;
          }
          if (torqCurrent != torqCurrentOld) {
            EEPROM.write(1, ((torqCurrent >> 0) & 0xFF));
            EEPROM.write(2, ((torqCurrent >> 8) & 0xFF));
            torqCurrentOld = torqCurrent;
          }
          break;
        case theReady: configActive = theCurrent; break;
      }
    while (unbouncedRotarySwitch() == LOW) {;}
    }
  }
}
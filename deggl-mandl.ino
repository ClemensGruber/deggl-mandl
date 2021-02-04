/* DegglMandl


  Copyright (C) 2021 by Marc Junker
  2021-01-27 Marc Junker    Initial version ver 0.1a
                            Den Code kann jeder frei verwenden, ändern und hochladen wo er will,
                            solange er nicht seinen eigenen Namen drüber setzt, oder diesen kommerziell verwertet, beispielsweise
                            indem Etikettiermaschinen mit diesem Code versehen und verkauft werden.
  2021-01-29 Marc Junker    Auslagerung deggl-mandl.h, Definition des Drehmomentes über konstanten max. Strom und variablen Nachlauf:
                            afterburner in ms. Konfiguration für 0.96" und 1.3" Oled.                          


  Todo:
         - Rotary integration: Einstellung des Drehmomentes. (Ev. über den Abschaltstrom in mA oder den Nachlauf in ms)
         - Abspeichern des Drehmomentes im EEProm / laden beim Start
         - splash screen logo 
  
*/

const char versionTag[] = "ver 0.1a";

// Größe des Oled Displays. Not defined = 0.96" / defined = 1.3"
#define DISPLAY_BIG      

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <INA219.h>
#include "deggl-mandl.h"

#ifdef DISPLAY_BIG
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);     // 1.3" Oled
#else
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);  // 0.96" Oled
#endif

INA219 monitor;

void setup() {
   pinMode(pwmEngine, OUTPUT);  
   pinMode(startSwitch, INPUT_PULLUP);
   monitor.begin();
   u8x8.begin();
   u8x8.clear();
   u8x8.setCursor(1,1);
   u8x8.setFont(u8x8_font_amstrad_cpc_extended_f); 
   u8x8.print("Deggl-Mandl");
   u8x8.setCursor(1,4);
   u8x8.print(versionTag);
   delay(3000);
   u8x8.clear();
   u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
}



void loop() {

//////////// Hebel runtergezogen //////////////////////////////////////////////////////////////
  if (digitalRead(startSwitch) == LOW) {           
    analogWrite(pwmEngine, rpmPWM);  
    u8x8.setCursor(1,3);
    u8x8.print("Working");    
    delay(50);    // hohen Anlaufstrom für 50ms ignorieren
//////////// volle Fahrt bis maximaler Strom gemessen wird oder Hebel in o-Stellung kommt /////
    current = monitor.shuntCurrent() * 1000;
    while ( (current < torqCurrent) && (digitalRead(startSwitch) == LOW)) {
        current = monitor.shuntCurrent() * 1000;
    }    
    delay(afterburner);  // Nachlauf in ms, bevor der Motor abgeschaltet wird
    analogWrite(pwmEngine, 0);
/////////// Max Strom wurde nicht erreicht. Passiert zB bei einem zu schwachen Netzteil ! //////   
    if (current < torqCurrent) {
      u8x8.setCursor(1,3);
      u8x8.print("Broke !");     
    }
////////// Max Strom wurde erreicht. Deckel sollte straff sitzen. //////////////////////////////    
    else {
      u8x8.clear();      
      u8x8.inverse();
      u8x8.fillDisplay();
      u8x8.setCursor(1,3);
      u8x8.print("Closed ");
    }    
    delay(200);
////////// Bildschirminhalt für 3sec belassen.     
    while (digitalRead(startSwitch) == LOW) {};
    timeStamp = millis();
    while ( (millis() -2800 ) < timeStamp ) {
        if (digitalRead(startSwitch) == LOW) {
          break;  // Aussteigen, falls der Hebel schon früher gezogen wird /////////////////////
          }
    }
    u8x8.noInverse();
    u8x8.clear();    
  }
  else {
////////// Warteposition....  //////////////////////////////////////////////////////////////////    
    analogWrite(pwmEngine, 0); 
    u8x8.setCursor(1,3);
    u8x8.print("Ready  ");
    u8x8.setCursor(1,7);
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
    //u8x8.print("torq= ");
    u8x8.print(torqCurrent);
    u8x8.print("mA    ");
    u8x8.print(afterburner);
    u8x8.print("ms");
    u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  }
}

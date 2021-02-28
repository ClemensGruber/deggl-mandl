# 1 "/var/folders/dx/kzrywdx92_j344kfzjc76tqr0000gq/T/tmpz0xqpsqf"
#include <Arduino.h>
# 1 "/Users/marc/Documents/GitHub/deggl-mandl/deggl-mandl.ino"
# 57 "/Users/marc/Documents/GitHub/deggl-mandl/deggl-mandl.ino"
const char versionTag[] = "ver 0.2d";


#define DISPLAY_BIG 







const int tempSensors[2] = {2,1};


#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <INA219.h>
#include <DallasTemperature.h>

#include "vars.h"
#include "tools.h"


#ifdef DISPLAY_BIG

  U8X8_SH1106_128X64_NONAME_HW_I2C u8x8( U8X8_PIN_NONE);
#else

  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8( U8X8_PIN_NONE);
#endif

INA219 monitor;
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);
void setup();
void loop();
#line 96 "/Users/marc/Documents/GitHub/deggl-mandl/deggl-mandl.ino"
void setup() {







  TCCR2B = TCCR2B & B11111000 | B00000101;




  pinMode(pwmEngine, OUTPUT);
  analogWrite(pwmEngine, 0);
  pinMode(startSwitch, INPUT_PULLUP);
  pinMode (encoderPinA, INPUT_PULLUP);
  pinMode (encoderPinB, INPUT_PULLUP);
  pinMode (outputSW, INPUT_PULLUP);
  pinMode(tacho, INPUT_PULLUP);

  monitor.begin();

  sensors.begin();

  u8x8.begin();
  u8x8.clear();

  u8x8.setCursor(1,1);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.print("DegglMandl");
  u8x8.setCursor(1,4);
  u8x8.print(versionTag);
  attachInterrupt(0, doEncoderA, CHANGE);
  attachInterrupt(1, doEncoderB, CHANGE);
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
  while (( pulseLength< tachoMin) || (pulseLength >tachoMax)) {
    analogWrite(pwmEngine, rpmVoid);
    delay(10);
    pulseLength = pulseIn(tacho,HIGH);
    u8x8.setCursor(0,7);
    u8x8.print("calibrating");





    sprintf(tmpBuf, "%3d", rpmVoid);


    u8x8.drawString(12, 7, tmpBuf);
    delay(200);
    if ((pulseLength >= tachoMax) || (pulseLength <= 0)) {
      rpmVoid +=1;
    }
    else if (pulseLength <= tachoMin ) {
      rpmVoid -=1;
    }
 }
  u8x8.setCursor(0,7);
  u8x8.print("..done !    ");
  analogWrite(pwmEngine, 0);
  delay(2000);
  u8x8.clear();
  u8x8.setFont(u8x8_font_px437wyse700b_2x2_r);
  buttonStateStart = HIGH;
  lastDebounceTimeStart = millis();
}


void loop() {


  if ((unbouncedStartSwitch() == LOW) && (configActive == theReady)) {

    u8x8.setCursor(1,3);
    u8x8.print("feel..");
    analogWrite(pwmEngine, rpmVoid);
    delay(30);
    do {
      pulseTMP = pulseIn(tacho,HIGH, (pulseLength * 10));
    } while ((pulseTMP < (pulseLength * 3.0)) && (pulseTMP != 0) && (unbouncedStartSwitch() == LOW));
    if (unbouncedStartSwitch() == LOW) {analogWrite(pwmEngine, rpmPWM);}

    u8x8.setCursor(1,3);
    u8x8.print("Working");
    delay(50);

    current = (int)(monitor.shuntCurrent() * 1000);
    while ( (current < torqCurrent) && (unbouncedStartSwitch() == LOW)) {
      current = (int)(monitor.shuntCurrent() * 1000);
    }
    delay(afterburner);

    currentAfterburner = (int)(monitor.shuntCurrent() * 1000);
    analogWrite(pwmEngine, 0);

    if (current < torqCurrent) {

      u8x8.setCursor(1,3);
      u8x8.print("Broke !");
    }

    else {

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

    while (unbouncedStartSwitch() == LOW) {};
    timeStampStart = millis();
    while ( (millis() -2800 ) < timeStampStart ) {
      if (unbouncedStartSwitch() == LOW) {
        break;
      }
    }
    u8x8.noInverse();
    u8x8.clear();
  }
  else {


    analogWrite(pwmEngine, 0);

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
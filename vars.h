


// ** Definition der pins
// ----------------------

//#define CALIBRATION

#define startSwitch 10  // Microswitch am Hebelarm
#define pwmEngine 11 // bei MarcW auf 6  
#define tacho 8
#define ONE_WIRE_BUS 5  // Temperatur-Sensoren = 8;
#define buzzer 9  // Pin für den passiven Buzzer

// Rotary Encoder
#define encoderPinA  3 // CLK
#define encoderPinB  2 // DT
#define outputSW 4 // Switch
volatile int encoderPos = 0;  // a counter for the dial
unsigned int lastReportedPos = 1;      // change management
//static boolean rotating=false;       // debounce management
boolean A_set = false;            
boolean B_set = false;

#define tachoMin 4000
#define tachoMax 4500

#define torqCurrentMax 3000
#define torqCurrentMin 1000
#define afterburnerMax 400
#define afterburnerMin 0

int buttonStateStart = HIGH;
int buttonStateRotarySW = HIGH;

char tmpBuf[6];

// Allgemeine Variablen
enum conigActiveEnum { theReady, theCurrent, theAfterburner, theBuzzer };
enum playBuzzerToneEnum { theOK, theError };
conigActiveEnum configActive = theReady;
  

int current = 0;
int currentTMP = 0;
int currentAfterburner = 0;
int afterburnerOld;
int buzzerActive = 1;
int buzzerActiveOld = 1;
int torqCurrent = 1200;     // max. Strom in mA
int torqCurrentOld = 1200;
int afterburner = 80;       // Nachlauf in ms. Definiert das Drehmoment ! 
int rpmPWM = 255;           // PWM für Motordrehzahl unter Last
int rpmRaise = 12;          // Temp rpm Wert, um die Drehzahl schrittweise zu erhöhen
int rpmVoid = 12;           // PWM für Kalibrierung 18
unsigned long pulseTMP;
unsigned long pulseLength;
unsigned long timeStampStart;
unsigned long timeStampRotarySW;
unsigned long lastDebounceTimeStart;
unsigned long lastDebounceTimeRotarySW;
unsigned long debounceDelay = 5;

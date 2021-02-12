


// ** Definition der pins
// ----------------------

#define startSwitch 6  // Microswitch am Hebelarm
#define pwmEngine 10  
#define ONE_WIRE_BUS 5  // Temperatur-Sensoren = 8;

// Rotary Encoder
#define encoderPinA  3 // CLK
#define encoderPinB  2 // DT
#define outputSW 4 // Switch
volatile unsigned int encoderPos = 0;  // a counter for the dial
unsigned int lastReportedPos = 1;      // change management
//static boolean rotating=false;       // debounce management
boolean A_set = false;            
boolean B_set = false;

int buttonState = HIGH;

// Allgemeine Variablen

int current = 0;
int currentAfterburner = 0;
int torqCurrent = 1800;     // max. Strom in mA
int afterburner = 80;       // Nachlauf in ms. Definiert das Drehmoment ! 
int rpmPWM = 255;           // PWM für Motordrehzahl
unsigned long timeStamp;
unsigned long lastDebounceTime;
unsigned long debounceDelay = 5;
int afterburnerOld;
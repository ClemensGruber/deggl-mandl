


// ** Definition der pins
// ----------------------

const int startSwitch = 6;  // Microswitch am Hebelarm
const int pwmEngine = 10;  
const int ONE_WIRE_BUS = 5;  // Temperatur-Sensoren = 8;

// Rotary Encoder
//const int outputA  = 34; // CLK
//const int outputB  = 35; // DT
//const int outputSW = 13;

int buttonState = HIGH;

// Allgemeine Variablen

float current = 0;
float currentAfterburner = 0;
int torqCurrent = 1800;     // max. Strom in mA
int afterburner = 80;       // Nachlauf in ms. Definiert das Drehmoment ! 
int rpmPWM = 255;           // PWM für Motordrehzahl
unsigned long timeStamp;
unsigned long lastDebounceTime;
int debounceDelay = 5;




// ** Definition der pins
// ----------------------

const int startSwitch = 6;  // Microswitch am Hebelarm
const int pwmEngine = 10;  


// Rotary Encoder klein für Steuerung
//const int outputA  = 34; // CLK
//const int outputB  = 35; // DT
//const int outputSW = 13;


// Allgemeine Variablen

float current = 0;
int torqCurrent = 1800;     // max. Strom in mA
int afterburner = 80;       // Nachlauf in ms. Definiert das Drehmoment ! 
int rpmPWM = 255;           // PWM für Motordrehzahl
unsigned long timeStamp;

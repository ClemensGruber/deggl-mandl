


int unbouncedStartSwitch() {
  int tmp = digitalRead(startSwitch);
  if ( (millis() - lastDebounceTimeStart) > debounceDelay) {
    lastDebounceTimeStart = millis();    
    buttonStateStart = tmp;
    return tmp;
  }
  else {
    return buttonStateStart;
  }
}

int unbouncedRotarySwitch() {
  int tmp = digitalRead(outputSW);
  if ( (millis() - lastDebounceTimeRotarySW) > debounceDelay) {
    lastDebounceTimeRotarySW = millis();    
    buttonStateRotarySW = tmp;
    return tmp;
  }
  else {
    return buttonStateRotarySW;
  }
}


void doEncoderA()
{
  //if ( rotating ) delay (1);  // wait a little until the bouncing is done
  if( digitalRead(encoderPinA) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if ( A_set && !B_set ) {
      if ((configActive == theCurrent) && (torqCurrent < torqCurrentMax)) {
        torqCurrent += 10;
      }
      else if ((configActive == theAfterburner) && (afterburner < afterburnerMax)) {
        afterburner +=1;
      }
      else if (configActive == theBuzzer) {
        buzzerActive = 1;
      }
    }
    //rotating = false;  // no more debouncing until loop() hits again
  }
}

void doEncoderB(){
  //if ( rotating ) delay (1);
  if( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set ) {
      if ((configActive == theCurrent) && (torqCurrent > torqCurrentMin)){
        torqCurrent -= 10;
      }
      else if ((configActive == theAfterburner) && (afterburner > afterburnerMin)) {
        afterburner -=1;
      }
      else if (configActive == theBuzzer) {
        buzzerActive = 0;
      }
    }
    //rotating = false;
  }
}


void playBuzzer(playBuzzerToneEnum playThis) {
  switch (playThis) {
   case theOK:
     if (buzzerActive) {tone(buzzer, 1000);} 
     delay(50);       
     noTone(buzzer);   
     delay(50);
     if (buzzerActive) {tone(buzzer, 1000);}
     delay(50);        
     noTone(buzzer);
     break;
   case theError:
     if (buzzerActive) {tone(buzzer, 100);} 
     delay(1000);        
     noTone(buzzer);
     break;
  }
}




int unbouncedStartSwitch() {
  int tmp = digitalRead(startSwitch);
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();    
    buttonState = tmp;
    return tmp;
  }
  else {
    return buttonState;
  }
}


void doEncoderA()
{
  //if ( rotating ) delay (1);  // wait a little until the bouncing is done
  if( digitalRead(encoderPinA) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if (( A_set && !B_set ) && (encoderPos < afterburnerMax))
      encoderPos += 1;
    //rotating = false;  // no more debouncing until loop() hits again
  }
}

void doEncoderB(){
  //if ( rotating ) delay (1);
  if( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( ( B_set && !A_set ) && (encoderPos >0))
      encoderPos -= 1;
    //rotating = false;
  }
}
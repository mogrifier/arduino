/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins 2, 4, and 7 as output for writing to the 4015 ABC channel select pins
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  //the 4015 is 8 channel, but I am only using 4. That just means there will be some 'skipped' lights
  randNumber = random(8);
  //convert this into binary for sending to the output channel selectors
  switch (randNumber) {
    case 0:
      //light up
      digitalWrite(2, LOW);
      digitalWrite(4, LOW);
      digitalWrite(7, LOW);
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH;
      digitalWrite(7, LOW);
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      digitalWrite(2, LOW;
      digitalWrite(4, HIGH;
      digitalWrite(7, HIGH);
      break;
    case 7:
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH;
      digitalWrite(7, HIGH);
      break;
  }




  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(2000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
}
/*
  Blink multiple LEDs (one at a time) by selecting channels on a 4051.
  The selectchannel will allow current to pass from pin 3 to the channel output
  and light up an LED connected there.


*/
int randNumber;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins 2, 4, and 7 as output for writing to the 4051 ABC channel select pins
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  //for inhibit control (all_off)
  pinMode(8, OUTPUT);
  //write data to serial port
  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  //the 4015 is 8 channel, but I am only using 4. That just means there will be some 'skipped' lights
  randNumber = random(8);
  //write the number to serial port
  Serial.println(randNumber);   

  //enable the 4051
  digitalWrite(8, LOW);

  //convert this into binary for sending to the output channel selectors
  switch (randNumber) {
    case 0:
      //light up LED on pin 14
      digitalWrite(2, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(7, LOW);
      break;
    case 3:
      //light led on pin 12
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(7, LOW);
      break;
    case 6:
      //light led on pin 2
      digitalWrite(2, LOW);
      digitalWrite(4, HIGH);
      digitalWrite(7, HIGH);
      break;
    case 7:
    //light led on pin 4
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(7, HIGH);
      break;
    default:
      break;
  }


  delay(100);
  //enable channel not in use to turn all off
  all_off();
}

void all_off() {
  digitalWrite(8, HIGH);
  //blink really fast
  delay(random (10) );
}
/*
  Blink multiple LEDs (one at a time) by selecting channels on a 4051.
  The selectchannel will allow current to pass from pin 3 to the channel output
  and light up an LED connected there.


*/
int randNumber;
float speed;

// the setup function runs once when you press reset or power the board
void setup() {
  analogReference(DEFAULT);
  // initialize digital pins 2, 4, and 7 as output for writing to the 4051 ABC channel select pins
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  //for inhibit control (all_off)
  pinMode(8, OUTPUT);
  //prepare to write data to serial port for troubleshooting
  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  //the 4015 is 8 channel so pick a random channel from 0 to 7.
  //Use binary on pins 2, 4, 7 to specify.
  randNumber = random(8);

  //read voltage from analog input 0; use for control of delay between blinks; value is 0-1023
  speed = analogRead(A1);
  /* It is critical that the Arduino and external circuits share a common ground so the
  analog input can be read properly. The simplest way to do this is use a SINGLE power supply
  for everything. Using multiple power supplies may fail, even if you ground the Arduino to the
  other supply. I know because I had two supplies (wall warts), grounded through the Arduino and
  while I could read the sensor data, I could not control opening and closing the channels. Once I 
  switched to a single power supply everything worked.
  */

  //enable the 4051
  digitalWrite(8, LOW);

  /* Convert this to binary for selecting the output channel 0..7.
  Pins 2-4-7 are in order of the bits (and control lines A B C) from low to high.
  So Low-High-High would be 110 = 6. Yeah, you have to think backwards.
  */
  switch (randNumber) {
    case 0:
      //light up LED on pin 13
      digitalWrite(2, LOW);
      digitalWrite(4, LOW);
      digitalWrite(7, LOW);
      break;
    case 1:
      //light up LED on pin 14
      digitalWrite(2, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(7, LOW);
      break;
    case 2:
      //light up LED on pin 15
      digitalWrite(2, LOW);
      digitalWrite(4, HIGH);
      digitalWrite(7, LOW);
      break;

    case 3:
      //light led on pin 12
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH);
      digitalWrite(7, LOW);
      break;
    case 4:
      //light up LED on pin 1
      digitalWrite(2, LOW);
      digitalWrite(4, LOW);
      digitalWrite(7, HIGH);
      break;
    case 5:
      //light up LED on pin 5
      digitalWrite(2, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(7, HIGH);
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
  }
  //keep on for a little while. Too low a value prevents the sounds from playing.
  delay(130);

  //turn all off. 
  all_off();
}

void all_off() {
  Serial.println(speed);
  //turn all channels of 4051 off
  digitalWrite(8, HIGH);
  //The potentiometer controls how long of delay between the circuit turning on a channel.
  //Really low values may glitch, which is fine!
  delay(speed);
}
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
  //write data to serial port
  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  //the 4015 is 8 channel, but I am only using 4. That just means there will be some 'skipped' lights
  randNumber = random(8);
  //write the number to serial port

  //read voltage from analog input 0; use for control of delay between blinks; value is 0-1023 
  //( mine closer to 800 since 3.8v max)

//analog in reads really poor. almost always high but should not be as I adjust the pot.
  speed = analogRead(A1);
  

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


  delay(200);
  //turn all off
  all_off();
}

void all_off() {
  Serial.println(speed);
  digitalWrite(8, HIGH);
  //speed needs to be a float this this to work
  float calculated_delay = 700 * (speed/1023);
  delay(calculated_delay );
}
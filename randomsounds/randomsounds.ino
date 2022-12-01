const int sendPin = 3;
const int readPin = 2;

int count = 0;
int mod = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(sendPin, OUTPUT);

  pinMode(readPin, INPUT_PULLUP);
  pinMode(readPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(readPin), highInterrupt, CHANGE);

  //prepare to write data to serial port for troubleshooting
  Serial.begin(57600);

  // DDRD = DDRD | B00000100;  // this is safer as it sets pins 2 as output, 7 as input
  // without changing the value of pins 0 & 1, which are RX & TX
}

void loop() {}

void stopCode() {
 // Serial.println("stop");

  for (int j = 0; j < 500; j++) {
    bitSet(PORTD, sendPin);
  }
}


void send() {
    mod++;
  if (mod > 1000000) {
    Serial.println("pulse");
    mod = 0;
  }
  PORTD = B000000;
  bitSet(PORTD, sendPin);
  bitSet(PORTD, sendPin);
  bitClear(PORTD, sendPin);
  bitSet(PORTD, sendPin);
  bitClear(PORTD, sendPin);
  bitSet(PORTD, sendPin);
  bitClear(PORTD, sendPin);
}

/*
void pulse() {

  count++;
  if (count > 10000) {
    Serial.println("pulse");
    count = 0;
  }

  cli();
  //send on pin 2
  PIND = B00000011;
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  delayMicroseconds(4);
  PIND = B00000011;
  sei();
}
*/

void highInterrupt() {

  int count = 0;

  int dataLength = random(500) + 300;
  //send datLength pulses really fast
  for (count = 0; count < dataLength; count++) {
    send();
  }

  stopCode();
  //Serial.println(micros());
}
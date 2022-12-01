#define BUTTON_PIN 2


int pw[50];
int index = 0;


void highInterrupt() {

if (index < 49) {
   pw[index++] = micros();
}

//pin toggling with port manipulation
//PORTD |= 0x10; // Switch on pin 4
//PORTD &= 0xEF; // Switch off pin4


}

void setup() {
  Serial.begin(57600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), highInterrupt, CHANGE);
}

void loop() {
  // do your other stuff here
  Serial.print("time = ");
  Serial.println(micros());
  Serial.println(index);
  delay(1000);

  if (index >= 49) {
    Serial.println("done");

    for (int i = 0; i < 49; i++ ) {

      Serial.println(pw[i + 1] - pw[i]);

    }

      index = 0;
      Serial.println("starting new measurements");
  }


}
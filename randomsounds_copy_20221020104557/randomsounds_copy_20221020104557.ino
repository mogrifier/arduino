#define BUTTON_PIN 2


int pw[500];
int index = 0;


void highInterrupt() {

if (index < 499) {
   pw[index++] = micros();
}

}

void setup() {
  Serial.begin(57600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), highInterrupt, HIGH);
}

void loop() {
  // do your other stuff here
  Serial.print("time = ");
  Serial.println(micros());
  delay(1000);

  if (index >= 500) {
    Serial.println("done");

    for (int i = 0; i < 499; i++ ) {

      Serial.println(pw[i + 1] - pw[i]);

    }

  }


}
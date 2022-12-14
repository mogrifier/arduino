
/*
receive pulse and trigger interrupt on rising edge
*/
int count = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(21, INPUT_PULLUP);
  Serial.begin(57600);
  attachInterrupt(2, gotPulse, RISING);  //digital pin 21
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("merrily we roll along");
  
  delay(20);
}


void gotPulse() {
  Serial.println("**");

    tone(9, 440, 25);
  
}
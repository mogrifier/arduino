/*
play tones. Arduino MEGA 1280.

This is creating really nice and usable sync-like tones. All with with output voice. 
Very worthwhile to put separate analog controls on the variables for freq, modRange, maxMod
and maybe loop delay.
This is a continuous player- run through FX for sure! 
duration should be less than loopdelay

*/

const int SPEAKERPIN = 9;
const int MODPIN = 0;
const int FREQ = 500;
const int LOOPDELAY = 15;
const int MAXMOD = 10;

int count = 0;
int deltaFreq = -5;
int inc = 2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {

  // put your main code here, to run repeatedly:
  int sensor = analogRead(MODPIN);
  int freq = map(sensor, 0, 1023, 100, 3000);  //100 hz is all little 8ohm speaker can get down to??

  //Serial.print("freq = ");
  //Serial.println(freq);
  //int mod = map(sensor, 0, 1023, -4, 4);  //+- 2 hz
  int modRange = map(sensor, 0, 1023, 5, 30);   
  //what should max value be? Need to tie better to freq - make a tunable parameter
  //Serial.print("modrange= ");
  //Serial.println(modRange);
  //programmable modulation
  count++;
  //this is only causing a slow pulse like a saw- NOT the sync sound. By inverting increment you get a phaser
  if (count % modRange == 0) {
    deltaFreq += inc;
    if (deltaFreq >= MAXMOD) {
      inc = -inc;
      //deltaFreq = -MAXMOD;
      Serial.println("max deltaFreq");
    }
    if (deltaFreq <= -MAXMOD) {
      inc = -inc;
      //deltaFreq = -MAXMOD;
      Serial.println("min deltaFreq");
    }
  }

  int duration = 10;  //you can hear down to 11ms or so
  tone(SPEAKERPIN, freq+ deltaFreq , duration);

  // long timeDelay = map(sensor, 0, 1023, 10, 20);

  delay(LOOPDELAY);
}
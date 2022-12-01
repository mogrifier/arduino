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
const int DURATION = 10;  //you can hear down to 11ms or so. combined with LOOPDELAY this creates extra sync sound
const int MAXMOD = 10;
const float TWELFTHROOT = 1.05946;
const float LOWA = 27.5;

int count = 0;
int deltaFreq = -5;
int inc = 3;

//array for pitch and duration (first value is pitch)
int music[2];
//starting point for IFS code. Store past and present for iteration.
float x = 0;
float y = 0;
float next_x = 0;
float next_y = 0;
int note_duration = 0;
int freq = 0;
//numbers of times a given note is played. cycles * duration >= note_duration
int cycles = 0;

//arrays for holding IFS matrix
float a[4];
float b[4];
float c[4];
float d[4];
float e[4];
float f[4];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);

//initial IFS matrix data for a fern
  a[0] = 0;
  a[1] = 0.85;
  a[2] = 0.2;
  a[3] = -0.15;

  b[0] = 0;
  b[1] = 0.04;
  b[2] = -0.26;
  b[3] = 0.28;

  c[0] = 0;
  c[1] = -0.04;
  c[2] = 0.23;
  c[3] = 0.26;

  d[0] = 0.16;
  d[1] = 0.85;
  d[2] = 0.22;
  d[3] = 0.24;

  e[0] = 0;
  e[1] = 0;
  e[2] = 0;
  e[3] = 0;

  f[0] = 0;
  f[1] = 1.6;
  f[2] = 1.6;
  f[3] = 0.44;


}

void loop() {

  // put your main code here, to run repeatedly:
  int sensor = analogRead(MODPIN);
 // int freq = map(sensor, 0, 1023, 100, 3000);  //100 hz is all little 8ohm speaker can get down to??
 
  if (cycles * DURATION >= note_duration) {
    //note has played for at least the amount of milliseconds specified so unlatch it
    //Serial.print("played for ms = ");
    //Serial.println(cycles * DURATION);
    //reset cycle counter
    cycles = 0;
    //get new pitch and duration
    compute_music();
    freq = music[0];
    note_duration = music[1];
    //Serial.println("new note");
  }  
  else {
    //use previous pitch and duration, but the duration has to "count down"
    freq = music[0];
    note_duration = music[1];
  }

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
    }
    if (deltaFreq <= -MAXMOD) {
      inc = -inc;
    }
  }

  tone(SPEAKERPIN, freq + deltaFreq, DURATION);
  // long timeDelay = map(sensor, 0, 1023, 10, 20);
  cycles += 1;
  delay(LOOPDELAY);
}


/*
Get the next pitch and duration from the IFS code. Just compute all as needed.
*/
void compute_music() {

  int k = get_chance();
  next_x = a[k] * x + b[k] * y + e[k];
  next_y = c[k] * x + d[k] * y + f[k];
  x = next_x;
  y = next_y;
  //Serial.println(x);
  //Serial.println(y);

  //the next note to play is next_x with a duration of next_y

  //scale values so in bounds and make sense for pitch frequency and duration in milliseconds
  int scale_x = int(abs(x) *  4100 + 100);
  int piano_key = map(scale_x, 100, 4200, 25, 88);
  int scale_y = int(abs(y) * 500 + random(200, 3000));

  if (scale_x < 4200 && scale_y < 2000) {
      music[0] = get_freq(piano_key);
      music[1] = scale_y;
  }
  else {
    //reset to new starting point for iteration
    x = 0;
    y = 0;
    //recompute
    next_x = a[k] * x + b[k] * y + e[k];
    next_y = c[k] * x + d[k] * y + f[k];
    x = next_x;
    y = next_y;

    //since starting over should be in bounds
    music[0] = scale_x;
    music[1] = scale_y;
  }

}

/*
Choose array indices based on a hard-coded probability distribution.
*/
int get_chance() {
    float r = (float)random(1, 100)/100;
    if (r <= 0.1)
        return 0;
    if (r <= 0.2)
        return 1;
    if ( r <= 0.4)
        return 2;
    else
        return 3;
}

/*
Convert the piano key position (1 to 88) to the corresponding frequency
*/
int get_freq(int key) {
  int octave = (int)(key/12);
  int note = (key % 12) - 1;
  float freq = LOWA * pow(2, octave) * pow(TWELFTHROOT, note);
  return int(freq);
}

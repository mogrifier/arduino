/*
play tones. Arduino MEGA 1280.

This is creating really nice and usable sync-like tones. All with with output voice. 
Very worthwhile to put separate analog controls on the variables for freq, modRange, maxMod
and maybe loop delay.
This is a continuous player- run through FX for sure! 
duration should be less than loopdelay

*/
//#define DEBUG true

const int SPEAKERPIN = 9;
const int MODPIN = 0;
const int FREQ = 500;
const int LOOPDELAY = 15;
const int DURATION = 12;  //you can hear down to 11ms or so. combined with LOOPDELAY this creates extra sync sound
const float TWELFTHROOT = 1.05946;
const float LOWA = 27.5;

//these are not constant since will likely be changeable with a sensor input
int INC_SENSOR = 3;
int MAXMOD_SENSOR = 10;

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
  d[3] = 0.44;

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

  //some variable values need to be saved between calls to loop so declare them as static
  static int cycles;     //for calculating how long a note has played
  static int count;      //
  static int deltaFreq;  //vibrato
  static int duration;   //how long the note will play (minimum)
  static int freq;
  // put your main code here, to run repeatedly:
  int sensor = analogRead(MODPIN);

  if (cycles * DURATION >= duration) {
    //note has played for at least the amount of milliseconds specified so get a new note
    //reset cycle counter
    cycles = 0;
    //compute new freq and duration values
    compute_music(freq, duration);

  #ifdef DEBUG {
      char buffer[40];
      sprintf(buffer, "Pitch %d and duration %d", freq, duration);
      Serial.println(buffer);
  }
  #endif
  }
  else {
    //use previous freq and duration- values is stored since variables are static, but the duration has to "count down"
    cycles += 1;
  }

  int modRange = map(sensor, 0, 1023, 5, 30);  //effectively control range and speed of vibrato
  //what should max value be? Need to tie better to freq - make a tunable parameter
  //Serial.print("modrange= ");
  //Serial.println(modRange);

  //programmable vibrato-like control. This creates a sinusoidal vibrato.
  count++;
  if (count % modRange == 0) {
    //reset count (if you don't it would overflow after many hours)
    count = 0;
    deltaFreq += INC_SENSOR;
    if (deltaFreq >= MAXMOD_SENSOR) {
      INC_SENSOR = -INC_SENSOR;
    }
    if (deltaFreq <= -MAXMOD_SENSOR) {
      INC_SENSOR = -INC_SENSOR;
    }
  }

  tone(SPEAKERPIN, freq + deltaFreq, DURATION);
  // long timeDelay = map(sensor, 0, 1023, 10, 20);
  delay(LOOPDELAY);
}


/*
Get the next pitch and duration from the IFS code. Just compute all as needed.
*/
void compute_music(int &freq, int &duration) {
  static int totalIterations;
  //starting point for IFS code. Store past and present for iteration.
  static float x;
  static float y;
  static float next_x;
  static float next_y;

  totalIterations += 1;
  int k = get_chance();
  next_x = a[k] * x + b[k] * y + e[k];
  next_y = c[k] * x + d[k] * y + f[k];
  x = next_x;
  y = next_y;

  //the next note to play is next_x with a duration of next_y

  //scale values so in bounds and make sense for pitch frequency and duration in milliseconds
  int scale_x = int(abs(x) * 100);
  if (scale_x > 100) {
    scale_x = 100;
  }
  //constrain the piano key range to one the arduino can play and also not too high since unpleasant
  int piano_key = map(scale_x, 0, 100, 25, 74);
  //y has a range up to 3.5 or so
  int scale_y = int(abs(y) * 600 + 400);

  //assign values to the variable references so changes are seen in the loop function
  freq = get_freq(piano_key);
  duration = scale_y;

  if (totalIterations > 100) {
    //reset to new starting point for iteration
    init_xy(x, y);
    totalIterations = 0;
  }
}

/*
Choose array indices based on a hard-coded probability distribution.
*/
int get_chance() {
  float r = (float)random(1, 100) / 100;
  if (r <= 0.1)
    return 0;
  if (r <= 0.2)
    return 1;
  if (r <= 0.4)
    return 2;
  else
    return 3;
}

void init_xy(float &x, float &y) {
  x = (float)random(1, 100) / 100;
  y = (float)random(1, 100) / 100;
}

/*
Convert the piano key position (1 to 88) to the corresponding frequency
*/
int get_freq(int key) {
  int octave = (int)(key / 12);
  int note = (key % 12) - 1;
  float freq = LOWA * pow(2, octave) * pow(TWELFTHROOT, note);
  //round to nearest whole value
  return int(freq + 0.5);
}

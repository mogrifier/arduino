/*
play tones. Arduino MEGA 1280.

This is creating really nice and usable sync-like tones. All with with output voice. 
Very worthwhile to put separate analog controls on the variables for freq, vibratorate, maxMod
and maybe loop delay.
This is a continuous player- run through FX for sure! 
duration should be less than loopdelay

*/

//#define DEBUG 0  //uncomment line to turn on some debug statements

//digital pins for sensing switch position (don't use 0 or 1 since for serial data)
const int SPEAKERPIN = 9;
const int VIBRATOMODEPIN = 5;
const int RANDOMPIN = 6;
const int SYNCONOFFPIN = 4;
const int MIDIONOFFPIN = 7;
const int MIDIINPUTPIN = 14;    //use Tx/Rx 3 for MIDI I/O
const int RECEIVESYNCPIN = 2;   //hardware interrupt
const int GLITCHPIN = 3;        //hardware interrupt
const int TENPOSITIONPIN = 21;  //hardware interrupt
//analog pins for continuous sensors, like potentiometers or other sources of 1-5v
const int VIBRATORATEPIN = 0;
const int ITERATIONPIN = 1;
const int DURATIONPIN = 2;
const int VIBRATODEPTHPIN = 3;

const int FREQ = 500;
const int LOOPDELAY = 15;
const int BURST_DURATION = 12;  //you can hear down to 11ms or so. combined with LOOPDELAY this creates extra sync sound
const float TWELFTHROOT = 1.05946;
const float LOWA = 27.5;
const int MAX_ITERATIONS = 120;

//these are not constant since will likely be changeable with a sensor input
int VAR_ITERATIONS = 100;

//other global variables- minimize use
int arpeggiatorDuration = 500;  //default to 120BPM
byte DEBOUNCE_DELAY = 200;      //use as milliseconds

/*
flag names for state of synthesizer switches. I am using toggle switches. The switch state is
captured in the software. This means the software must track each button press and update the state.
*/
int VIBRATO_FLAG = 0;
int RANDOM_FLAG = 0;
int SYNC_FLAG = 0;
int GLITCH_FLAG = 0;
int MIDI_FLAG = 0;
//flag values
const int SINE_VIBRATO = 0;
const int SQUARE_VIBRATO = 1;
const int OFF = 0;
const int ON = 1;
//MIDI stuff
const byte NOTEOFF = 0x80;
const byte NOTEON = 0x90;

/*
Need buffers to store data for playback of arpeggio patterns from either a saved random
pattern or a set of notes played in via the keyboard. byte uses half memory of int and works fine.
*/
byte kIndex[MAX_ITERATIONS];

//arrays for holding IFS matrix
float a[4];
float b[4];
float c[4];
float d[4];
float e[4];
float f[4];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //MIDI is hooked up to Rx (and Tx, if dsired later) on RX/Tx3, pins 1
  Serial3.begin(31250);
  //vibrato pin; I am using the internal pullup resistor to simplify the circuit
  pinMode(VIBRATOMODEPIN, INPUT_PULLUP);
  //random operation selector
  pinMode(RANDOMPIN, INPUT_PULLUP);
  //5v trigger receive sync
  pinMode(RECEIVESYNCPIN, INPUT_PULLUP);
  //sync on-off pin
  pinMode(SYNCONOFFPIN, INPUT_PULLUP);
  //glitch on-off pin
  pinMode(GLITCHPIN, INPUT_PULLUP);
  //ten position switch pin
  pinMode(TENPOSITIONPIN, INPUT_PULLUP);
  //midi on off pin
  pinMode(MIDIONOFFPIN, INPUT_PULLUP);

  //initialize kIndex
  fill_kIndex();

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

  //initialize initial state to match toggle switch positions
  updateSynthState();
  //use hardware interrupt to receive trigger pulse- format is pin, method, type of pulse
  attachInterrupt(digitalPinToInterrupt(RECEIVESYNCPIN), receiveTrigger, RISING);  //on RECEIVESYNCPIN
  //use hardware interrupt to receive glitch. FALLING worked better for debouncing
  attachInterrupt(digitalPinToInterrupt(GLITCHPIN), glitchTrigger, FALLING);  //on GLITCHPIN
  //use hardware interrupt to activate function for ten position switch. FALLING worked better for debouncing
  attachInterrupt(digitalPinToInterrupt(TENPOSITIONPIN), tenPositionTrigger, FALLING);  //on TENPOSITIONPIN
}

void loop() {

  //some variable values need to be saved between calls to loop so declare them as static
  static int cycles;     //for calculating how long a note has played
  static int count;      //
  static int sineFreq;   //sine vibrato
  static int deltaFreq;  //total vibrato to apply
  static int duration;   //how long the note will play (minimum)
  static int freq;
  static int square_vibrato_sign = 1;
  static int sine_vibrato_sign = 1;
  static int inc_amount = 3;
  //midi variables
  static byte key = 0;
  static byte velocity = 0;
  static byte lastNote;
  int squareFreq;  //square vibrato- calculated, not saved
  // put your main code here, to run repeatedly:

  /*update flags by looking for button presses. I am using toggle switches. Debounce not an issue.
  How you do this depends on the type of switches you use. Momentary means use interrupt, toggle means poll.
  */
  updateSynthState();

  byte midibytes = Serial3.available();
  if (MIDI_FLAG == ON) {
    //are there midi data bytes to read?
    if (midibytes >= 1) {
      //read command byte
      byte command = Serial3.read();
      //filter data for proper reading
      if (command == NOTEOFF || command == NOTEON) {
        //read 2 data bytes
        key = Serial3.read();
        velocity = Serial3.read();
      } else {
        //some other data- flush and ignore for now
        Serial.flush();
      }
      if (command == NOTEON) {
        //got new data so reset the freq value
        freq = get_freq(key);
        lastNote = key;
      } else if (command == NOTEOFF) {
        //turn off the synth only if the key matches the last note
        if (key == lastNote) {
          //received a note off for a matching key
          noTone(SPEAKERPIN);
          freq = 0;
          return;
        }
      }
    }

    //this means the note is on still
    /*
If BURST_DURATION used, you get the dual tone like RM (the lone tone of 66Hz or so).
With no burst duration, you can get a pure tone when using the PWM tone library. Has slight trouble
with legato playing- not sure why. Commands out of order? too close?        */
    tone(SPEAKERPIN, freq);  //, BURST_DURATION);
    delay(LOOPDELAY);

    return;
  }


  if (cycles * BURST_DURATION >= duration) {
    //note has played for at least the amount of milliseconds specified so get a new note
    //reset cycle counter
    cycles = 0;

    //compute new freq and duration values from IFS function
    compute_music(freq, duration);

#if defined(DEBUG)
    char buffer[40];
    sprintf(buffer, "Pitch %d and duration %d", freq, duration);
    Serial.println(buffer);
#endif
  } else {
    //use previous freq and duration- values is stored since variables are static, but the duration has to "count down"
    cycles += 1;
  }

  if (SYNC_FLAG == ON) {
    //use duration calculated from trigger pulses
    duration = arpeggiatorDuration;
#if defined(DEBUG)
    char buffer[40];
    sprintf(buffer, "sync is on and duration= %d", duration);
    Serial.println(buffer);
#endif
  }

  //hardware knob is wired backwards so changing in software so that turning knob CW increases rate as expected
  int vibratorate = map(analogRead(VIBRATORATEPIN), 0, 1023, 120, 5);  //effectively control speed of vibrato
  //depth affects pitch change and how quickly you get to new pitch when on sine version
  int vibratodepth = map(analogRead(VIBRATODEPTHPIN), 0, 1023, 5, 150);
  int inc_deviation = vibratodepth / 5;

  //programmable vibrato-like control.
  count++;
  if (count % vibratorate == 0) {
    //reset count (if you don't it would overflow after many hours)
    count = 0;
    if (VIBRATO_FLAG == SINE_VIBRATO) {
      //This creates a sinusoidal vibrato. apply modifier based on depth amount
      sineFreq += sine_vibrato_sign * (inc_amount + inc_deviation);
      if (sineFreq >= vibratodepth || sineFreq <= -vibratodepth) {
        //flip the sign when out of bounds in either direction
        sine_vibrato_sign = -sine_vibrato_sign;
      }
      deltaFreq = sineFreq;
    } else {
      //flip the square wave vibrato sign
      square_vibrato_sign = -square_vibrato_sign;
      //this creates a square vibrato. deltaFreq oscillates between a positive and a negative value
      deltaFreq = square_vibrato_sign * (vibratodepth);
    }
  }

  tone(SPEAKERPIN, freq + deltaFreq, BURST_DURATION);
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

  //read sensor
  int iterSensor = analogRead(ITERATIONPIN);
  //hardware knob is backwards so changed here to make CW rotation increase iterations
  VAR_ITERATIONS = map(iterSensor, 0, 1023, 3, MAX_ITERATIONS);
  totalIterations += 1;
  byte k = get_chance();
  next_x = a[k] * x + b[k] * y + e[k];
  next_y = c[k] * x + d[k] * y + f[k];
  x = next_x;
  y = next_y;

#if defined(DEBUG)
  Serial.print("raw x = ");
  Serial.println(x);
#endif

  //the next note to play is next_x with a duration of next_y

  //scale values so in bounds and make sense for pitch frequency and duration in milliseconds
  int scale_x = int(abs(x) * 100);
  if (scale_x > 100) {
    scale_x = 100;
  }
  //constrain the piano key range to one the arduino can play and also not too high since unpleasant
  int piano_key = map(scale_x, 0, 100, 25, 74);
  /*y has a range up to 10 or so. The map gives a set of discrete values in 125msec intervals.
  300msec = 200bpm, 2000msec = 30bpm
  Apply sensor to scale duration
  */
  //knob wired backwards. Changed here so CW rotation increases speed
  int durationScale = map(analogRead(DURATIONPIN), 0, 1023, 6, 1);
#if defined(DEBUG)
  Serial.print("durationscale = ");
  Serial.println(durationScale);
#endif
  //10 * 125 = 1250. why 125? 1/8th of 1000. Implicit 60BPM timebase.
  int scale_y = map(abs(y), 0, 10, 125, 1375) * durationScale;  //int(abs(y) * 600 + 400);

  //assign values to the variable references so changes are seen in the loop function
  freq = get_freq(piano_key);
  duration = scale_y;

#if defined(DEBUG)
  char buffer[20];
  sprintf(buffer, "Variable iterations limit = %d ", VAR_ITERATIONS);
  Serial.println(buffer);
#endif
  if (totalIterations >= VAR_ITERATIONS) {
    //reset to new starting point for iteration
    init_xy(x, y);
    totalIterations = 0;
  }
}

/*
Choose array indices based on a hard-coded probability distribution.
*/
byte get_chance() {
  static byte index = 0;
  if (RANDOM_FLAG == ON) {
    return getIFSProbabilty();
  } else {
    //random off- just return next value from kIndex (up to VAR_ITERATIONS) or start over at 0
    if (index == VAR_ITERATIONS) {
      index = 0;
    }
    //index will either be 0 or the next value. This logic keeps it in range.
    return kIndex[index++];
  }
}

/*
Just initialize to 0,0 so you have a consistent start point and can almost get an arpeggio to play the same notes
each time. I say almost because a random probability (through get_chance) is still used.
*/
void init_xy(float &x, float &y) {
  if (RANDOM_FLAG == ON) {
    x = (float)random(1, 100) / 100;
    y = (float)random(1, 100) / 100;
  } else {
    //random off- same start point for IFS
    x = 0;
    y = 0;
  }
}

/*
Convert the piano key position (1 to 88) to the corresponding frequency
*/
int get_freq(int key) {
  //adjust for difference between piano and midi numbering
  key = key + 4;
  int octave = (int)(key / 12);
  int note = (key % 12) - 1;
  float freq = LOWA * pow(2, octave) * pow(TWELFTHROOT, note);
  //round to nearest whole value
  return int(freq + 0.5);
}

/*
Get toggle button state and update the flag variable. This uses polling and is called in main loop.
*/
void updateSynthState() {
  //each flag is 1 or 0
  VIBRATO_FLAG = digitalRead(VIBRATOMODEPIN);
  byte flag = RANDOM_FLAG;
  //hardware switch wired backwards so NOT the value read
  RANDOM_FLAG = !digitalRead(RANDOMPIN);
  //sync. hardware switch wired backwards so changed value using NOT
  SYNC_FLAG = !digitalRead(SYNCONOFFPIN);
  MIDI_FLAG = !digitalRead(MIDIONOFFPIN);

  //if state goes from off to on, reset the kIndex array with new random values
  if (flag == OFF && RANDOM_FLAG == ON) {
//state transitioned from off to on so refill with new values
#if defined(DEBUG)
    Serial.println("******random on");
#endif
    fill_kIndex();
  }
}

void fill_kIndex() {
  //fill with random values. This only affects pitch, by design.
  for (byte i = 0; i < MAX_ITERATIONS; i++) {
    kIndex[i] = getIFSProbabilty();
  }
}

/*
This gets a k value (array index) based on probability rules that are part of the IFS.
*/
float getIFSProbabilty() {
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


void executeGlitch() {
  GLITCH_FLAG = ON;
  Serial.println("glitch");
  GLITCH_FLAG = OFF;
}

/*
Receives a trigger pulse for playback syncronization. Pulse needs to be a 5v trigger
like from a drum machine or other synth. Needs two pulses for determining note 
duration for use in arpeggiator.
*/
void receiveTrigger() {
  //need to calculate time between pulses (i.e, bpm) and turn into value for duration
  static long oldPulse;
  long newPulse;
  //I see pulse accuracy of +-1 millisecond which is fine
  newPulse = millis();
  //80% of pulse interval
  arpeggiatorDuration = int((newPulse - oldPulse) * 0.8);
  oldPulse = newPulse;
#if defined(DEBUG)
  Serial.println("pulse");
#endif
}

/*
Acts when the glitch momentary button is pressed. Two ways to control debounce- timing, which works pretty well,
or a flag for glitch on that will work even better. Turn on in the glitch method (not the interrupt)
and turn off when done. IF on, the interrupt should do nothing.
*/
void glitchTrigger() {
  //store in a static variable for later use
  static long lastGlitch = 0;
  static byte glitchCount = 0;
  if (GLITCH_FLAG == ON || glitchCount >= 1) {
    return;
  } else if ((millis() - lastGlitch) > DEBOUNCE_DELAY) {
    glitchCount += 1;
    lastGlitch = millis();
    executeGlitch();
    glitchCount = 0;
  }
}

/*
Do something in conjunction with setting of the ten position switch. The button activates the function. Maybe.
*/
void tenPositionTrigger() {
  //FIXME will need debouncing
#if defined(DEBUG)
  Serial.println("ten position");
#endif
}

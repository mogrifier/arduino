/*  
 * File:          incbin-print-self.ino
 * Description:   incbin Aduino example. Prints out the content of this file to Serial.
 * URL:           https://github.com/AlexIII/incbin-arduino
 * 
 */
//                                    !!! IMPORTANT !!!
//          !!! This example WON'T COMPILE unless you follow these instructions !!!
/* -----------------------------------------------------------------------------------------------
 *      1. SAVE this example (CTRL+S) to any folder on your disk
 *      2. CLOSE ALL Arduino windows
 *      3. Re-open the *.ino file you've just saved via DOUBLE-CLICK (DO NOT do File -> Open...)
 * -----------------------------------------------------------------------------------------------
 * 
 * 
 *    - You can use this library in your *.ino project as usual,
 *      it will compile fine, but you will have to open it via double-click every time.
 *    - See the github page for detailed explanation of these shenanigans.
 *    - If you have troubles using this library feel free to open an issue on the github.
 *    
 */

#include "incbin.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Alternatively you can include the file as BINARY data
// --
INCBIN(AUDIO, "8bit-AMEN_LOO.WAV");
// --
// This will create global variables:
//  const unsigned char gSketchTextData[];              // Pointer to the data
//  const unsigned char *const gSketchTextEnd;          // Pointer to the end of the data
//  const unsigned int gAUDIOSize;                 // Size of the data in bytes


//AKWF sampling information
const float sampleRate = 8000.0;
const float period = (float)1 / sampleRate;
//AKWF all use 600 samples. I am using bytes so 600 8-bit samples
const int OFFSET = 25000;
const int SPEAKERPIN = 9;

/*
this can play arbitrary regions of memory by exceeding the size of the array with index used.
Implication is a single file (like the mirage disks with 6 sets of samples) could be loaded in 
and if the correct indices are know, jump into any chunk of memory and play it.
However, so far all I get when playing is a siren sound. Not good enough. Can get static, too.

Maybe I need the second order modulator? Or maybe I need to oversample. I think I may be hitting
computational limits. Nothing has sound close to source material.
Still, stepping through memory at higher rates may produce proper pitches. 

Hardware time looks like a must. Software loop is still slow I think. At least I know digitalWrite is slow.
*/

/*
Interrupt Service Routine will be called each time the timer reaches its compare value.
*/
unsigned int reload = 1999;
byte bit;

/*
This is working BUT power level (volume) is super low since pulse durations are too fast.
For simple stuff, go with 8KHz sampling rate. The tone I hear sounds really nice. Increase
duty cycle and try again.
*/
ISR(TIMER1_COMPA_vect) {
  bit = getWavetablePDM();  //getSinePDM(880);
  if (bit == 1) {
    PORTB = PORTB | B00000010;  //pin 9 HIGH
    //Serial.println("hi");
  } else {
    PORTB = PORTB & B11111101;  //pin 9 LOW
    //Serial.println("low");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SPEAKERPIN, OUTPUT);
  Serial.print("cpu clock = ");

  Serial.println(F_CPU);

  Serial.println(F("----------------------------------"));
  // Print data size
  Serial.print(F("wavetable file size: "));
  Serial.println(gAUDIOSize);

  /* Alternatively, read the data from ROM byte-by-byte */
  /*
  PGM_P p = reinterpret_cast<PGM_P>(gAUDIOData);
  for(int i = 0; i < gAUDIOSize; ++i) {
    unsigned char c = pgm_read_byte(p++);
   Serial.write(c);
  }
  Serial.println();
*/

  initTimer();
}

void initTimer() {
  DDRB = B00000010;
  // initialize timer

  cli();
  //TCCR1A = 0;
  //TCCR1B = 0;
  //OCR1A = reload;
  //TCCR1B = (1 << WGM12) | (1); // << CS12);

  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

  OCR1A = reload;
  TIMSK1 |= _BV(OCIE1A);  //(1 << OCIE1A);
  sei();
  Serial.println("TIMER1 Setup Finished.");
}

/*
point of this is to play the adventure kid loop at a reasonable rate. The original sample rate is 44100.
That means if you play it there, the pitch will be accurate. Let's see what a software counter can do.
*/
void loop() {

  //toggle built-in LED
  /*
PORTB = PORTB | B00100000;
  delay(500);
  PORTB = PORTB & B11011111;
  delay(500);
  Serial.println("toggle");
  */
}


/*
# sampling information
Fs = 44100 # sample rate
T = 1/Fs # sampling period
t = 0.1 # seconds of sampling
N = Fs*t # total points in signal

# signal information
freq = 200 # in hertz, the desired natural frequency
omega = 2*np.pi*freq # angular frequency for sine waves
#creates a list of monotonically increasing integers up to N, then * T for each.
t_vec = np.arange(N)*T
# Produces 4,410 samples (0.1 seconds worth) in ndarray. Samples are 64-bit floats
y = np.sin(omega*t_vec)

*/
int getSinePDM(float freq) {
  static float count = 0.0;
  static float error = 0.0;  //making it static keeps a running total as needed
  float omega = TWO_PI * freq;
  int intermediate = -1;
  int output;

  //count up to two pi radians which is one full cycle
  float angle = omega * period * count;
  //Serial.println(omega);
  float sine = sin(angle);
  //Serial.println(sine);
  error += sine;
  if (error > 0) {
    intermediate = 1;
  } else {
    intermediate = -1;
  }
  error = error - intermediate;
  count += 1.0;
  if (angle >= TWO_PI) {
    count = 0.0;
  }
  output = intermediate;
  //only return 0 or 1
  if (output == -1) {
    output = 0;
  }
  return output;
}



/*
streaming PDM example. First order. sigma-delta modulator. Sample in range -1 to 1.

function pdm(real[0..s] x, real qe = 0) // initial running error is zero
    var int[0..s] y
  
    for n from 0 to s do
        qe := qe + x[n]
        if qe > 0 then
            y[n] := 1
        else
            y[n] := −1
        qe := qe - y[n]
  
    return y, qe // return output and running error
*/

int getWavetablePDM() {
  static int count = OFFSET;  //program offset to memory where audio file is stored- maybe
  static float error;         //making it static keeps a running total as needed
  int intermediate;
  int output;

  //need to normalize the audio sample range from unsigned bytes to [-1,1]
  //Serial.println(((float)gAUDIOData[count] / 255.0) * 2.0 - 1.0);

  byte sample = gAUDIOData[count];
  error += ((float)sample / 255.0) * 2.0 - 1.0;
  if (error > 0) {
    intermediate = 1;
  } else {
    intermediate = -1;
  }
  error = error - intermediate;
  //Serial.println(error);
  count = count + 1;

  if (count >= gAUDIOSize + OFFSET) {
    count = OFFSET;
  }
  output = intermediate;
  //only return 0 or 1
  if (output == -1) {
    output = 0;
  }

  //Serial.println(output);

  return output;
}

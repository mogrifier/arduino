/*
play tones. Arduino MEGA 1280.
*/
#include <Arduino.h>

#define TONE_USE_INT
#define TONE_PITCH 440
#include <TonePitch.h>

const int SPEAKERPIN = 9;
const int PITCHPIN = 0;


int index = 0;
int notes[20] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5,
 NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6};
int maskbits[32];

void setup() {
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensor = analogRead(PITCHPIN);
  int duration = map(sensor, 0, 1023, 10, 400);   
  int result = xorop(notes[index]);

  if (index == sizeof(notes)) {
    index = 0;
  }

  
  int play = xorop(random()) & 1111;
  Serial.println(play);


//clearly has no notion of array size so can play random memory locations outside the array!!
  tone(SPEAKERPIN, notes[random(0, 20)], duration);
  delay(duration);

/*
If I want a truly generative music experience, I need memory of what has gone before and to use that going forward.


*/


}

int xorop(int operand) {
  //Serial.println("");
  //Serial.println(operand);
  computeRandom();
  int result = 0;
  for (int i = 0; i < 31; i++) {
    int bit = bitRead(operand, i);
    int output = bit | maskbits[i];
    //add bits as you go to the result

    //apply XOR
    if (output == 1 && bit == 1 && maskbits[i] == 1) {
      output = 0;
    }

    //store
    if (output == 1) {
      result += 2 ^ i;
    }    


  }


return result;
}



void computeRandom() {
  //initialize array of 1 's and 0's representing the mask
  int r = random();
  for (int i = 0; i < 31; i++) {
    maskbits[i] = bitRead(r, i);
  }  

}


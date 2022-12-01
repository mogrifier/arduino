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
int notes[10] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5};
int maskbits[32];

void setup() {
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensorReading = analogRead(PITCHPIN);

  int result = xorop(notes[index]);
  //replace original notes as you go through
  notes[index++] = result;  
  Serial.println(result);

  if (index == 10) {
    index = 0;
  }


  int duration = 250;
  tone(SPEAKERPIN,NOTE_C4, duration);
  delay(700);

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

    //Serial.print(bit);
    //Serial.print(" xor ");
    //Serial.print(maskbits[i]);
    //Serial.print(" = ");
    //Serial.println(output);

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


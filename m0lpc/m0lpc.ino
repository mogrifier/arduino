#include "incbin.h"

INCBIN(Birth, "birth.bin");
INCBIN(Birthday, "birthday.bin");
INCBIN(Day, "day.bin");
INCBIN(Happy, "happy.bin");
INCBIN(Happyhappybirthday, "happyhappybirthday.bin");

const int sendPin = 3;
const int readPin = 2;

int count = 0;
int mod = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(sendPin, OUTPUT);

  pinMode(readPin, INPUT_PULLUP);
  pinMode(readPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(readPin), highInterrupt, CHANGE);

  //prepare to write data to serial port for troubleshooting
  Serial.begin(57600);

  // DDRD = DDRD | B00000100;  // this is safer as it sets pins 2 as output, 7 as input
  // without changing the value of pins 0 & 1, which are RX & TX

  //examine a binary file included with incbin
  Serial.print(F("happyhappybirthday.bin size: "));
  Serial.println(gHappyhappybirthdaySize);

  Serial.print(F("happy.bin size: "));
  Serial.println(gHappySize);

  //the Data is an Array and this prints/proves it
  //for (int i = 0; i < gDaySize; i++) {
  //  Serial.print(gDayData[i]);
  //}
}

void loop() {}

void stopCode() {
  Serial.println("stop");

  for (int j = 0; j < 300; j++) {
    bitSet(PORTD, sendPin);
  }
}


void send(byte data[], int length) {

  int bitCount = 0;
  mod++;
  if (mod > 1000000) {
    Serial.println("pulse");
    mod = 0;
  }
  PORTD = B000000;

  //need to convert each byte to 8 bits and send as a bitSet or bitClear
  for (int j = 0; j < length; j++) {
    for (int b = 0; b < 8; b++) {
      if (bitRead(data[j], b) == 1) {
        bitSet(PORTD, sendPin);
      } else {
        bitClear(PORTD, sendPin);
      }
      bitCount++;
    }
  }
  Serial.println("bits sent");
  Serial.println(bitCount);
}


void sendBit(byte data[], int length) {
  PORTD = B000000;
  int theByte = round(count / 8);
  int theBit = count % 8;

  /*
Serial.println("byte");
Serial.println(theByte);
Serial.println("bit");
Serial.println(theBit);
*/

//send 8 bits- test
for (int i =0; i < 8; i++) {
  if (bitRead(data[theByte], i) == 1) {
    bitSet(PORTD, sendPin);
  } else {
    bitClear(PORTD, sendPin);
  }
}


}


void highInterrupt() {

  //Serial.println(micros());
  if (count >= gHappyhappybirthdaySize * 8) {
    count = 0;
    stopCode();
  } else {
    //send a bit at a time
    sendBit(gHappyhappybirthdayData, gHappyhappybirthdaySize);
    count+=8;
  }

  /*
  //choose word(s) to send randomly
  long word = random(5);
  switch (word) {
    case 0:
      send(gDayData, gDaySize);
      break;
    case 1:
      send(gBirthData, gBirthSize);
      break;
    case 2:
      send(gBirthdayData, gBirthdaySize);
      break;
    case 3:
      send(gHappyData, gHappySize);
      break;
    case 4:
      send(gHappyhappybirthdayData, gHappyhappybirthdaySize);
      break;

      
  }
  */


  //stopCode();
}
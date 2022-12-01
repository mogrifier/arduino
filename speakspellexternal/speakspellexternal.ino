/*
 * Interrupts sketch
 * see #responding_to_an_infrared_remote_cont for connection diagram
 */

const int pin = 2; // pin the receiver is connected to
const int numberOfEntries = 32; // set this number to any convenient value        

volatile unsigned long microseconds;
volatile byte idx = 0;
volatile unsigned long results[numberOfEntries];

void setup() {
  // put your setup code here, to run once:
  /*
  All connected digital pins 5-11 are digital IN except 8 is two-way so will need to be toggled for write 
  (output) operations.

  5- M1
  6- M2
  7- ROMCLK
  8- ADDR8
  9- ADDR4
  10- ADDR2
  11- ADDR1

  */

  //for debug
  Serial.begin(57600);

  pinMode(pin, INPUT_PULLUP);

  // Use the pin's interrupt to monitor for changes
  attachInterrupt(digitalPinToInterrupt(pin), analyze, CHANGE);
  results[0]=0;
}

void loop()
{
  if(idx >= numberOfEntries)
  {
    Serial.println("Durations in Microseconds are:") ;
    for( byte i=0; i < numberOfEntries; i++)
    {
      Serial.print(i); Serial.print(": ");
      Serial.println(results[i]);
    }
    idx = 0; // start analyzing again
  }
  delay(1000);
}

void analyze()
{
  if(idx < numberOfEntries)
  {
    if(idx > 0)
    {
      results[idx] = micros() - microseconds;
    }
    idx = idx + 1;
  }
  microseconds = micros();
}
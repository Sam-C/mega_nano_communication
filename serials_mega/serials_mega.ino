/**
 * MEGA
 * 
 * send via TX1
 * receive via digital pins
 * 
 * Not all pins on the Mega and Mega 2560 support change interrupts, 
 * so only the following can be used for RX: 
 * 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, 
 * A8 (62), A9 (63), A10 (64), A11 (65), 
 * A12 (66), A13 (67), A14 (68), A15 (69).
 */

#include <SoftwareSerial.h>

// Defining constants
#define FEEDBACK_FREQUENCY 20// In Hz
#define FEEDBACK_FREQUENCY_COUNT 1000/FEEDBACK_FREQUENCY
#define SAMPLETIME (5000/FEEDBACK_FREQUENCY)
#define TIME_STEP 1.0/FEEDBACK_FREQUENCY
#define NUMBER_CONNECTED_NANOS 2

bool ledState = LOW;

//we can turn it to an array later
SoftwareSerial serialNano0(10, 22); // RX, TX
SoftwareSerial serialNano1(12, 23); // RX, TX

void setup() {
  Serial1.begin(115200);  //USB
  Serial.begin(115200); //broadcast
  serialNano0.begin(115200);
  serialNano1.begin(115200);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  Serial.println("Mega is online.");Serial.flush();
  //OCR0A = 0xAF;
  //TIMSK0 |= _BV(OCIE0A);
  setup_timer1();
}

void loop() {
  //request feedback from nano0
  

  

  //delay(1000);

  //read feedback
  

  //delay(2000);
}

ISR(TIMER1_COMPA_vect)
{
  //unsigned long int t_0 = micros();
  for (int i = 0; i < NUMBER_CONNECTED_NANOS; i++){
    if(i==0){
      serialNano0.listen();
      String request = "f"+String(i);
      Serial1.println(request);
      //Serial.println("sent: " + request);Serial.flush();
      if (serialNano0.available() > 0){
        String feedback = serialNano0.readStringUntil('\n');
        ledState = !ledState;
        digitalWrite(13,ledState);
        //Serial.println("received from nano0: " + feedback);Serial.flush();
      } else {
        //Serial.println("no response");Serial.flush();
      }
      //delay(10);
    } else if(i==1){
      //serialNano1.listen();
      //String request = "f"+String(i);
      //Serial1.println(request);
      Serial1.println("Hello");
      //Serial.println("sent: " + request);Serial.flush();
      /*if (serialNano1.available() > 0){
        String feedback = serialNano1.readStringUntil('\n');
        Serial.println("received from Serial1: " + feedback);Serial.flush();
      } else {
        //Serial.println("no response");Serial.flush();
      }*/
      
    }
  }
  //Serial.println(micros() - t_0);
  //Serial.flush();
}

void setup_timer1() {  // initialize Timer1
  cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
  // set compare match register to desired timer count:
  OCR1A = (15624 / FEEDBACK_FREQUENCY);
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  sei();          // enable global interrupts
}




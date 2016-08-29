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

//we can turn it to an array later
SoftwareSerial serialNano0(10, 12); // RX, TX

void setup() {
  Serial1.begin(9600);  //USB
  Serial.begin(9600); //broadcast
  //serialNano0.begin(9600);
  //Serial.println("Mega is online.");Serial.flush();
}

void loop() {
  //request feedback from nano0
  String request = "f0";
  Serial1.println(request);
  Serial.println("sent: " + request);Serial.flush();

  //serialNano0.listen();

  delay(1000);

  //read feedback
  if (Serial1.available() > 0){
    String feedback = Serial1.readStringUntil('\n');
    Serial.println("received from Serial1: " + feedback);Serial.flush();
  } else {
    //Serial.println("no response");Serial.flush();
  }

  delay(2000);
}

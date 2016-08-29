/**
 * MEGA
 * 
 * send via TX1
 * receive via digital pins
 */

#include <SoftwareSerial.h>

SoftwareSerial serialNano0(22, 23); // RX, TX
SoftwareSerial serialNano1(24, 25); // RX, TX

void setup() {
  Serial.begin(115200);  //USB
  Serial1.begin(115200); //broadcast
  serialNano0.begin(115200);
  serialNano1.begin(115200);
  Serial.println("Mega is online.");

}

void loop() {
  //request feedback from nano0
  String request = "f0";
  Serial1.println(request);
  Serial.println("sent: " + request);

  serialNano0.listen();
  //wait
  delay(1000);

  //read feedback
  if (serialNano0.available() > 0){
    String feedback = serialNano0.readStringUntil('\n');
    Serial.println("received from serialNano0: " + feedback);
  }

  //request feedback from nano1
  request = "f1";
  Serial1.println(request);
  Serial.println("sent: " + request);

  serialNano1.listen();
  //wait
  delay(50);

  //read feedback
  if (serialNano1.available() > 0){
    String feedback = serialNano1.readStringUntil('\n');
    Serial.println("received from serialNano1: " + feedback);
  }

  delay(3000);

}

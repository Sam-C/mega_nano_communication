/*
 * nano
 * 
 * receive via TX0
 * send via RX0
 * 
 * software serial baud rate: max. 57600 for 8MHz CPU, 
 * 115200 for 16MHz CPU (mega, nano)
 */

#define THIS_NANO_ID 0
int led = 13;

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  digitalWrite(led,LOW);
}

void loop() {
  //wait and read request
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');
    Serial.println(requestReceived);  //echo the string received
    Serial.flush();
    digitalWrite(led,HIGH);
//    if (requestReceived == ("f" + THIS_NANO_ID)){  //if it's a feedback request for this nano
//      Serial.println(THIS_NANO_ID + "777");                     //send feedback
//    }

  } else {
    //Serial.println("I can't here you!");
    //Serial.flush();
  }
  delay(3000);
}

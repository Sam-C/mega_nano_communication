/*
 * nano
 * 
 * receive via TX0
 * send via RX0
 * 
 * software serial baud rate: max. 57600 for 8MHz CPU, 
 * 115200 for 16MHz CPU (mega, nano)
 */

//#define THIS_NANO_ID 0
#define THIS_NANO_ID 1
int led = 13;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led,LOW);
}

void loop() {
  //wait and read request
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');    
    // FOR THE MOMENT THIS ONLY SUPPORTS 10 OPTIONS
    int receivedID = int(requestReceived.charAt(1)) - '0';
    if(receivedID == THIS_NANO_ID){
      digitalWrite(led,HIGH);
      //Serial.println("Im here");
      Serial.println("me too");
      Serial.flush();
    }
  } else {
    //Serial.println("I can't here you!");
    //Serial.flush();
  }
  //delay(3000);
}

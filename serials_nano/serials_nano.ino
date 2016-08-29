/*
 * nano
 * 
 * receive via TX0
 * send via RX0
 */

#define THIS_NANO_ID 0

void setup() {
  Serial.begin(115200);
}

void loop() {
  //wait and read request
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');
    if (requestReceived == ("f" + THIS_NANO_ID)){  //if it's a feedback request for this nano
      Serial.println("Im here."); 
      //Serial.println(THIS_NANO_ID + "777");                     //send feedback
    }
  }

}

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
  Serial.println("Hi, I'm nano 0.");
  delay(3000);
}

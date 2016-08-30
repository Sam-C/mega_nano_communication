/*
 * nano
 * 
 * receive via TX0
 * send via RX0
 * 
 * software serial baud rate: max. 57600 for 8MHz CPU, 
 * 115200 for 16MHz CPU (mega, nano)
 */

#define NANO_ID 5
#define LENGTH_HEX_NUM_DIGITS 4
#define RECEIVE_PREFIX_LENGTH_CMD 'l'
#define RECEIVE_FEEDBACK_REQUEST 'f'

int led = 13;

/////////////////////////// FEEDBACK VARIABLES ///////////////////////////

/// servo position as 'pwm' value (see _feedback for scale above) ///
int servoPos;
int servoLastPos;
int servoPosChange;

/// from servoPos derived angle and length of cable (radians) ///
double servoDeg;
double cableLength = 184;
double cableLengthChange=0;

/// string for Serial.read() /// 
String str = "";


/////////////////////////// OUTPUT VARIABLES ///////////////////////////

/// aim for length of cable ///
double destinationLength = 0;
double destinationLengthChange = 0;
double oldDestinationLength;

/// derived aim of angle ///
double destinationDeg = 0;
double destinationDegChange = 0;

/// translated length of cable to 'pwm' value ///
int destinationPWM;

/// control communication with serial and servo ///
boolean systemOn = 0;
boolean enableServo = 0;
boolean stillmode = 0;

/////////////////////////// FUNCTION PRECALLING ///////////////////////////

//void setup_timer1();
void sendFeedback();
void readSerial();
// void setInitialLength(String str);
void setDestinationLength(String str);
//void readPositionFeedback();
//void calculateCableLengthChange();
//void crossing();
//void ctrl_motor();
//void mapping();
//void limitDegree();
//void deadzone();
//void servoPulse(int servoPin, int pulseWidth);
//void control();
//void updateDestinationDeg();

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led,LOW);
}

void loop() {
  readSerial();
/*
  //wait and read request
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');    
    // FOR THE MOMENT THIS ONLY SUPPORTS 10 OPTIONS
    int receivedID = int(requestReceived.charAt(1)) - '0';
    if(receivedID == THIS_NANO_ID){
      digitalWrite(led,HIGH);
      Serial.println(String(THIS_NANO_ID) + " is here");
      Serial.flush();
    }
  } else {
    //Serial.println("I can't here you!");
    //Serial.flush();
  }
  //delay(3000);*/
}

void readSerial() //receive characterizing prefix (+ length in 4 digit Hex, when appropriate)
{
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');    
    char command = requestReceived[0];
    if(command == RECEIVE_PREFIX_LENGTH_CMD){
      Serial.println("HERE");
       setDestinationLength(requestReceived);
       Serial.println(destinationLength);
       cableLength = destinationLength;
       Serial.println(cableLength);
    } else if(command == RECEIVE_FEEDBACK_REQUEST){
       // FOR THE MOMENT THIS ONLY SUPPORTS 10 OPTIONS
      int receivedID = int(requestReceived.charAt(1)) - '0';
      if(receivedID == NANO_ID){
        sendFeedback();
      }     
    } // ADD CALIBRATION LATER
  }
}

void setDestinationLength(String str)
{
  char tmp[LENGTH_HEX_NUM_DIGITS+1];
  for (int j = LENGTH_HEX_NUM_DIGITS*NANO_ID + 1; j < LENGTH_HEX_NUM_DIGITS*NANO_ID + 5; j++)
  {
    tmp[j-(LENGTH_HEX_NUM_DIGITS*NANO_ID + 1)] = str[j]; //omits first letter, since this is message prefix then extracts the relavent component of the message
  }
  tmp[4] = '\0';
  Serial.println(tmp);
  destinationLength = ((double)strtol(tmp, 0, 16)); //input is received in HEX, scale is millimeters - strtol converts string to double
}

void sendFeedback()
{
  // Send the first character of F to indicate feedback
  Serial.print(NANO_ID);
  // Round to nearest integer value
  int l_rounded = cableLength + 0.5;
  char t[LENGTH_HEX_NUM_DIGITS];
  char s[LENGTH_HEX_NUM_DIGITS+1];
  // Pad the 0s
  itoa(l_rounded,t,16);
  // Find the first 0 value
  int j = strlen(t);
  // Now rearrange by shifting the values across
  for(int k = 0; k < LENGTH_HEX_NUM_DIGITS; k++){
    if(k < j){
      s[LENGTH_HEX_NUM_DIGITS - (j-k)] = t[k];
    } else {
      s[k-j] = '0';
    }      
  }
  s[LENGTH_HEX_NUM_DIGITS] = '\0';
  Serial.println(s);
  Serial.flush();
}


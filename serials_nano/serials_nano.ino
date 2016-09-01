/*
 * nano
 * 
 * receive via TX0
 * send via RX0
 * 
 * software serial baud rate: max. 57600 for 8MHz CPU, 
 * 115200 for 16MHz CPU (mega, nano)
 */

#include <string.h>
#include <Wire.h>
#include <math.h>

int b = 482;
#define ADD_CONSTANT 1;
int add_term = ADD_CONSTANT;
#define NANO_ID 0
//int led = 13;
/// defining constants that used in this program
#define FEEDBACK_FREQUENCY 40// In Hz
#define FEEDBACK_FREQUENCY_COUNT 1000.0/FEEDBACK_FREQUENCY
#define SAMPLETIME (5000.0/FEEDBACK_FREQUENCY)
#define TIME_STEP 1.0/FEEDBACK_FREQUENCY
#define LENGTH_HEX_NUM_DIGITS 4
#define LENGTH_MULT 10
#define RADIUS 20 //milli meters
#define Kp 1.0 // Gain for proportional controller
#define Ki 0.5 // Gain for intergrater
#define RECEIVE_PREFIX_LENGTH_CMD 'l'
#define RECEIVE_FEEDBACK_REQUEST 'f'

/////////////////////////// DEBUGGING AND TIMING VARIABLES //////////////
int led = 13;
unsigned long int t_ref;
unsigned long int loopTime;


/////////////////////////// SERVO PARAMETERS ///////////////////////////

/// number of Arduino pin at nano, to which the motor is connected ///
int MOTOR_PIN = 5;

/// PWM scale for position feedback from servo ///
int maximum_pwm_feedback = 1502; //1970;
int minimum_pwm_feedback = 480; //635;
int difference_pwm_feedback = maximum_pwm_feedback - minimum_pwm_feedback;

/// PWM scale for position output to servo ///
int maximum_pwm_output = 1502;
int minimum_pwm_output = 480;
int difference_pwm_output = maximum_pwm_output - minimum_pwm_output;

/// scale for velocity output to servo ///
int clockwise_max = 2400;
int clockwise_min = 2090;
int anticlockwise_max = 1650;
int anticlockwise_min = 1850;

double initialPos = -1;

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
boolean firstTime = 1;
boolean enableServo = 0;
boolean stillmode = 0;

/////////////////////////// FUNCTION PRECALLING ///////////////////////////

//void setup_timer1();
void sendFeedback();
void readSerial();
// void setInitialLength(String str);
void setDestinationLength(String str);
void readPositionFeedback();
void calculateCableLengthChange();
void crossing();
void ctrl_motor();
void mapping();
void limitDegree();
void deadzone();
void servoPulse(int servoPin, int pulseWidth);
void control();
void updateDestinationDeg();


/////////////////////////// CONTROL VARIABLES ///////////////////////////

/// controlling of the method crossing() ///
boolean left = 0;
boolean right = 0;
boolean cross = 0;
int crossPulse = 0;

/// control for errors ///
double ref, ITerm, lastErr, dInput;
unsigned long lastTime;

void setup() {
  Serial.begin(115200);
  //setup_timer1();
  //pinMode(led, OUTPUT);
  //digitalWrite(led,LOW);
}

void loop() {
  readSerial();
  if((millis() - t_ref) > TIME_STEP*1000){
    t_ref = millis();
    //the ISR of Timer1 is triggered with the above defined frequency
    readPositionFeedback(); //reads position feedback from servo and calculates angle
    calculateCableLengthChange(); //calculates the change in the cable length since last movement
    updateDestinationDeg(); //updates the destination degree from the serial monitor (if no input  readSerial();
    limitDegree(); //keeps the destinationDegree within 0 - 360 degree
    crossing(); //
    if(enableServo) //in input has been received, transmission to servo is enabled
    {
      ctrl_motor(); //transmits the output signal towards the motor
    }
    // THIS NEEDS TO BE REMOVED
//    if((b+=add_term) > 1502){
//      add_term = -ADD_CONSTANT;
//    } else if((b+=add_term) < 480) {
//      add_term = ADD_CONSTANT;
//    }
    //Serial.println(b);
    //destinationLength = b/2 -150;
    //Serial.println(destinationLength);
    //ctrl_motor();
  }
}

/*ISR(TIMER1_COMPA_vect){
  //the ISR of Timer1 is triggered with the above defined frequency
  readPositionFeedback(); //reads position feedback from servo and calculates angle
  calculateCableLengthChange(); //calculates the change in the cable length since last movement
  updateDestinationDeg(); //updates the destination degree from the serial monitor (if no input  readSerial();
  limitDegree(); //keeps the destinationDegree within 0 - 360 degree
  crossing(); //
  if(enableServo) //in input has been received, transmission to servo is enabled
  {
    ctrl_motor(); //transmits the output signal towards the motor
  }
  // THIS NEEDS TO BE REMOVED
  if((b+=add_term) > 1502){
    add_term = -ADD_CONSTANT;
  } else if((b+=add_term) < 480) {
    add_term = ADD_CONSTANT;
  }
  //Serial.println(b);
  destinationLength = b/2 -150;
  //Serial.println(destinationLength);
  //ctrl_motor();
  ///
}*/

/*void setup_timer1() {  // initialize Timer1
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
}*/

void readSerial() //receive characterizing prefix (+ length in 4 digit Hex, when appropriate)
{
  if (Serial.available() > 0){
    String requestReceived = Serial.readStringUntil('\n');    
    char command = requestReceived[0];
    if(command == RECEIVE_PREFIX_LENGTH_CMD){
      if(firstTime){
        enableServo = 1;
        firstTime = 0;
      }
      setDestinationLength(requestReceived);
       //cableLength = destinationLength;
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
  //Serial.println(tmp);
  destinationLength = ((double)strtol(tmp, 0, 16)); //input is received in HEX, scale is millimeters - strtol converts string to double
}

void readPositionFeedback()
{//reads position feedback from servo and calculates angle
  int oldPos = servoPos;

  digitalWrite(MOTOR_PIN, HIGH);
  delayMicroseconds(50);
  digitalWrite(MOTOR_PIN, LOW);
  servoPos = pulseIn(MOTOR_PIN, HIGH, 2000); //triggers servo, then measures time until next HIGH signal, cuts off after 3000us or 3ms

  if ((servoPos < 300) || (servoPos > 2000)) { //results outside these boundaries are faulty
    servoPos = oldPos;
  }
  servoDeg = (float(servoPos-minimum_pwm_feedback) / float(difference_pwm_feedback)) * 360; //maps the pwm value to a degree (0-360)
}

void calculateCableLengthChange()
{//calculates the change in the cable length since last movement
  if(servoLastPos == 0){ //first time this method is entered
    servoLastPos = servoPos;
  }
  servoPosChange = (servoPos - servoLastPos); // as value in 'pwm'
  if (servoPosChange >= 200) //changes of this dimension imply outside forces rapidly turning the servos
  {
    servoPosChange = -3; //3 is an approximate change in pwm for the jump at the gap between maximum pwm and minimum pwmoutput
  }
  if (servoPosChange <= -200)
  {
    servoPosChange = 3;
  }
  cableLengthChange = (servoPosChange * 2 * M_PI * RADIUS) / difference_pwm_feedback; //'converted to 'cable length by calculating the radians 
  cableLength +=  cableLengthChange;
  servoLastPos = servoPos;
}

void updateDestinationDeg()
{//updates the destination degree from the serial monitor (if no input, no change)
  if (initialPos < 0)
  { //first time this method is entered
    initialPos = servoDeg;
  }

  if (stillmode == false) // stillmode is used to keep end-effector under tension
  {
    destinationLengthChange = oldDestinationLength - destinationLength; //calculate the change in cable length
    destinationDegChange = destinationLengthChange * 180 / (M_PI * RADIUS); //maps change from radians to degree
    destinationDeg = destinationDeg - destinationDegChange; // update the destination degree, taking into account the commanded change in cable length
  }
  else if (servoDeg >= (initialPos - 1))
  {
    destinationDeg = initialPos - 2; // if stillmode is true, the servo pulls back by two degrees, keeping the end-effector under tension
  }
  oldDestinationLength = destinationLength;

}

void limitDegree()
{//keeps the destinationDegree within 0 - 360 degree, e.g. -100 for example 375 will map to 15, -100 will map to 260 
  if (destinationDeg < 0) //negative degrees should be mapped from 360 downwards
  {
    destinationDeg = (360 -  fabs(fmod(destinationDeg, 360))); //the modulo ensures a value between -1 and -360, then subtracting the absolute value of this from 360
  }
  if (destinationDeg > 360);  //values above 360 will be mapped to 0 to 360
  {
    destinationDeg = fmod(destinationDeg, 360); //modulo takes out all integral multiples of 360 to achieve correct mapping
  }
}

void crossing()
{// ding
  if (left == true)
  {
    //Serial.print("desDeg");
    //Serial.println(destinationDeg);
    //Serial.print("servoDeg");
    //Serial.println(servoDeg);
    if (((destinationDeg >= 1) && (destinationDeg <= 15)) && (servoDeg >= 340))
    {
           //Serial.print("Case left true");
      cross = true;
      crossPulse = clockwise_min;
    }
    if ((servoDeg >= 2) && (servoDeg <= 20))
    {
      cross = false;
      left = false;
           //Serial.print("exit");
    }
  }
  if (right == true)
  {
    if (((destinationDeg <= 359) && (destinationDeg >= 345)) && (servoDeg <= 20))
    {
          // Serial.print("Case right true");
      cross = true;
      crossPulse = anticlockwise_min;
    }
    if ((servoDeg <= 359) && (servoDeg >= 340))
    {
      cross = false;
      right = false;
      //     Serial.print("exit");
    }
  }
}

void ctrl_motor()
{//transmits the output signal towards the motor
  if (cross == true)//ding
  {
    servoPulse(MOTOR_PIN, crossPulse);
  }
  else
  {
    mapping();
    servoPulse(MOTOR_PIN, destinationPWM);
  }
}

void servoPulse(int servoPin, int pulseWidth)
{
  digitalWrite(servoPin, HIGH);
  delayMicroseconds(pulseWidth);
  digitalWrite(servoPin, LOW);
  delayMicroseconds(3000 - pulseWidth);
}

void mapping()
{//inversely maps the calculated destinationDeg onto the pwm_output scale
  destinationPWM = (difference_pwm_output * destinationDeg / 360 + minimum_pwm_output); 
  deadzone();
  control();
}


void deadzone ()
{//ding
  //Serial.print("PWM");
  //Serial.println(destinationPWM);
  if ((destinationPWM >= (maximum_pwm_feedback - 10)) || ((left == true) && (destinationPWM < (minimum_pwm_feedback + 10))))
  {
    destinationPWM = maximum_pwm_output - 10;
    if (right == false)
    {
      left = true;
        //Serial.println("left");
    }
  }
  if ((left == true) && ((destinationPWM < (maximum_pwm_feedback - 10)) && (destinationPWM > 1200)))
  {
    left = false;
    //Serial.println("left false");
  }

  if ((destinationPWM <= (minimum_pwm_feedback + 10)) || ((right == true) && (destinationPWM > (maximum_pwm_feedback - 10))))
  {
    destinationPWM = minimum_pwm_output + 10;
    if (left == false)
    {
     right = true;
     //Serial.println("right");
    }
  }
  if ((right == true) && ((destinationPWM > (minimum_pwm_feedback + 10)) && (destinationPWM < 800)))
  {
    right = false;
    //Serial.println("right false");
  }
}

void control() {// Control basically enhance the signal for motor to achieve the position that commanded to, consist of Proportional and integral parts.
  double min = destinationPWM - 30;
  double max = destinationPWM + 30;
  if ((servoPos < min) || (servoPos > max))
  {
    ITerm = 0; // The integrater will trigger once the feedback is close enough(30pwm value) to the destinationPWM. 
  }
  else
  {
    unsigned long now = millis();double cableLength = 184;
    int timeChange = (now - lastTime);
    int error = destinationPWM - servoPos ;
    if (timeChange >= (SAMPLETIME - 50)) // the controller will trigger every 50millis
    {
      ITerm += (Ki * error);
      lastTime = now;
    }
    destinationPWM = destinationPWM + Kp * error + ITerm; // here is the final output on pwm signal
  }
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


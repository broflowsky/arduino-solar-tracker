#include <Servo.h>

#define FILTER_SPEED 2.0  //For smoothness, just an averaging filter
#define INV_FILTER_SPEED 1/FILTER_SPEED //that would work
#define THRESH 20 //threshold value for the delta-amplitude for up-down and left-right photoresistor
#define NUM_PHOTORESISTORS 4 //number of photoresistors
#define LOOP_DELAY 50

#define PITCH_MOTOR 4 //Digital pin out Nano33 BLE  PITCH => Vertical
#define YAW_MOTOR 3 //Digital pin out Nano33 BLE    YAW => Horizontal
#define PITCH_LIMIT_HIGH 90 // using 180 degrees servomotors
#define YAW_LIMIT_HIGH 180
#define PITCH_LIMIT_LOW 0
#define YAW_LIMIT_LOW 0



//reset function
//pure virtual function pointer that returns void and takes void as argument
//used in the bootloader
// call resetFunc(); to soft reset the arduino
void (*resetFunc) (void) = 0x0000;
void StartupSequence();

//nano 33 BLE pinout, photoresistors
//Change the analog output pins according to the board
const byte R_BL = A0;
const byte R_TL = A1;
const byte R_TR = A2;
const byte R_BR = A3;
const byte R[4] = {R_TR, R_TL, R_BR, R_BL};

////LEDs that correspond to each photoresistor/////////
//The one that gets the most light will turn on
//May leave it for final prototype
//Also change those pins according to the board
const byte L_BL = 12;
const byte L_TL = 11;
const byte L_TR = 10;
const byte L_BR = 9;
const byte L[4] = {L_TR, L_TL, L_BR, L_BL};
////////////////////////////////////////////


float readings[4];//Photoresistor readings
Servo servo_pitch;
Servo servo_yaw;
short pos_yaw;
short pos_pitch;

void setup() {
  //Debugging only, Serial will be needed to communicate with the raspberry Pi
  Serial.begin(9600);

  for (int i = 0; i < 4; ++i) {
    readings[i] = analogRead(R[i]);
    pinMode(L[i], OUTPUT);
    delay(20);
  }

  //Servos setup
  servo_pitch.attach(PITCH_MOTOR);
  servo_yaw.attach(YAW_MOTOR);


  //Initial servos position (degrees), correspond to middle
  pos_pitch = servo_pitch.read();
  pos_yaw = servo_yaw.read();
}

void loop() {

  //Find the photoresistor that receives the most light

  byte largest = 0;
  for (byte i = 0; i < 4; ++i)
  { //Low-pass filter on the readings for smoothness
    readings[i] = ( readings[i] * (FILTER_SPEED - 1.0) + ((float)analogRead(R[i]))) * INV_FILTER_SPEED;
    if (readings[i] > readings[largest])
      largest = i;
  }

  //Fire up LED corresponding to most lit photoresistor
  //turning off the others
//  for (byte i = 0; i < 4; ++i) {
//    if (i == largest)
//      digitalWrite(L[i], HIGH);
//    else digitalWrite(L[i], LOW);
//  }

  //find direction for servos
  int avgT = (readings[0] + readings[1]) * .5;
  int avgB = (readings[2] + readings[3]) * .5;
  int avgL = (readings[1] + readings[3]) * .5;
  int avgR = (readings[0] + readings[2]) * .5;

  int pitch_diff = avgT - avgB;
  int yaw_diff = avgR - avgL;

  //Serial plot for debugging
  //

    Serial.print("avgT ");
    Serial.print(avgT);
    Serial.print(", ");
    Serial.print("avgB ");
    Serial.print(avgB);
    Serial.print(", ");
    Serial.print("avgL ");
    Serial.print(avgL);
    Serial.print(", ");
    Serial.print("avgR ");
    Serial.println(avgR);

  //  Serial.print("pitch_difference: ");
  //  Serial.print(pitch_diff);
  //  Serial.print(", ");
  //  Serial.print("yaw_difference: ");
  //  Serial.println(yaw_diff);


  ////////////////Move servos if needed////////////////
  //check if more light coming off one side horizontally
  if (abs(yaw_diff) > THRESH) {
    //find which side and increment servo position
    pos_yaw = yaw_diff > 0 ? ++pos_yaw : --pos_yaw;
  }

  //more light comes from above or below
  if (abs(pitch_diff) > THRESH) {
    pos_pitch = pitch_diff > 0 ? ++pos_pitch : --pos_pitch;
  }

  //////////////////Check boundaries/////////////////
  //we dont wanna destroy our tracker :) triple check before running

  //Vertically
  if (pos_pitch > PITCH_LIMIT_HIGH)
    pos_pitch = PITCH_LIMIT_HIGH;
  else if (pos_pitch < PITCH_LIMIT_LOW)
    pos_pitch = PITCH_LIMIT_LOW;

  //Horizontally
  if (pos_yaw > YAW_LIMIT_HIGH)
    pos_yaw = YAW_LIMIT_HIGH;
  else if (pos_yaw < YAW_LIMIT_LOW)
    pos_yaw = YAW_LIMIT_LOW;



  ///////////////Check whether current position is different from newly calculated position//////

  servo_yaw.read() == pos_yaw ? void() : servo_yaw.write(pos_yaw);
  servo_pitch.read() == pos_yaw ? void() : servo_pitch.write(pos_pitch);

  delay(LOOP_DELAY);
}
void MoveMotor(Servo &servo, byte stepDelay, byte newPos) {
  byte oldPos = servo.read();
  if (oldPos < newPos)
    for (byte i = servo.read(); i < newPos; ++i) {
      servo.write(i);
      delay(stepDelay);
    }
  else for (byte i = servo.read(); i > newPos; --i) {
      servo.write(i);
      delay(stepDelay);
    }



}

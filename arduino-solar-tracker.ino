#include <Servo.h>
#define FILTER_SPEED 2.0  //For smoothness, tho may not be necessary with sunlight, need to do some testing
#define UP_DOWN_MOTOR 4 //Digital pin out
#define RIGHT_LEFT_MOTOR 3 //Digital pin out
#define THRESH 30 //threshold value for the delta-amplitude for up-down and left-right photoresistor
#define PITCH_LIMIT_HIGH 90
#define YAW_LIMIT_HIGH 180
#define PITCH_LIMIT_LOW 0
#define YAW_LIMIT_LOW 0

void StartupSequence();

//nano 33 BLE pinout, photoresistors
const byte R_BL = A0;
const byte R_TL =A1;
const byte R_TR = A2;
const byte R_BR = A3;
const byte R[4] = {R_TR,R_TL,R_BR,R_BL};

////LEDs, only needed for debbuging/////////
const byte L_BL = 12;
const byte L_TL =11;
const byte L_TR =10;
const byte L_BR =9;
const byte L[4] = {L_TR,L_TL,L_BR,L_BL};
////////////////////////////////////////////


float readings[4];
Servo servo_updown;
Servo servo_rightleft;

//Initial servos position (degrees)
short pos_horizontal;
short pos_vertical;


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  Serial.println("Starting");
  
  for(int i=0;i<4;++i){
    readings[i]=analogRead(R[i]);  
    pinMode(L[i], OUTPUT);
  }
  //Servos setup
  servo_updown.attach(UP_DOWN_MOTOR);       //up-down movement
  servo_rightleft.attach(RIGHT_LEFT_MOTOR); //left right movement
  //Initial servos position (degrees), correspond to middle
  pos_vertical = pos_horizontal = 90;
  
  //For demo purposes
  
  StartupSequence();
}

void loop() {
  byte largest = 0;
  //Low-pass filter on the readings for smoothness
  
  for(int i=0;i<4;++i)
  {
    readings[i]= ((readings[i]*(FILTER_SPEED-1.0)) + analogRead(R[i]))/FILTER_SPEED; 
    if (readings[i] > readings[largest])
        largest = i;
  }
 //Fire up LED corresponding to most lit photoresistor
  for(int i=0;i<4;++i){
    if(i == largest)
      digitalWrite(L[i],HIGH);
    else digitalWrite(L[i],LOW);
  }  
  
 //find direction for servos
    int avgT = readings[0]+readings[1]/2;
    int avgB = readings[2]+readings[3]/2;
    int avgL = readings[1]+readings[3]/2;
    int avgR = readings[0]+readings[2]/2;
  
    int vertical_diff =avgT-avgB;
    int horizontal_diff =avgR-avgL;

    //Serial plot for debugging
//
//    for(int i =0; i<4;++i){
//      Serial.print(readings[i]);
//      //Serial.print(map(readings[i],0,1023,0,3.3));
//      Serial.print(",");
//    }
//    Serial.println(" ");
//
//       Serial.print("avgT ");
//       Serial.print(avgT);
//       Serial.print(", ");
//       Serial.print("avgB ");
//       Serial.print(avgB);
//       Serial.print(", ");
//       Serial.print("avgL ");
//       Serial.print(avgL);
//       Serial.print(", ");
//       Serial.print("avgR ");
//       Serial.println(avgR);

    Serial.print("vertical difference: ");
    Serial.print(vertical_diff);
    Serial.print(",");
    Serial.print("horizontal difference: ");
    Serial.println(horizontal_diff);


//   Serial.print("horizontal ");
//   Serial.print(pos_horizontal);
//   Serial.print(" ");
//   Serial.print("vertical ");
//   Serial.println(pos_vertical);

   //Move servos if needed
  if(abs(horizontal_diff) >THRESH)
  {
    if(horizontal_diff > 0)
      pos_horizontal =  ++pos_horizontal;
    else 
      pos_horizontal = --pos_horizontal;
  }
  if(abs(vertical_diff) > THRESH)
  {
    if(vertical_diff > 0)
      pos_vertical = ++pos_vertical;
    else 
      pos_vertical = --pos_vertical;
  }

  //Check boundaries
  if(pos_vertical > PITCH_LIMIT_HIGH)
    pos_vertical=PITCH_LIMIT_HIGH;
  else if(pos_vertical<PITCH_LIMIT_LOW)
      pos_vertical=PITCH_LIMIT_LOW;
      
  if(pos_horizontal > YAW_LIMIT_HIGH)
      pos_horizontal=YAW_LIMIT_HIGH;
  else if(pos_horizontal<YAW_LIMIT_LOW)
      pos_horizontal=YAW_LIMIT_LOW;

  servo_rightleft.read()==pos_horizontal? delay(10):servo_rightleft.write(pos_horizontal);
  
   servo_updown.read()==pos_horizontal? delay(10):servo_updown.write(pos_vertical);
  
  delay(10);
}
void StartupSequence(){
  
  

  for(int i=0;i<4;++i)
  {
    digitalWrite(L[i],HIGH);
    delay(100);
  }

  servo_updown.write(PITCH_LIMIT_LOW);
  servo_rightleft.write(YAW_LIMIT_LOW);
  
  delay(1500);
  for(int i=0;i<4;++i)
  {
    digitalWrite(L[i],LOW);
    delay(100);
  }
  
  //Set them to default position
  servo_updown.write(pos_vertical);
  servo_rightleft.write(pos_horizontal);
  delay(1000);
  
  }

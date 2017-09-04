#include <AFMotor.h>
#include <Arduino.h>
#include <Wire.h>
#include <HMC5883L_Simple.h>


/*Forward is backward
 * Backward is forward
 */
AF_DCMotor Rmotor(3);  //left
AF_DCMotor Lmotor(4);  //right

int MSpeed = 200; //motor speed

int t = 150;  //delay time
int balanceTimer = 25;  //balance timer for balancing in lane

// Create a compass
HMC5883L_Simple Compass;

#define C_Trig 32 //Center
#define C_Echo 42

#define L_Trig 30 //Left
#define L_Echo 40 

#define R_Trig 34 //Right
#define R_Echo 44

int maximumRange = 10 ;// Maximum range needed
int minimumRange = 3; //Minimum range needed
int maxCenterRange = 15; // Maximum center range

static float heading;
static float targetLHeading;  //for Left turns
static float targetRHeading;  //for Right turns

static int hippoL=0; //adala nowena ekak
static int hippoR=0; //adala nowena ekak


long CDuration, CDistance;
long LDuration, LDistance;
long RDuration, RDistance;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //compass config
  Wire.begin();
  Compass.SetDeclination(23, 35, 'E');    //recheck this baibeeeee  
  Compass.SetSamplingMode(COMPASS_SINGLE);
  Compass.SetScale(COMPASS_SCALE_130);
  Compass.SetOrientation(COMPASS_HORIZONTAL_X_NORTH);
//

  pinMode(C_Trig,OUTPUT);
  pinMode(C_Echo,INPUT);

  pinMode(L_Trig,OUTPUT);
  pinMode(L_Echo,INPUT);

  pinMode(R_Trig,OUTPUT);
  pinMode(R_Echo,INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  heading=Compass.GetHeadingDegrees();    //getting the heading from compass
  
  //Center Pulse Read
  digitalWrite(C_Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(C_Trig, HIGH);
  delayMicroseconds(20);
  CDuration = pulseIn(C_Echo, HIGH);

//Left Pulse Read  
  digitalWrite(L_Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(L_Trig, HIGH);
  delayMicroseconds(20);
  LDuration = pulseIn(L_Echo, HIGH);

//Right Pulse Read  
  digitalWrite(R_Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(R_Trig, HIGH);
  delayMicroseconds(20);
  RDuration = pulseIn(R_Echo, HIGH);

  CDistance = CDuration / 58.2;
  LDistance = LDuration / 58.2;
  RDistance = RDuration / 58.2;
 
  Serial.print("Heading: ");
  Serial.print(heading);
  Serial.print("\n");
  
  Serial.print("||");
  Serial.print("Left: ");
  Serial.print(LDistance);
  Serial.print("||");
  Serial.print("Center: ");
  Serial.print(CDistance);
  Serial.print("||");
  Serial.print("Right: ");
  Serial.print(RDistance);
  Serial.print("||");
  Serial.print("\n\n");
  //delay(150);
  
  if(LDistance > maximumRange && RDistance > maximumRange && CDistance > maxCenterRange){
      Halt();
    }
  else if(LDistance <= maximumRange){
   
    if(CDistance > maxCenterRange){ //normal forward 
        Forward();
      }
     else if(RDistance >= maximumRange){  //right is free ; turn right
        if(hippoR){
          targetRHeading=getTarRHeading();  //used to get the target heading only once            
        }
        TurnRight();
      }
      else{ //turn 180 from right; center left right blocked
           if(hippoR){
            targetRHeading=getTarRHeading180();  //used to get the target heading only once            
           }
           TurnRight();  
        }
    }
    else{
      if(hippoL){
        targetLHeading=getTarLHeading();  //used to get the target heading only once
      }
      TurnLeft();
    }
}



/*
Function Declarations
*/

float getTarLHeading(){
  //get the target left turn heading
  if(heading<=90){
        targetLHeading = 360-(90-heading);
      }
  else{
      targetLHeading = heading - 90;
    }
    
    Serial.print("Target Left Heading: ");
    Serial.print(targetLHeading);
    Serial.print("\n");
  }

float getTarRHeading180(){
  //This function turns 180 degrees from Right
    if(heading>=180){
        targetRHeading = (heading - 180);
      }
  else{
      targetRHeading = heading + 180;
    }
    
    Serial.print("Target Right Heading: ");
    Serial.print(targetRHeading);
    Serial.print("\n");
  }


float getTarRHeading(){
  //get the target right turn heading
  if(heading>=270){
        targetRHeading = 360-heading;
      }
  else{
      targetRHeading = heading + 90;
    }
    
    Serial.print("Target Right Heading: ");
    Serial.print(targetRHeading);
    Serial.print("\n");
  }

void TurnLeft(){
  
  int whilecount=0; //this is used to track the while loop count
  
  while(heading>targetLHeading){
        whilecount += 1;
      
        Serial.print("Turninng LEFT ");
        //Serial.print(whilecount);
        Serial.print("\n");
        
        if(whilecount==5){
            hippoL =1;
            break;
          }
        //forward is backward... backward is forward
        Rmotor.setSpeed(MSpeed);
        Lmotor.setSpeed(MSpeed);
        Rmotor.run(BACKWARD);
        Lmotor.run(FORWARD);
        
        
        Serial.print("turning: ");
        Serial.print(heading);
        Serial.print("\n");
        heading=Compass.GetHeadingDegrees();
    }

    if(heading<targetLHeading){
        hippoL = 0;
      }
    Rmotor.run(RELEASE);
    Lmotor.run(RELEASE);
  }

void TurnRight(){
  
  int whilecount=0; //this is used to track the while loop count
  
  while(heading<targetRHeading){
        whilecount += 1;
      
        Serial.print("Turning Right ");
        //Serial.print(whilecount);
        Serial.print("\n");
        
        if(whilecount==5){
            hippoR =1;
            break;
          }
        //forward is backward... backward is forward
        Rmotor.setSpeed(MSpeed);
        Lmotor.setSpeed(MSpeed);
        Rmotor.run(FORWARD);
        Lmotor.run(BACKWARD);
        
        /*
        Serial.print("turning: ");
        Serial.print(heading);
        Serial.print("\n");
        */
        heading=Compass.GetHeadingDegrees();
    }

    if(heading<targetRHeading){
        hippoR = 0;
      }
    Rmotor.run(RELEASE);
    Lmotor.run(RELEASE);
  }


  void Forward(){
    //forward is backward... backward is forward
    //Serial.print("Forward");
    //Serial.print("\n");
    Rmotor.setSpeed(MSpeed);
    Lmotor.setSpeed(MSpeed);
    Rmotor.run(BACKWARD);
    Lmotor.run(BACKWARD);
    delay(t);

    LDistance = LeftPulse();
    if(LDistance > maximumRange){
          Rmotor.run(BACKWARD);
          delay(balanceTimer);
          Rmotor.run(RELEASE);
          Lmotor.run(BACKWARD);
          delay(balanceTimer);
          Lmotor.run(RELEASE);
      }
    if(LDistance < minimumRange){
          Lmotor.run(BACKWARD);
          delay(balanceTimer);
          Lmotor.run(RELEASE);
          Rmotor.run(BACKWARD);
          delay(balanceTimer);
          Rmotor.run(RELEASE);
      } 
    
    Rmotor.run(RELEASE);
    Lmotor.run(RELEASE);
    
}

  void Halt(){
    Rmotor.run(RELEASE);
    Lmotor.run(RELEASE);
    }

   long LeftPulse(){
      digitalWrite(L_Trig, LOW);
      delayMicroseconds(2);
      digitalWrite(L_Trig, HIGH);
      delayMicroseconds(20);
      LDistance = pulseIn(L_Echo, HIGH);
      return (LDistance / 58.2);
    }

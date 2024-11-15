/* 
*  The following directives are used by Doxygen for automated source code documentation
*  It is generally considered professional style to include these in each file's header
* 
*  @file MechatronicsDrive.ino
*  @brief The Drive System implementation for team Vibe Check final robot project
*
*  @author Steve Christensen
*  @date August 9, 2023
*  @version 1.0
*  Part of the Final Project for MCEN 5115 - Mechatronics and Robotics I
*  University of Colorado Boulder
*/ 

#define FALSE (0)
#define TRUE  (!FALSE)

#include <Motoron.h>

// Define the GPIO ports
const int FrontCornerOuter = 45; // These are the IR line sensor digital inputs
const int FrontCornerInner = 44;
const int FrontSideInner   = 43;
const int FrontSideOuter   = 42;
const int BackSideOuter    = 49;
const int BackSideInner    = 48;
const int BackCornerInner  = 47;
const int BackCornerOuter  = 46;

// Define the analog GPIO ports and associated constants
#define ADC_STEPS        (1024)
#define ADC_HYSTERESIS   (200)
#define ADC_THRESHOLD_HI ((ADC_STEPS/2)+(ADC_HYSTERESIS/2))
#define ADC_THRESHOLD_LO ((ADC_STEPS/2)-(ADC_HYSTERESIS/2))

int AnalogButton1;   // Use the analog inputs for buttons...
int AnalogButton2;
int DigitalButton1;  // ...and here are the digitized versions, which
int DigitalButton2;  //   use analog hysterisis for debouncing
int PreviousButton1; // We save the previous state of the buttons so we
int PreviousButton2; //   can act upon a change of state of the button
int TriggerButton1;  // The "Trigger" bits here are set on the initial
int TriggerButton2;  //   press of the corresponding button

// Define the Motoron motor controller constants
const int LeftControllerAddress = 15;
const int RightControllerAddress = 16;
const int FrontMotor = 1;
const int BackMotor = 2;
const int MinFwdSpeed = 0;
const int MaxFwdSpeed = 800;
const int DefaultAcceleration = 100;
const int DefaultDeceleration = 500;
const int DefaultCommandTimeout = 100;

// Define the driving states 
const int DriveIdle = 0;
const int DriveFindLine = 1;
const int DriveAlignBack = 2;
const int DriveAlignFront = 3;
const int DriveStraight = 6;
const int DriveInsideCorner = 7;
const int DriveStrafeFrontRight = 8;
const int DriveStrafeBackRight = 9;
const int DriveStrafeFrontLeft = 10;
const int DriveStrafeBackLeft = 11;
const int DriveOutsideCorner = 12;
const int DriveRotateRight = 13;
const int DriveRotateLeft = 14;

// Define the timing constants
const int VeerCorrection = 30;
const int DefaultSpeed = 400;
const int PivotSpeed = 300;
const int InsideCornerSpeedLeft = -300;
const int InsideCornerSpeedRight = 250;
const int RotateSlowSpeed = 250;
const int RotateFastSpeed = 350;
const int DefaultLoopTimeMs = 20;
const int BrakingLoopTimeMs = DefaultCommandTimeout - 5;
const int OvershootLoopTimeMs = DefaultCommandTimeout - 5;
const int AlignmentTimeMs = DefaultCommandTimeout - 5;

// Define the variables
int DriveState = DriveIdle;
int FrontRightSpeed = DefaultSpeed;
int FrontLeftSpeed = DefaultSpeed;
int BackRightSpeed = DefaultSpeed;
int BackLeftSpeed = DefaultSpeed;
int StraightCount = 0;
int LoopDelayTime = DefaultLoopTimeMs;

// Since the digital inputs change in realtime, we should read them once
// once per loop cycle and store the results in the int's below
int SensorFrontCornerOuter;
int SensorFrontCornerInner;
int SensorFrontSideInner;
int SensorFrontSideOuter;
int SensorBackSideOuter;
int SensorBackSideInner;
int SensorBackCornerOuter;

// This code creates an object for each Motoron controller.
// The number passed as the first argument to each constructor
// below should be the 7-bit I2C address of the controller.
MotoronI2C LeftCtrl(LeftControllerAddress);
MotoronI2C RightCtrl(RightControllerAddress);

// You can call functions directly on each of the objects
// created above (mc1, mc2, etc.) but if you want to write
// reusable code, the function below shows how to do that
// using references, a feature of the C++ language.
void setupMotoron(MotoronI2C & mc) {
  mc.reinitialize();
  mc.disableCrc();

  // Clear the reset flag, which is set after the controller
  // reinitializes and counts as an error.
  mc.clearResetFlag();
}

void setup() {
  Wire.begin();
  Serial.begin(9600);

  setupMotoron(LeftCtrl);
  setupMotoron(RightCtrl);

  LeftCtrl.setCommandTimeoutMilliseconds(DefaultCommandTimeout);
  RightCtrl.setCommandTimeoutMilliseconds(DefaultCommandTimeout);

  LeftCtrl.setMaxAcceleration(FrontMotor, DefaultAcceleration);
  LeftCtrl.setMaxDeceleration(FrontMotor, DefaultDeceleration);

  LeftCtrl.setMaxAcceleration(BackMotor, DefaultAcceleration);
  LeftCtrl.setMaxDeceleration(BackMotor, DefaultDeceleration);

  RightCtrl.setMaxAcceleration(FrontMotor, DefaultAcceleration);
  RightCtrl.setMaxDeceleration(FrontMotor, DefaultDeceleration);

  RightCtrl.setMaxAcceleration(BackMotor, DefaultAcceleration);
  RightCtrl.setMaxDeceleration(BackMotor, DefaultDeceleration);

  pinMode(FrontCornerInner, INPUT); // Set the IR sensor pins to inputs
  pinMode(FrontCornerOuter, INPUT);
  pinMode(FrontSideInner  , INPUT);
  pinMode(FrontSideOuter  , INPUT);
  pinMode(BackCornerInner , INPUT);
  pinMode(BackCornerOuter , INPUT);
  pinMode(BackSideInner   , INPUT);
  pinMode(BackSideOuter   , INPUT);

  LoopDelayTime = DefaultLoopTimeMs;

  Serial.println("Hello World!!!"); // Show sign of life
}

void loop() {
  delay(LoopDelayTime);
  LoopDelayTime = DefaultLoopTimeMs;
  ButtonsHandle();
  if (Serial.available()) {
    int SerialCommand = Serial.read();
    Serial.write(SerialCommand);
    switch (SerialCommand) {
      case 's': // Start
        Serial.println("Acks");
        LoopDelayTime = DefaultLoopTimeMs;
      case '1': // StrafeFrontRight
        DriveState = DriveFindLine;
        StrafeFrontRight();
        DriveContinue(); 
      break;
      case '2': // StrafeBackRight
        DriveState = DriveStrafeBackRight;
        StrafeBackRight();
        DriveContinue(); 
        break;
      case '3': // StrafeFrontLeft
        DriveState = DriveStrafeFrontLeft;
        StrafeFrontLeft();
        DriveContinue(); 
        break;
      case '4': // StrafeBackLeft
        DriveState = DriveStrafeBackLeft;
        StrafeBackLeft();
        DriveContinue(); 
        break;
      case 'c': // Take the corner
        Serial.println("Ackc");
        CornerLeft();
        DriveState = DriveInsideCorner;
        DriveContinue(); // Should check sensors first to see if we're already good
      break;
      case 'L': // Pivot left by shifting the back (shooter perspective) right
        FrontLeftGo(-PivotSpeed);
        BackLeftGo(-PivotSpeed);
        FrontRightGo(0);
        BackRightGo(0);
        Serial.println("AckR");
      break;
      case 'R': // Pivot right by shifting the back (shooter perspective) left
        FrontLeftGo(PivotSpeed);
        BackLeftGo(PivotSpeed);
        FrontRightGo(0);
        BackRightGo(0);
        Serial.println("AckL");
      break;
      //case 'd': // Backward
      case 'r': // Backward
        FrontLeftGo(-DefaultSpeed);
        BackLeftGo(-DefaultSpeed);
        FrontRightGo(-DefaultSpeed);
        BackRightGo(-DefaultSpeed);
        Serial.println("Ackr");
      break;
      //case 'f': // Forward
      case 'l': // Forward
        FrontLeftGo(DefaultSpeed);
        BackLeftGo(DefaultSpeed);
        FrontRightGo(DefaultSpeed);
        BackRightGo(DefaultSpeed);
        Serial.println("Ackl");
      break;
      default:
      break;
    }
  }
  if (TriggerButton1) { // Start by button
    TriggerButton1 = FALSE;
    Serial.println("Acks");
    StrafeFrontRight();
    DriveState = DriveFindLine;
    DriveContinue(); 
  }
  if (TriggerButton2) { // Corner by button
    TriggerButton2 = FALSE;
    CornerLeft();
    DriveState = DriveInsideCorner;
    DriveContinue();
  }

  // Sample the sensor inputs once per loop here:
  SensorFrontCornerOuter = !digitalRead(FrontCornerOuter);
  SensorFrontCornerInner = !digitalRead(FrontCornerInner);
  SensorFrontSideInner = !digitalRead(FrontSideInner);
  SensorFrontSideOuter = !digitalRead(FrontSideOuter);
  SensorBackSideOuter = !digitalRead(BackSideOuter);
  SensorBackSideInner = !digitalRead(BackSideInner);
  SensorBackCornerOuter = !digitalRead(BackCornerOuter);

  // Below is where the driving occurs
  switch ( DriveState ) {
    case DriveIdle: // Do nothing
    break;
    case DriveFindLine: // Find any righthand line
      if (SensorBackCornerOuter) {
        DriveForwardDefault(); // We backed up too far
        DriveContinue();
      }
      else if (SensorFrontSideOuter || SensorFrontSideInner) {
        if (!SensorBackSideInner) { // Test back coarse
          if (SensorBackSideOuter) { // Test back fine
            if (!SensorFrontSideInner) { // Test front fine
              //AllStop(); // We're on the line perfect
              DriveForwardDefault(); // We're on the line perfect
              DriveContinue();
              DriveState = DriveStraight;
            }
            else {
              StrafeFrontLeft(); // We're over the line on the front, and perfect in the back
              DriveContinue();
              DriveState = DriveAlignFront;
            }
          }
          else {
            StrafeBackRight(); // We're off the line on both back sensors
            DriveContinue();
            DriveState = DriveAlignBack;
          }
        }
        else {
          StrafeBackLeft(); // The back is over the line right
          DriveContinue();
          DriveState = DriveAlignBack;
        }
      }
      else if (SensorBackSideInner) {
        StrafeBackLeft(); // Don't go too far right with the back
        DriveContinue();
      } 
      else {
        StrafeFrontRight(); // Keep searching for a line on the front right
        DriveContinue();
      }
      break;

    case DriveAlignBack: // 
      if ((SensorBackSideOuter || SensorBackSideInner)) {
        if (!SensorFrontSideInner) { // Test front coarse
          if (SensorFrontSideOuter) { // Test front fine
            if (!SensorBackSideInner) { // Test back fine
              //AllStop(); // We're on the line perfect
              DriveForwardDefault(); // We're on the line perfect
              DriveContinue();
              DriveState = DriveStraight;
            }
            else {
              StrafeBackLeft(); // We're over the line on the back, and perfect in the front
              DriveContinue();
              DriveState = DriveAlignBack;
            }
          }
          else {
            StrafeFrontRight(); // We're off the line on both front sensors
            DriveContinue();
            DriveState = DriveAlignFront;
          }
        }
        else {
          StrafeFrontLeft(); // The front is over the line right
          DriveContinue();
          DriveState = DriveAlignFront;
        }
      }
      else {
        StrafeBackRight(); // Keep searching for a line on the back right
        DriveContinue();
      }
    break;

    case DriveAlignFront: //
      if (SensorFrontSideOuter || SensorFrontSideInner) {
        if (!SensorBackSideInner) { // Test back coarse
          if (SensorBackSideOuter) { // Test back fine
            if (!SensorFrontSideInner) { // Test front fine
              //AllStop(); // We're on the line perfect
              DriveForwardDefault(); // We're on the line perfect
              DriveContinue();
              DriveState = DriveStraight;
            }
            else {
              StrafeFrontLeft(); // We're over the line on the front, and perfect in the back
              DriveContinue();
              DriveState = DriveAlignFront;
            }
          }
          else {
            StrafeBackRight(); // We're off the line on both back sensors
            DriveContinue();
            DriveState = DriveAlignBack;
          }
        }
        else {
          StrafeBackLeft(); // The back is over the line right
          DriveContinue();
          DriveState = DriveAlignBack;
        }
      }
      else {
        StrafeFrontRight(); // Search for a line on the front right
        DriveContinue();
      }
      break;

    case DriveInsideCorner: // Turn left
      if (SensorFrontCornerOuter || SensorFrontCornerInner) {
        CornerLeft();
        DriveContinue();
      }
      else { // Go straight
        DriveContinue(); 
        DriveState = DriveFindLine;
      break;
    case DriveStraight:
      if (SensorFrontCornerOuter) { 
        Serial.println("Corner detected"); // The front edge is over a line 
        Serial.println("Ackc");
        AllStop();
      }
      else {
        if (SensorFrontSideInner) { // We have veered to the right, over the line
          if (FrontRightSpeed < MaxFwdSpeed) {
            FrontRightSpeed = DefaultSpeed + VeerCorrection;
          }
          if (FrontLeftSpeed > MinFwdSpeed) {
            FrontLeftSpeed = DefaultSpeed - VeerCorrection;
          }
          Serial.println("Front veered to the right");
        }
        else if (!SensorFrontSideOuter) { // We have veered to the left, off the line
          if (FrontRightSpeed > MinFwdSpeed) {
            FrontRightSpeed = DefaultSpeed - VeerCorrection;
          }
          if (FrontLeftSpeed < MaxFwdSpeed) {
            FrontLeftSpeed = DefaultSpeed + VeerCorrection;
          }
          Serial.println("Front veered to the left");
        }
        else { // This is the correct sensor readings for straight driving
          FrontRightSpeed = DefaultSpeed;
          FrontLeftSpeed = DefaultSpeed; 
        }
        DriveContinue(); // This sends the updated speed to all the wheels
      }
      break;

    case DriveStrafeFrontRight: // 
      if (SensorFrontSideOuter || SensorFrontSideInner) {
        AllStop();
        DriveState = DriveIdle;
      }
      else {
        StrafeFrontRight();
        DriveContinue(); // 
      }
      break;

    case DriveStrafeBackRight: // 
      if (SensorBackSideOuter || SensorBackSideInner) {
        AllStop();
        DriveState = DriveIdle;
      }
      else {
        StrafeBackRight();
        DriveContinue(); // 
      }
      break;

    case DriveStrafeFrontLeft:
      StrafeFrontLeft();
      DriveContinue();
      break;

    case DriveStrafeBackLeft:
      StrafeBackLeft();
      DriveContinue();
      break;

    case DriveRotateLeft:
      RotateLeft();
      DriveContinue();
      break;

    case DriveRotateRight:
      RotateRight();
      DriveContinue();
      break;

    default:
      Serial.println("Ackl"); // Lost
      AllStop();
      DriveState = DriveIdle;
    break;
  }
}

void AllStop() {
  DriveState = DriveIdle;
  LoopDelayTime = BrakingLoopTimeMs;
  LeftCtrl.setBraking(BackMotor, BackLeftSpeed);
  LeftCtrl.setBraking(FrontMotor, FrontLeftSpeed);  
  RightCtrl.setBraking(BackMotor, BackRightSpeed);  
  RightCtrl.setBraking(FrontMotor, FrontRightSpeed);    
}

void RotateLeft() {
  FrontLeftSpeed = -RotateSlowSpeed;
  BackLeftSpeed = -RotateSlowSpeed;
  FrontRightSpeed = RotateFastSpeed;
  BackRightSpeed = RotateFastSpeed;
}

void RotateRight() {
  FrontLeftSpeed = RotateSlowSpeed;
  BackLeftSpeed = RotateSlowSpeed;
  FrontRightSpeed = -RotateFastSpeed;
  BackRightSpeed = -RotateFastSpeed;
}

void StrafeFrontRight() {
  FrontLeftSpeed = -DefaultSpeed;
  BackLeftSpeed = DefaultSpeed;
  FrontRightSpeed = 0;
  BackRightSpeed = -DefaultSpeed;
}

void StrafeBackRight() {
  FrontLeftSpeed = -DefaultSpeed;
  BackLeftSpeed = DefaultSpeed;
  FrontRightSpeed = DefaultSpeed;
  BackRightSpeed = 0;
}

void StrafeFrontLeft() {
  FrontLeftSpeed = DefaultSpeed;
  BackLeftSpeed = -DefaultSpeed;
  FrontRightSpeed = 0;
  BackRightSpeed = DefaultSpeed;
}

void StrafeBackLeft() {
  FrontLeftSpeed = DefaultSpeed;
  BackLeftSpeed = -DefaultSpeed;
  FrontRightSpeed = -DefaultSpeed;
  BackRightSpeed = 0;
}

void CornerLeft() {
  FrontLeftSpeed = InsideCornerSpeedLeft;
  BackLeftSpeed = InsideCornerSpeedLeft;
  FrontRightSpeed = InsideCornerSpeedRight;
  BackRightSpeed = InsideCornerSpeedRight;
}

void DriveContinue() {
  FrontLeftGo(FrontLeftSpeed);
  BackLeftGo(BackLeftSpeed);
  FrontRightGo(FrontRightSpeed);
  BackRightGo(BackRightSpeed);
}

void DriveForwardDefault() {
  FrontLeftSpeed = DefaultSpeed; 
  BackLeftSpeed = DefaultSpeed;   
  FrontRightSpeed = DefaultSpeed;
  BackRightSpeed = DefaultSpeed; 
}

void FrontRightGo(int Speed) {
  RightCtrl.setSpeed(FrontMotor, Speed);
}

void BackRightGo(int Speed) {
  RightCtrl.setSpeed(BackMotor, Speed);
}

void FrontLeftGo(int Speed) {
  LeftCtrl.setSpeed(FrontMotor, -Speed);
}

void BackLeftGo(int Speed) {
  LeftCtrl.setSpeed(BackMotor, -Speed);
}

void ButtonsHandle() {
  AnalogButton1 = analogRead(A3);
  AnalogButton2 = analogRead(A2);

  if (AnalogButton1 > ADC_THRESHOLD_HI) {
    DigitalButton1 = HIGH;
  }
  else if (AnalogButton1 < ADC_THRESHOLD_LO) {
    DigitalButton1 = LOW;
  }

  if (AnalogButton2 > ADC_THRESHOLD_HI) {
    DigitalButton2 = HIGH;
  }
  else if (AnalogButton2 < ADC_THRESHOLD_LO) {
    DigitalButton2 = LOW;
  }

  if(DigitalButton1 == LOW) { // The switch is pressed
    if (PreviousButton1 == HIGH) { // Test for initial button press detection
      TriggerButton1 = TRUE;
    }
    PreviousButton1 = LOW;
  }
  else{
    PreviousButton1 = HIGH;
  }

  if(DigitalButton2 == LOW) { // The switch is pressed
    if (PreviousButton2 == HIGH) { // Test for initial button press detection
      TriggerButton2 = TRUE;
    }
    PreviousButton2 = LOW;
  }
  else{
    PreviousButton2 = HIGH;
  }
}



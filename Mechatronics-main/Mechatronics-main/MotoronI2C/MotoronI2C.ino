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
const int DefaultSpeed = 400;
const int DefaultAcceleration = 100;
const int DefaultDeceleration = 100;
const int DefaultCommandTimeout = 100;

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

/*
*/

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

  pinMode(FrontCornerInner, INPUT);           // set pin to input
  pinMode(FrontCornerOuter, INPUT);           // set pin to input
  pinMode(FrontSideInner  , INPUT);           // set pin to input
  pinMode(FrontSideOuter  , INPUT);           // set pin to input
  pinMode(BackCornerInner , INPUT);           // set pin to input
  pinMode(BackCornerOuter , INPUT);           // set pin to input
  pinMode(BackSideInner   , INPUT);           // set pin to input
  pinMode(BackSideOuter   , INPUT);           // set pin to input

  Serial.println("Hello World!!!");

}

void loop() {
  delay(10);

//  FrontLeftGo(DefaultSpeed);
//  BackLeftGo(DefaultSpeed);
//  FrontRightGo(DefaultSpeed);
//  BackRightGo(DefaultSpeed);

  if (Serial.available()) {
    int SerialCommand = Serial.read();

    //Serial.write(SerialCommand);
    switch (SerialCommand) {
      case 'L': // Pivot left by shifting the back (shooter perspective) right
        FrontLeftGo(-(DefaultSpeed/2));
        BackLeftGo(0);
        FrontRightGo(-(DefaultSpeed/2));
        BackRightGo(0);
        Serial.println("AckR");
      break;
      case 'R': // Pivot right by shifting the back (shooter perspective) left
        FrontLeftGo((DefaultSpeed/2));
        BackLeftGo(0);
        FrontRightGo((DefaultSpeed/2));
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
      /*
      //case 'a': // Strafe left
      case 'l': // Strafe left
        LeftCtrl.setSpeed(FrontMotor, -DefaultSpeed);
        LeftCtrl.setSpeed(BackMotor, DefaultSpeed);
        RightCtrl.setSpeed(FrontMotor, DefaultSpeed);
        RightCtrl.setSpeed(BackMotor, -DefaultSpeed);
      break;
      //case 's': // Strafe right
      case 'r': // Strafe right
        LeftCtrl.setSpeed(FrontMotor, DefaultSpeed);
        LeftCtrl.setSpeed(BackMotor, -DefaultSpeed);
        RightCtrl.setSpeed(FrontMotor, -DefaultSpeed);
        RightCtrl.setSpeed(BackMotor, DefaultSpeed);
      break;
      /*
      case 'e': // Pivot left
        LeftCtrl.setSpeed(FrontMotor, 0);
        LeftCtrl.setSpeed(BackMotor, DefaultSpeed);
        RightCtrl.setSpeed(FrontMotor, 0);
        RightCtrl.setSpeed(BackMotor, DefaultSpeed);
      break;
      case 'r': // Pivot right
        LeftCtrl.setSpeed(FrontMotor, DefaultSpeed);
        LeftCtrl.setSpeed(BackMotor, 0);
        RightCtrl.setSpeed(FrontMotor, DefaultSpeed);
        RightCtrl.setSpeed(BackMotor, 0);
      break;
      case 'q':
      break;
      case 'w':
      break;
      default:
      break;
      */
    }
  }
}

void FrontLeftGo(int Speed) {
  LeftCtrl.setSpeed(FrontMotor, -Speed);
}

void FrontRightGo(int Speed) {
  RightCtrl.setSpeed(FrontMotor, Speed);
}

void BackLeftGo(int Speed) {
  LeftCtrl.setSpeed(BackMotor, -Speed);
}

void BackRightGo(int Speed) {
  RightCtrl.setSpeed(BackMotor, Speed);
}



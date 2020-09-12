/* HOW TO RUN
 * add additional boards managers to preferences
 * https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
 * in board manager search for SparkFun and add board manager for pro micro
 * set board
 * set processor
 * set port
 * upload
 */

/* DESIGN SPEC
 * control four servos
 * set a mode
 * mode-zero = zero all servos (required for assembly)
 * mode-wave = control them in a wave like manner (creates movement)
 */


/* REFERENCE MATERIAL 
 * pins: https://images-na.ssl-images-amazon.com/images/I/81nOeGRzxPL._AC_SL1500_.jpg 
 */
 
 /* TODO
  * replace repetitive code with collections (e.g. arrays)
  */

#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h>

//// receiver
//#include <RH_ASK.h>

//// imu
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BNO055.h>
//#include <Wire.h>

//// receiver
//#define TxPin 10 
//#define RxPin 10 
//#define PowerPin 16

#define ServoPin1 18
#define ServoPin2 19
#define ServoPin3 20
#define ServoPin4 21

/* SETTINGS */

int ModeZero = 0;
int ModeWave = 1;

// mode
int mode = ModeZero; 
//int mode = ModeWave; 

// delay
float delayMs = 20.0;

// wave
float waveSpeed = 0.5; // nr of complete cycles per minute
float waveSize = 20.0; // max turning angle
float wavePhase = 0.75; // part of complete sine wave
int nrServos = 4.0;

// derive features
float nrStepsPerSecond = 1000.0 / delayMs;
float waveTimeStep = waveSpeed / nrStepsPerSecond;
float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos

/* INITIALIZE */

// servo globals
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

float servoPhasePos1 = 0.0;
float servoPhasePos2 = 0.0;
float servoPhasePos3 = 0.0;
float servoPhasePos4 = 0.0;

float servoPos1 = 90.0;
float servoPos2 = 90.0;
float servoPos3 = 90.0;
float servoPos4 = 90.0;

// time
unsigned long SECOND = 1000000; 
unsigned long current_time = micros();

//// receiver
//RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

/* SETUP */

void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");

  // servos
  servo1.attach(ServoPin1);
  servo2.attach(ServoPin2);
  servo3.attach(ServoPin3);
  servo4.attach(ServoPin4);

  servo1.write(servoPos1);
  servo2.write(servoPos2);
  servo3.write(servoPos3);
  servo4.write(servoPos4);
    
  // finalize
  Serial.println("setup complete");

}

/* RUN */

void loop() {

  /* input section */

  // pass - no inputs at this time

  /* control section */

  if (mode==ModeZero) {

//    control_zero();

  }

  if (mode==ModeWave) {

    control_wave(true);

  }

  /* act section */

  act_servos();

  delay(delayMs); 

}

/* INPUT */

// pass - no inputs at this time

/* CONTROL */

void control_zero() 
{
  servoPos1 = 90.0;
  servoPos2 = 90.0;
  servoPos3 = 90.0;
  servoPos4 = 90.0;
}

void control_wave(bool wordy) 
{
  // calculate phase
  servoPhasePos1 = (servoPhasePos1 + waveTimeStep); // % 1.0; // take remainer to avoid incrementing indefinitely, and improve interpretability
  servoPhasePos2 = servoPhasePos1 + waveServoStep;
  servoPhasePos3 = servoPhasePos2 + waveServoStep;
  servoPhasePos4 = servoPhasePos3 + waveServoStep;

  // convert to servo position
  servoPos1 = sin(unit_to_rad(servoPhasePos1)) * waveSize + 90.0;
  servoPos2 = sin(unit_to_rad(servoPhasePos2)) * waveSize + 90.0;
  servoPos3 = sin(unit_to_rad(servoPhasePos3)) * waveSize + 90.0;
  servoPos4 = sin(unit_to_rad(servoPhasePos4)) * waveSize + 90.0;

  // debug
  if (wordy) {

    String phaseStr = "Phase positions: ";
    phaseStr += String(servoPhasePos1, 2) + "; ";
    phaseStr += String(servoPhasePos2, 2) + "; ";
    phaseStr += String(servoPhasePos3, 2) + "; ";
    phaseStr += String(servoPhasePos4, 2) + "; ";

    String servoStr = "Servo positions: ";
    servoStr += String(servoPos1, 2) + "; ";
    servoStr += String(servoPos2, 2) + "; ";
    servoStr += String(servoPos3, 2) + "; ";
    servoStr += String(servoPos4, 2) + "; ";

    Serial.println(phaseStr);
    Serial.println(servoStr);
  }
}

/* ACT */

void act_servos() 
{
  servo1.write(servoPos1);
  servo2.write(servoPos2);
  servo3.write(servoPos3);
  servo4.write(servoPos4);
}

/* HELPERS */

float unit_to_rad(float unit)
{
  return unit * 6.28318531;
}

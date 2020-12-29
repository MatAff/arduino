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
//int mode = ModeZero; 
int mode = ModeWave; 

// global servo positions
float servoPosGlobal1 = 90.0;
float servoPosGlobal2 = 90.0;
float servoPosGlobal3 = 90.0;
float servoPosGlobal4 = 90.0;

// delay
float delayMs = 50.0;

// control class that can be shared between arduino and cpp
class SnakeControl
{

private:

    // delay
    float delayMs = 50.0;

    // wave
    float waveSpeed = 0.5; // nr of complete cycles per second
    float waveSize = 30.0; // max turning angle
    float wavePhase = 0.5; // part of complete sine wave
    int nrServos = 4.0;

    // derive features
    float nrStepsPerSecond = 1000.0 / delayMs;
    float waveTimeStep = waveSpeed / nrStepsPerSecond;
    float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos

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
    unsigned long setTime = micros();
    int prestartDurationSecs = 5;
    int runDurationSecs = 20;

public:

    float servoPosArr[5];

//    // cpp
//    long micros()
//    {
//        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    }

    float unit_to_rad(float unit)
    {
        return unit * 6.28318531;
    }

    void control_wave() 
    {
        // calculate phase
        servoPhasePos1 = (servoPhasePos1 + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
        if (servoPhasePos1 > 1.0) {
          servoPhasePos1 = servoPhasePos1 - 1.0;
        }
        servoPhasePos2 = servoPhasePos1 + waveServoStep;
        servoPhasePos3 = servoPhasePos2 + waveServoStep;
        servoPhasePos4 = servoPhasePos3 + waveServoStep;

        // convert to servo position
        servoPos1 = sin(unit_to_rad(servoPhasePos1)) * waveSize + 90.0;
        servoPos2 = sin(unit_to_rad(servoPhasePos2)) * waveSize + 90.0;
        servoPos3 = sin(unit_to_rad(servoPhasePos3)) * waveSize + 90.0;
        servoPos4 = sin(unit_to_rad(servoPhasePos4)) * waveSize + 90.0;

        // create array and return
        servoPosArr[0] = servoPos1;
        servoPosArr[1] = servoPos2;
        servoPosArr[2] = servoPos3;
        servoPosArr[3] = servoPos4;

    }

};


/* INITIALIZE */

// servo globals
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;


// time
unsigned long SECOND = 1000000; 
unsigned long setTime = micros();
int prestartDurationSecs = 5;
int runDurationSecs = 20;

//// receiver
//RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

/* SETUP */

// snake control
SnakeControl sc = SnakeControl();

void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");

  // servos
  servo1.attach(ServoPin1);
  servo2.attach(ServoPin2);
  servo3.attach(ServoPin3);
  servo4.attach(ServoPin4);


  
  // finalize
  Serial.println("setup complete");

  // wait and reset setTime
  while (micros() < (setTime + (prestartDurationSecs * SECOND))) {
    delay(delayMs);
  }
  setTime = micros();

}

/* RUN */

void loop() {

  /* input section */

  // pass - no inputs at this time

  /* control section */

  if (mode==ModeZero) {

     control_zero();

  }

  if (mode==ModeWave) {

    sc.control_wave();
    servoPosGlobal1 = sc.servoPosArr[0];
    servoPosGlobal2 = sc.servoPosArr[1];
    servoPosGlobal3 = sc.servoPosArr[2];
    servoPosGlobal4 = sc.servoPosArr[3];
//    control_wave(true);

  }

  /* act section */

  act_servos();

  delay(delayMs); 


  /* pause execution */
  
  while (micros() > setTime + (runDurationSecs * SECOND)) {
    delay(delayMs);
  }

}

/* INPUT */

// pass - no inputs at this time

/* CONTROL */

void control_zero() 
{
  servoPosGlobal1 = 90.0;
  servoPosGlobal2 = 90.0;
  servoPosGlobal3 = 90.0;
  servoPosGlobal4 = 90.0;
}

/* ACT */

void act_servos() 
{
  servo1.write(servoPosGlobal1 + 0.0);
  servo2.write(servoPosGlobal2 + 0.0);
  servo3.write(servoPosGlobal3 + 0.0);
  servo4.write(servoPosGlobal4 + 0.0);
}

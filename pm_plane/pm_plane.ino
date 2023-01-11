/* HOW TO RUN
   add additional boards managers to preferences
   https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
   in board manager search for SparkFun and add board manager for pro micro
   set board
   set processor
   add https://github.com/SMFSW/Queue (Download ZIP >> Sketch >> Incl library >> ZIP library
   set port
   upload
*/

/* TROUBLESHOOTING
 *  https://www.shellhacks.com/arduino-dev-ttyacm0-permission-denied/
 */

/* REFERENCE
   pins: https://cdn.sparkfun.com/assets/9/c/3/c/4/523a1765757b7f5c6e8b4567.png
*/

/* DESIGN SPEC
   adjust control surfaces to maintain roll target (defaults to 0.0)
   adjust tail to maintain pitch, reducte pitch when prop not running
   run motor at start kill after set time
*/

/* TODO
   include scale a global nature into variable names
   figure out why pitch > -20.0 work for cutting motor on decend
    line 237 (control pitch based on esc) and 345 (control esc based on pitch) should be together?
*/

//#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h>
#include <SD.h>
#include <cppQueue.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>

#define TxPin 10
#define RxPin 10
#define PowerPin A0 //16 // to 7
#define ServoPinR 6
#define ServoPinL 8
#define ServoPinTail 4
#define EscPin 9
#define IMPLEMENTATION FIFO
#define OVERWRITE true
#define SECOND 1000000

// sd globals
char fileName[] = "230110a.txt"; // SD library only supports up to 8.3 names
File fd;
const uint8_t chipSelect = 5; // 8;
const uint8_t cardDetect = 7; // 9;
bool alreadyBegan = false;  // SD.begin() misbehaves if not first call
bool cardExists = false;

// class to display fps at regular interval
class FPS {
  private:
    int count = 0;
    float fps = 0.0;
    long lastUpdateTime = micros();
  public:
    float getFps() {
      this->count += 1;
      long currentTime = micros();
      long deltaTime = currentTime - lastUpdateTime;

      // calculate fps when sufficient time has passed
      if (deltaTime > SECOND) {
        this->fps = float(count) / deltaTime * SECOND;
        this->count = 0.0;
        this->lastUpdateTime = currentTime;
        //        Serial.print("fps: ");
        //        Serial.println(this->fps);
      } else {
        return this->fps;
      }
    }
};

struct Segment {
  String name;
  float durationSecs;
  float targetRoll;
  float targetPitch;
  float propSpeed;
};

class RateOfChange {
  private:
    struct PosTime {
      float pos;
      unsigned long timee;
    };
    int speedSteps = 5;
    cppQueue posTimeQueue = cppQueue(sizeof(PosTime), speedSteps, IMPLEMENTATION, OVERWRITE);
  public:
    float rate = 0;
    float getRate(float pos, unsigned long timee) {
      PosTime posTime = {pos, timee};
      posTimeQueue.push(&posTime);
      int queueLength = posTimeQueue.getCount();
      if (queueLength == 0) {
        return rate;
      }
      PosTime prev;
      if (queueLength < speedSteps) {
        posTimeQueue.peekIdx(&prev, pos);
      } else {
        posTimeQueue.pop(&prev);
      }
      float deltaPos = pos - prev.pos;
      float deltaTime = timee - prev.timee;
      rate = deltaPos / deltaTime * SECOND;
      return rate;
    };
};

// header - predeclare functions to allow order change
extern float set_servo(Servo servo, float pos, bool do_invert = false, bool wordy = false);
extern void input_roll_pitch(bool wordy = false);
extern void control_roll_pitch(float rollTarget = 0.0, float pitchTarget = 0.0);
extern float set_esc(Servo esc, float pos, bool invert = false, bool wordy = false);
extern void control_journey(bool wordy = false);

// mode
#define CONTROLLED_JOURNEY 3 // refers to button D3
int mode = CONTROLLED_JOURNEY;

// servo globals
Servo servoR;
Servo servoL;
Servo servoTail;
float servoMin = 10.0;
float servoMax = 170.0;
float servoPosLeft = 0.0;
float servoPosRight = 0.0;
float servoPosTail = 0.0;
float servoLastLeft = -1.0;
float servoLastRight = -1.0;
float servoLastTail = -1.0;

// esc globals
Servo esc;
float escMin = 1200.0;
float escMax = 1800.0;
float escPos = -1.0;
float escLast = -1.0;
int escStepSize = 0.01; // in unit

// prop control
unsigned long escEndTime = micros() - (0 * SECOND);  // initialize esc end time as one second ago
float propSpeed = 0;

// roll pitch
float rollTarget = 0.0;
float pitchTarget = 0.0;

// receiver
//RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

// IMU globals
double roll = 0;
double pitch = 0;
double accX = 0;
double accY = 0;
double accZ = 0;
RateOfChange rollRate;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10;  // how often to read data from the board

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                    id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
unsigned long startTime = micros();

// fps
FPS fps;

// segment - duration, bank, lift, engine
Segment climb = {"climb", 4.0, 0.0, 20.0, 1.0};
Segment cruise = {"cruise", 8.0, 0.0, 0.0, 1.0};
Segment left = {"left", 3.0, 15.0, 0.0, 1.0};
Segment right = {"right", 3.0, -15.0, 0.0, 1.0};
Segment land = {"land", 3.0, 0.0, -20.0, -1.0};

// journey
Segment journey[] = {climb, cruise, land};
// Segment journey[] = {cruise, land};
int segmentIndex = -1;
int lastSegmentIndex = 2 - 1; // TODO: simplify, compare to len(journey) instead of defining last
Segment currentSegment;
float segmentEndTime = 0.0;

/* SETUP */

void setup()
{
  Serial.begin(9600);  // Debugging only
//    Serial.begin(57600);  // Debugging only
    Serial.println("Initializing");
  //  while (!Serial); // don't run if not plugged in via USB

  // servos
  Serial.println("Setting up servos");
  servoR.attach(ServoPinR);
  servoL.attach(ServoPinL);
  servoTail.attach(ServoPinTail);
  set_servo(servoL, servoPosLeft);
  set_servo(servoR, servoPosRight);
  set_servo(servoR, servoPosTail);
  delay(15);

  // esc
  Serial.println("Setting up esc");
  esc.attach(EscPin);
  esc_calibrate();

  // // receiver
  // if (!driver.init()) {
  // //    Serial.println("init failed - receiver");
  //   while (1); // don't run if receiver not detected
  // }

  // imu
  Serial.println("Setting up imu");
  if (!bno.begin()) {
    //    Serial.print("No BNO055 detected");
    while (1); // don't run if imu not detected
  }
  delay(1000);

  // finalize
  //  Serial.println("setup complete");

  // fps
  Serial.println("Setting up fps");
  fps = FPS();

  // logging
  Serial.println("Setting up sd card");
  pinMode(cardDetect, INPUT);
  setup_sd_card();
}

/* RUN */

void loop() {

  // input section
  input_roll_pitch(false); // imu
  // int sentCode = input_receiver();
  fps.getFps();

  // control section
  if (mode == CONTROLLED_JOURNEY) {
    control_journey(false);
    control_prop();
    if (propSpeed < 0.0) {
      pitchTarget = -15.0; // adjust desired pitch based on escPos
    }
    control_roll_pitch(rollTarget, pitchTarget);
  }

  // act section
  act_servos();
  act_esc();
  write_log_data();
  delay(15);

}

/* INPUT */

void input_roll_pitch(bool wordy)
{
  // set global roll and pitch
  sensors_event_t orientationData, linearAccelData;

  //  triggered after delay
  if ((micros() - startTime) > (BNO055_SAMPLERATE_DELAY_MS * 1000)) {

    startTime = micros(); // reset timer

    // get orientation (degrees 0 to 359) and standardize (-1 to 1)
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    roll = roll_standardize(orientationData.orientation.z);
    pitch = pitch_standardize(orientationData.orientation.y);

    // get acceleration data (m/s^2)
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    accX = linearAccelData.acceleration.x;
    accY = linearAccelData.acceleration.y;
    accZ = linearAccelData.acceleration.z;

    // Get rate of change
    rollRate.getRate(roll, micros());

    if (wordy) {
      printEvent(&orientationData);
      messageValue("Roll: ", roll, wordy);
      messageValue("Pitch: ", pitch, wordy);
    }
  }
}

//int input_receiver()
//{
//  // return int of message received or zero
//  uint8_t buf[50];
//  uint8_t buflen = sizeof(buf);
//  memset(buf, 0, buflen);
//  int sentCode = 0;
//
//  if (driver.recv(buf, &buflen)) {
//    sentCode = String((char*)buf).toInt();  // update sent code
//    //    Serial.print("Message: ");
//    //    Serial.println(sentCode);
//  }
//
//  return sentCode;
//
//}

/* CONTROL */

void control_journey(bool wordy = false) {
  float currentTime = micros();
  if (currentTime > segmentEndTime) {
    if (segmentIndex < lastSegmentIndex) { // TODO: drop use of lastSegmentIndex
      segmentIndex++;
      currentSegment = journey[segmentIndex];
      segmentEndTime = micros() + (currentSegment.durationSecs * SECOND);
      if (wordy) {
        Serial.print("Starting segment ");
        Serial.print(segmentIndex);
        Serial.print("/");
        Serial.print(lastSegmentIndex);
        Serial.print(" ");
        Serial.println(currentSegment.name);
      }
    } else {
      currentSegment = land;
      segmentEndTime = (micros() + 3600) * SECOND;
    }

    pitchTarget = currentSegment.targetPitch;
    rollTarget = currentSegment.targetRoll;
    propSpeed = currentSegment.propSpeed;

    messageValue("rollTarget: ", rollTarget, wordy);
    messageValue("pitchTarget: ", pitchTarget, wordy);
    messageValue("propSpeed: ", propSpeed, wordy);
  }
}

void control_roll_pitch(float rollTarget = 0.0, float pitchTarget = 0.0)
{
  float rollDelta = rollTarget - roll;
  float pitchDelta = pitchTarget - pitch;
  float rollP = 0.1;
  float pitchP = -0.05;

  // updates left right tail globals
  servoPosLeft = limit(rollDelta * rollP, -1.0, 1.0);
  servoPosRight = limit(rollDelta * rollP, -1.0, 1.0);
  servoPosTail = limit(pitchDelta * pitchP, -1.0, 1.0);
}

// DEB - sets escPos
bool control_prop() // int sentCode)
{

  // set esc target - update global
  if (pitch > -20.0) {
    escPos = propSpeed;
    return true;
  } else {
    escPos = -1.0; // TODO: Update code so stop can be 0.0
    return false;
  }

}

/* ACT */

void act_servos()
{

  if (servoLastLeft != servoPosLeft) {
    servoLastLeft = set_servo(servoL, servoPosLeft, true, false);
  }

  if (servoLastRight != servoPosRight) {
    servoLastRight = set_servo(servoR, servoPosRight, true, false);
  }

  if (servoLastTail != servoPosTail) {
    servoLastTail = set_servo(servoTail, servoPosTail, false, false);
  }
}

// DEB - calculates escPosLimit and passes to set_esc
void act_esc() {

  if (escLast != escPos) {

    // limit to steps size
    //    float escPosLimit = escLast + limit(escPos - escLast, escStepSize, -escStepSize);
    float escPosLimit = escPos;

    //    Serial.print("escPosLimit: ");
    //    Serial.println(escPosLimit);

    escLast = set_esc(esc, escPosLimit, false, true);
  }
}

/* HELPERS */

void messageValue(String msg, float val, bool wordy) {
  if (wordy) {
    Serial.print(msg);
    Serial.println(val);
  }
}

float scale(float val, float inMin, float inMax, float outMin, float outMax) {
  return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

float limit(float val, float minVal, float maxVal) {
  return min(max(val, minVal), maxVal);
}

float invert(float val, float minVal, float maxVal) {
  return maxVal - val + minVal;
}

float set_servo(Servo servo, float pos, bool do_invert = false, bool wordy = false)
{
  // scale, limit, invert, write
  float posScaled = scale(pos, -1.0, 1.0, servoMin, servoMax);
  posScaled = limit(posScaled, servoMin, servoMax);
  if (do_invert) {
    posScaled = invert(posScaled, servoMin, servoMax);
  }
  servo.write(posScaled);

  messageValue("Setting servo position: ", posScaled, wordy);

  return pos;
}

// DEB - scales, writes and returns
float set_esc(Servo esc, float pos, bool invert = false, bool wordy = false) {

  // scale, limit, write
  float posScaled = scale(pos, -1.0, 1.0, escMin, escMax);
  posScaled = limit(posScaled, escMin, escMax);
  esc.writeMicroseconds(posScaled);

  messageValue("Setting esc position: ", posScaled, wordy);

  return pos;
}

float roll_standardize(float roll) {
  return roll;
}

float pitch_standardize(float pitch) {
  return -pitch;
}

void esc_calibrate_step(String msg, int val, int delayMs) {
  //  Serial.println(msg);
  esc.writeMicroseconds(val);
  delay(delayMs);
}

void esc_calibrate() {
  esc_calibrate_step("esc - low", escMin, 2000);
  esc_calibrate_step("esc - high", escMax, 2000);
  esc_calibrate_step("esc - low", escMin, 2000);
}

void printEvent(sensors_event_t* event) {
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if ((event->type == SENSOR_TYPE_GYROSCOPE) || (event->type == SENSOR_TYPE_ROTATION_VECTOR)) {
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }

  Serial.print(event->type);
  Serial.print(": x= ");
  Serial.print(x);
  Serial.print(" | y= ");
  Serial.print(y);
  Serial.print(" | z= ");
  Serial.println(z);
}

void write_log_data(void) {
  Serial.println(cardExists);
  if (cardExists) {
    fd = SD.open(fileName, FILE_WRITE);
    Serial.println(fd);  // 0 = False
    if (fd) {
      Serial.println("writing to sd");
      // HEADERS
      // "ts","segment","roll","pitch","servoPosLeft","servoPosRight","servoPosTail","esc","roll_target","pitch_target","esc_target","accX","accY","accZ","rollRate"
      fd.print(micros()); fd.print(",");
      fd.print(currentSegment.name); fd.print(",");
      // roll/pitch sensor
      fd.print(roll); fd.print(",");
      fd.print(pitch); fd.print(",");
      // servo/motor positions
      fd.print(servoPosLeft); fd.print(",");
      fd.print(servoPosRight); fd.print(",");
      fd.print(servoPosTail); fd.print(",");
      fd.print(escPos); fd.print(",");
      // servo/motor targets
      fd.print(rollTarget); fd.print(",");
      fd.print(pitchTarget); fd.print(",");
      fd.print(propSpeed); fd.print(",");
      // accelerometer
      fd.print(accX); fd.print(",");
      fd.print(accY); fd.print(",");
      fd.print(accZ); fd.print(",");
      // rate of change
      fd.print(rollRate.rate); # fd.print(",");
      fd.print("\n");

      fd.flush();
      fd.close();
    }
  }
}

void setup_sd_card(void) {
  if (digitalRead(cardDetect)) {
    if (!SD.begin(chipSelect) && !alreadyBegan) {
      // begin uses half-speed...
    } else {
      alreadyBegan = true;
    }
    delay(250); // Debounce insertion
    Serial.println("Card detected");
    cardExists = true;
  } else {
    Serial.println("No card detected");
    cardExists = false;
  }
}

void update_sd_card(void) {
  if (!digitalRead(cardDetect)) {
    if (cardExists) {
      Serial.println("Card removed");
      cardExists = false;
    }
  }
}

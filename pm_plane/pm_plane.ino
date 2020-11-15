/* HOW TO RUN
   add additional boards managers to preferences
   https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
   in board manager search for SparkFun and add board manager for pro micro
   set board
   set processor
   set port
   upload
*/

/* REFERENCE
 * pins: https://cdn.sparkfun.com/assets/9/c/3/c/4/523a1765757b7f5c6e8b4567.png
 */

/* DESIGN SPEC
   adjust control surfaces to maintain roll target (defaults to 0.0)
   adjust tail to maintain pitch, reducte pitch when prop not running
   run motor at start kill after set time
*/

/* TODO
   update code to adhere to style guide
   include scale a global nature into variable names
   update scale function to rescale from any scale
   write tests for functions
   figure out why pitch > -20.0 work for cutting motor on decend
   investigate options for strucs/classes >> Done
   add tail control based on pitch >> Done
*/

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h>
#include <SD.h>

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

char fileName[] = "log201115.txt"; // SD library only supports up to 8.3 names
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
    long SECOND = 1000000;
  public:
    get_fps() {
      this->count += 1;
      long currentTime = micros();
      long deltaTime = currentTime - lastUpdateTime;

      // calculate fps when sufficient time has passed
      if (deltaTime > this->SECOND) {
        this->fps = float(count) / deltaTime * this->SECOND;
        this->count = 0.0;
        this->lastUpdateTime = currentTime;
        Serial.print("fps: ");
        Serial.println(this->fps);
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

// header - predeclare function to allow order change
extern float set_servo(Servo servo, float pos, bool do_invert = false, bool wordy = false);
extern void input_roll_pitch(bool wordy = false);
extern void control_roll_pitch(float rollTarget = 0.0, float pitchTarget = 0.0);
extern float set_esc(Servo esc, float pos, bool invert = false, bool wordy = false);
extern void control_journey(bool wordy = false);

// mode
int mode = 3; // refers to button D3

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
unsigned long SECOND = 1000000;
unsigned long escEndTime = micros() - (0 * SECOND);  // initialize esc end time as one second ago
float escDurSecs = 5.0; // TODO: use journey instead
float propSpeed = 0.75; // close to max // TODO: use journey instead

// roll pitch
float rollTarget = 0.0;
float pitchTarget = 0.0;

// receiver
RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

// IMU globals
double roll = 0;
double pitch = 0;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10;  // how often to read data from the board

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                    id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);  // TODO: unused, determine relevance
unsigned long startTime = micros();

// fps
FPS fps;

// segment
Segment takeoff = {"takeoff", 4.0, 0.0, 25.0, 1.0};
Segment cruise = {"cruise", 2.0, 0.0, 15.0, 1.0};
Segment left = {"left", 3.0, 15.0, 0.0, 0.25};
Segment right = {"right", 3.0, -15.0, 0.0, 0.25};
Segment land = {"land", 3.0, 0.0, -20.0, -1.0};

// journey
Segment journey[] = {takeoff, cruise, land};
int segmentIndex = -1;
int lastSegmentIndex = 3 - 1;
Segment currentSegment;
float segmentEndTime = 0.0;

/* SETUP */

void setup()
{
//  Serial.begin(9600);  // Debugging only
  Serial.begin(57600);  // Debugging only
  Serial.println("Initializing");
  while (!Serial);

  // servos
  servoR.attach(ServoPinR);
  servoL.attach(ServoPinL);
  servoTail.attach(ServoPinTail);
  set_servo(servoL, servoPosLeft);
  set_servo(servoR, servoPosRight);
  set_servo(servoR, servoPosTail);

  // esc
  esc.attach(EscPin);
  set_servo(esc, escPos); // TODO: drop, this shouldn't do anything
  esc_calibrate();

  // receiver
  if (!driver.init()) {
    Serial.println("init failed - receiver");
    while (1); // TODO: figure out what this does
  }

  // imu
  if (!bno.begin()) {
    Serial.print("No BNO055 detected");
    while (1); // TODO: figure out what this does
  }
  delay(1000);

  // finalize
  Serial.println("setup complete");

  // set kill time
  // escEndTime = micros() + (escDurSecs * SECOND);

  // fps
  fps = FPS();

  // logging
  pinMode(cardDetect, INPUT);
  setup_sd_card();
}

/* RUN */

void loop() {

  /* input section */

  // imu
  input_roll_pitch(false);

  // receiver
  int sentCode = input_receiver();

  // fps
  fps.get_fps();

  /* control section */

  // decision logic
  if (mode == 3) { // TODO use named constants instead of numbers

    control_journey(true);
    
    // prop
    control_prop(sentCode);

    // adjust desired pitch based on escPos
    if (propSpeed < 0.0) {
      pitchTarget = -15.0;
    }

    // roll
    control_roll_pitch(rollTarget, pitchTarget);

  }

  /* act section */

  // servos
  act_servos();

  // esc
  act_esc();

  // log
  write_log_data();

}

/* INPUT */

void input_roll_pitch(bool wordy = false)
{
  // set global roll and pitch
  sensors_event_t orientationData, linearAccelData;

  //  triggered after delay
  if ((micros() - startTime) > (BNO055_SAMPLERATE_DELAY_MS * 1000)) {

    startTime = micros(); // reset timer

    // get orientation and standardize
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    roll = roll_standardize(orientationData.orientation.z);
    pitch = pitch_standardize(orientationData.orientation.y);

    if (wordy) {
      printEvent(&orientationData);
      Serial.print("Roll: ");
      Serial.println(roll);
      Serial.print("Pitch: ");
      Serial.println(pitch);
    }
  }
}

int input_receiver()
{
  // return int of message received or zero
  uint8_t buf[50];
  uint8_t buflen = sizeof(buf);
  memset(buf, 0, buflen);
  int sentCode = 0;

  if (driver.recv(buf, &buflen)) {
    sentCode = String((char*)buf).toInt();  // update sent code
    Serial.print("Message: ");
    Serial.println(sentCode);
  }

  return sentCode;

}

/* CONTROL */

void control_journey(bool wordy = false) {
  float currentTime = micros();
  if (currentTime > segmentEndTime) {
    if (segmentIndex < lastSegmentIndex) {
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

    if (wordy) {
      message_value("rollTarget: ", rollTarget);
      message_value("pitchTarget: ", pitchTarget);
      message_value("propSpeed: ", propSpeed);
    }
  }
}

void control_roll_pitch(float rollTarget = 0.0, float pitchTarget = 0.0)
{
  float rollMax = 10.0;
  float rollDelta = rollTarget - roll;
  float pitchMax = 10.0;
  float pitchDelta = pitchTarget - pitch;

  // updates left right globals
  servoPosLeft = limit(rollDelta / rollMax, -1.0, 1.0);
  servoPosRight = limit(rollDelta / rollMax, -1.0, 1.0);

  // update tail global
  servoPosTail = limit(pitchDelta / pitchMax, -1.0, 1.0) * -1.0;
}

// DEB - sets escPos
bool control_prop(int sentCode)
{

  // update end esc end time
  if (sentCode == 1) {
    escEndTime = micros() + (escDurSecs * SECOND);
    Serial.println("setting");
    Serial.println(escEndTime);
  }

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

    Serial.print("escPosLimit: ");
    Serial.println(escPosLimit);

    escLast = set_esc(esc, escPosLimit, false, true);
  }
}

/* HELPERS */

void message_value(String msg, float val) {
  Serial.print(msg);
  Serial.println(val);  
}

float scale(float val, float inMin, float inMax, float outMin, float outMax) {
  return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;  
}

float limit(float val, float minVal, float maxVal)
{
  return min(max(val, minVal), maxVal);
}

float invert(float val, float minVal, float maxVal) {
  return maxVal - val + minVal;
}

float set_servo(Servo servo, float pos, bool do_invert = false, bool wordy = false)
{
  // scale, limit, invert, write
  float posScaled = scale(pos, -1.0, 1.0, servoMin, servoMax);
  posScaled = min(max(posScaled, servoMin), servoMax);
  if (do_invert) {
    posScaled = invert(posScaled, servoMin, servoMax);
  }
  servo.write(posScaled);

  if (wordy) { message_value("Setting servo position: ", posScaled); }

  delay(15);  // TODO: move delay to overall loop

  return pos;
}

// DEB - scales, writes and returns
float set_esc(Servo esc, float pos, bool invert = false, bool wordy = false) {

  // scale, limit, write
  float posScaled = scale(pos, -1.0, 1.0, escMin, escMax);
  posScaled = limit(posScaled, escMin, escMax);
  esc.writeMicroseconds(posScaled);

  if (wordy) { message_value("Setting esc position: ", posScaled); }

  delay(15); // TODO: move to overall loop

  return pos;
}

float roll_standardize(float roll)
{
  return roll - 90.0;
}

float pitch_standardize(float pitch)
{
  return -pitch;
}

void esc_calibrate_step(String msg, int val, int delayMs) {
  Serial.println(msg);
  esc.writeMicroseconds(val);
  delay(delayMs);
}

void esc_calibrate() {
  esc_calibrate_step("esc - low", escMin, 2000);
  esc_calibrate_step("esc - high", escMax, 2000);
  esc_calibrate_step("esc - low", escMin, 2000);
}

void printEvent(sensors_event_t* event) {
  Serial.println();
  Serial.print(event->type);
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

  Serial.print(": x= ");
  Serial.print(x);
  Serial.print(" | y= ");
  Serial.print(y);
  Serial.print(" | z= ");
  Serial.println(z);
}

void write_log_data(void) {

  if (cardExists) {
    fd = SD.open(fileName, FILE_WRITE);
    if (fd) {
      Serial.println("writing to sd");
      fd.print(micros()); fd.print(",");
      fd.print(currentSegment.name); fd.print(",");
      fd.print(roll); fd.print(",");
      fd.print(pitch); fd.print(",");
      fd.print(servoPosLeft); fd.print(",");
      fd.print(servoPosRight); fd.print(",");
      fd.print(servoPosTail); fd.print(",");
      fd.print(escPos);
      fd.print("\n");
      
      fd.flush();
      fd.close();
    }
  }
}

void setup_sd_card(void) {
  if (digitalRead(cardDetect)) {
    if (!SD.begin(chipSelect) && !alreadyBegan) { // begin uses half-speed...
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

// UNUSED CODE BELOW

//float scale_unit(float val, float minVal, float maxVal, bool limit, bool invert)
//{
//  /* scale val from unit range [-1, 1] to [minVal, maxVal] */
//
//  val = (val + 1.0) / 2.0 * (maxVal - minVal) + minVal;
//
//  if (limit) {
//    val = min(max(val, minVal), maxVal);
//  }
//
//  if (invert) {
//    val = maxVal - val + minVal;
//  }
//
//  return val;
//}


//// translate x or y stick values to -1 to 1 range
//float stick_trans(float xy)
//{
//  return xy * 2 -1;
//}
//
//// round to zero if value is near zero
//float stick_near_zero(float xy, float thresh = 0.05)
//{
//  if (abs(xy - 0.0) < thresh) {
//    return 0.0;
//  } else {
//    return xy;
//  }
//}
//
//float stick_offset_x(float x)
//{
//  return max(x - 0.02, 0.0);
//}
//

//class Segment {
//  private:
//    String name;
//    float durationSecs;
//    float targetRoll;
//    float targetPitch;
//    float propSpeed;
//  public:
//    Segment(String name, float durationSecs, float targetRoll, float targetPitch, float propSpeed) {
//      this->name = name;
//      this->durationSecs = durationSecs;
//      this->targetRoll = targetRoll;
//      this->targetPitch = targetPitch;
//      this->propSpeed = propSpeed;
//      init();
//    }
//
//    void init() { }
//
//    String getName() { return this->name; }
//    float getDurationSecs() { return this->durationSecs; }
//    float getTargetRoll() { return this->targetRoll; }
//    float getTargetPitch() { return this->targetPitch; }
//    float getPropSpeed() { return this->propSpeed; }
//
//};

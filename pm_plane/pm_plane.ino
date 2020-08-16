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
 * adjust control surfaces to maintain roll target (defaults to 0.0)
 * when passed signal temporatily power esc)
 */

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <Servo.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>

#define TxPin 10
#define RxPin 10
#define PowerPin 16
#define ServoPinR 9
#define ServoPinL 8
#define EscPin 7 // TODO: verify this can be used

// mode
int mode = 3; // refers to button D3

// servo globals
Servo servoR;
Servo servoL;
float servoMin = 10.0;
float servoMax = 170.0;
float servoPosLeft = 0.0;
float servoPosRight = 0.0;
float servoLastLeft = -1.0;
float servoLastRight = -1.0;

// esc globals
Servo esc;
float escMin = 1200.0;
float escMax = 1800.0;
float escPos = 0.0;
float escLast = 0.0;

// prop control
unsigned long SECOND = 1000000; 
unsigned long escEndTime = micros() - SECOND;  // initialize esc end time as one second ago
float escDurSecs = 1.0;

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


float scale_unit(float val, float minVal, float maxVal, bool limit, bool invert) 
{
  /* scale val from unit range [-1, 1] to [minVal, maxVal] */
  
  val = (val + 1.0) / 2.0 * (maxVal - minVal) + minVal;

  if (limit) { 
    val = min(max(val, minVal), maxVal);
  }
  
  if (invert) {
    val = maxVal - val + minVal;
  }

  return val; 
}

float limit(float val, float minVal, float maxVal)
{
  return min(max(val, minVal), maxVal);
}

float set_servo(Servo servo, float pos, bool invert=false, bool wordy=false) 
{
  pos = scale_unit(pos, servoMin, servoMax, true, invert);
  
  if (wordy) {
    Serial.print("Setting position: ");
    Serial.println(pos);
  }

  servo.write(pos);
  
  delay(15);  // TODO: move delay to overall loop
  
  return pos;
}

float set_esc(Servo esc, float pos) {
  // TODO: write this
}

float roll_standardize(float roll)
{
  return roll;
}

float pitch_standardize(float pitch)
{
  return -pitch;
}

void input_roll_pitch(bool wordy=false) 
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

void control_roll(float rollTarget=0.0, float pitchTarget=0.0)
{
  float rollMax = 15.0;
  float rollDelta = rollTarget - roll;

  // updates globals
  servoPosLeft = limit(rollDelta / rollMax, -1.0, 1.0);
  servoPosRight = limit(rollDelta / rollMax, -1.0, 1.0) * -1.0;
}

bool control_prop(int sentCode)
{

  // update end esc end time
  if (sentCode==1) {
    escEndTime = micros() + (escDurSecs * SECOND); 
    Serial.println("setting");
    Serial.println(escEndTime);
  }

  // set esc target - update global
  if (micros() < escEndTime) {
    escPos = 0.5;
    return true;
  } else {
    escPos = 0.0;
    return false;
  }

}

void act_servos()
{
  
  if (servoLastLeft != servoPosLeft) {
    servoLastLeft = set_servo(servoL, servoPosLeft, true, false);
  }
  
  if (servoLastRight != servoPosRight) {
    servoLastRight = set_servo(servoR, servoPosRight, false, false);
  }

}

void act_esc() {
  if (escLast != escPos) {
    // TODO: review this functionality
    escLast = set_servo(esc, escPos, false, false);
  }
}

// SETUP PHASE
void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");

  // servos
  servoR.attach(ServoPinR);
  servoL.attach(ServoPinL);
  set_servo(servoL, servoPosLeft);
  set_servo(servoR, servoPosRight);
  
  // esc
  esc.attach(EscPin);
  set_servo(esc, escPos);

  // receiver
  if (!driver.init()) {
    Serial.println("init failed - receiver");
    while (1);
  }
  
  // imu
  if (!bno.begin()) {
    Serial.print("No BNO055 detected");
    while (1);
  }

  // TODO: add esc calibration sequence here

  delay(1000);
  Serial.println("setup complete");
}


// RUN PHASE
void loop() {

  /*** input section ***/

  // imu
  input_roll_pitch();

  // receiver
  int sentCode = input_receiver();

  /*** control section ***/

  // decision logic
  if (mode==3) {

    // roll
    control_roll();

    // prop
    control_prop(sentCode);

  }

  /*** act section ***/

  // servos
  act_servos();

  // esc
  act_esc();

}




// translate x or y stick values to -1 to 1 range
float stick_trans(float xy)
{
  return xy * 2 -1;
}

// round to zero if value is near zero
float stick_near_zero(float xy, float thresh = 0.05)
{  
  if (abs(xy - 0.0) < thresh) {
    return 0.0;
  } else {
    return xy;
  }
}

float stick_offset_x(float x)
{
  return max(x - 0.02, 0.0);
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

// OLD STUFF BELOW // OLD STUFF BELOW // OLD STUFF BELOW //

//double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

//int servo_positions[3] = {10, 90, 170};
//int servo_position = 90;


//  Serial.print("Servo position: ");
//  Serial.println(servo_position);

  // bno.getEvent(&angVelData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  // bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);


    /*int servo_adjust = String((char*)buf).toInt();
    if (servo_adjust >= 1 && servo_adjust <= 3) {
      servo_position = servo_positions[servo_adjust-1];
    }*/

    
    //double z = orientationData.orientation.z;

    /*if (z > 10) 
    {
      servo_position = servo_positions[0];
      Serial.println("right");
    }
    if (z < -10) 
    {
      servo_position = servo_positions[2];
      Serial.println("left");
    }*/

 //float unit_to_servo(float xy, float servoMin, float servoMax)
//{
//  /* convert range [-1, 1] to [servoMin, servoMax] */
//  return (xy + 1.0) / 2.0 * (servoMax - servoMin) + servoMin;
//}
//
//
//float invert(float value, float min_value, float max_value)
//{
//  return max_value - value + min_value;
//}

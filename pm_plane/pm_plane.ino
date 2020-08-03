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
 * when passed signal temporatily override servo pos (will replace with prop throttle when working)
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
#define PropPin 7

// mode
int mode = 3; // refers to button D3

// servo globals
Servo servoR;
Servo servoL;
float servo_pos_left = 0.0;
float servo_pos_right = 0.0;
float servo_last_left = 0.0; // TODO: seems like last should not be set the same as current (as initial position won't get set)
float servo_last_right = 0.0;
float servo_min = 10.0;
float servo_max = 170.0;

// prop globals
Servo prop;
float prop_last = 0.0;
float prop_pos = 0.0;
float prop_min = 0.0;
float prop_max = 10.0;

// receiver
RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

// IMU globals
double roll = 0;
double pitch = 0; 
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; // how often to read data from the board
uint16_t SECOND = 1000; 
unsigned long tStartOne = micros() - (10 * SECOND);

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                    id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
unsigned long tStart = micros();

// limit value to range
float limit(float value, float min_value=-1.0, float max_value=1.0)
{
  return min(max(value, min_value), max_value);
}

float unit_to_servo(float xy)
{
  return (xy + 1.0) / 2.0 * (servo_max - servo_min) + servo_min;
}

float set_servo(Servo servo, float pos, bool invert=false, bool wordy=false) 
{
  pos = unit_to_servo(pos);
  pos = limit(pos, servo_min, servo_max);
  if (invert) {
    pos = servo_max - pos + servo_min;
  }
  
  if (wordy) {
    Serial.print("Setting position: ");
    Serial.println(pos);
  }

  servo.write(pos);
  delay(15);
  
  return pos;
}

// set prop using wrapper around set_servo
float set_prop(Servo servo, float pos, bool invert=false, bool wordy=false) 
{
  return set_servo(servo, pos, false, wordy);
}

void input_roll_pitch(bool wordy=false) 
{
  // set global roll and pitch 
  sensors_event_t orientationData, linearAccelData;

  //  triggered after delay
  if ((micros() - tStart) > (BNO055_SAMPLERATE_DELAY_MS * 1000))
  {
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    tStart = micros();
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
  int sent_code = 0;

//  Serial.println("Checking receiver");

  if (driver.recv(buf, &buflen)) {
    Serial.print("Message: ");
    Serial.println((char*)buf);
    sent_code = String((char*)buf).toInt();  // update sent code
    
  }

  return sent_code;

}

void control_roll(float roll_target=0.0, float pitch_target=0.0)
{
  float max_point = 15.0;
  float roll_delta = roll_target - roll;
 
  servo_pos_left = limit(roll_delta / max_point);
  servo_pos_right = limit(roll_delta / max_point) * -1.0;
}

bool control_override(int sent_code)
{

  // receiver
  if (sent_code==1) {
    tStartOne = micros();
    Serial.println("setting");
  }

  // override pos based on button press
  if (micros() < (tStartOne + SECOND)) {
    servo_pos_right = servo_max;
    Serial.println("yeah");
    return true;
  }

  return false;
}

void act_servo_prop()
{
  // set servos to positions
  if (servo_last_left != servo_pos_left) {
    servo_last_left = set_servo(servoL, servo_pos_left, true, false);
  }
  if (servo_last_right != servo_pos_right) {
    servo_last_right = set_servo(servoR, servo_pos_right, false, false);
  }

  // set prop
  if (prop_last != prop_pos) {
    prop_last = set_prop(prop, prop_pos, false, false);
  }

}


// SETUP PHASE
void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");

  // servo
  servoR.attach(ServoPinR);
  servoL.attach(ServoPinL);
  set_servo(servoL, servo_pos_left);
  set_servo(servoR, servo_pos_right);

  // prop
  prop.attach(PropPin);
  set_prop(prop, prop_pos);

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

  delay(1000);

  Serial.println("setup complete");
}


// RUN PHASE
void loop() {

  /* inputs */

  input_roll_pitch();
  int sent_code = input_receiver();

  /* controls */

  // decision logic
  if (mode==3) {

    control_roll();

    //control_override(sent_code);

  }

  /* act */

  act_servo_prop();

}




float roll_standardize(float roll)
{
  return roll;
}

float pitch_standardize(float pitch)
{
  return -pitch;
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

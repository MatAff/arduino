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

// servo globals
Servo servoR;
Servo servoL;
int servo_positions[3] = {10, 90, 170};
int servo_position = 90;
int last_servo_position = 0;

// receiver
RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

// IMU globals
double xPos = 0, yPos = 0, headingVel = 0;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; //how often to read data from the board
uint16_t PRINT_DELAY_MS = 500; // how often to print the data
uint16_t printCount = 0; //counter to avoid printing every 10MS sample

//velocity = accel*dt (dt in seconds)
//position = 0.5*accel*dt^2
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
unsigned long tStart = micros();

void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");

  // servo
  servoR.attach(ServoPinR);
  servoL.attach(ServoPinL);
  // set_servo_position(90);

  // receiver
  if (!driver.init())
    Serial.println("init failed - receiver");

  // imu
  if (!bno.begin())
  {
    Serial.print("No BNO055 detected");
    while (1);
  }

  delay(1000);
}


void loop() {

  
  // imu
  
  sensors_event_t orientationData , linearAccelData;

  // wait - imu
  if ((micros() - tStart) > (BNO055_SAMPLERATE_DELAY_MS * 1000))
  {
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    tStart = micros();
    //printEvent(&orientationData);
    double z = orientationData.orientation.z;
    if (z > 10) 
    {
      servo_position = servo_positions[0];
      Serial.println("right");
    }
    if (z < -10) 
    {
      servo_position = servo_positions[2];
      Serial.println("left");
    }
  }
  
  //  bno.getEvent(&angVelData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  //bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);

  // receiver
  uint8_t buf[50];
  uint8_t buflen = sizeof(buf);
  memset(buf, 0, buflen);
  if (driver.recv(buf, &buflen)) {
    Serial.print("Message: ");
    Serial.println((char*)buf);
    int servo_adjust = String((char*)buf).toInt();
    if (servo_adjust >= 1 && servo_adjust <= 3) {
      servo_position = servo_positions[servo_adjust-1];
    }
  }


//  Serial.print("Servo position: ");
//  Serial.println(servo_position);
  if (last_servo_position != servo_position) {
    set_servo_position(servo_position);
  }

  // wait - imu
  /*while ((micros() - tStart) < (BNO055_SAMPLERATE_DELAY_MS * 1000))
  {
    //poll until the next sample is ready
  }*/
}

void set_servo_position(int position) {
  Serial.print("Setting servo position: ");
  Serial.println(position);
  if (position > 0 && position <= 180) {
    servoR.write(position);
    servoL.write(position);
    last_servo_position = position;
  }
  delay(15);
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

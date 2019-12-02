#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <Servo.h>

#define TxPin 10
#define RxPin 10
#define PowerPin 16
#define ServoPin 9

RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);
Servo servo;
int servo_position = 90;
int last_servo_position = 0;

void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");
  
  servo.attach(9);
//  set_servo_position(90);
  if (!driver.init())
    Serial.println("init failed");
}

void loop() {
  uint8_t buf[50];
  uint8_t buflen = sizeof(buf);
  memset(buf, 0, buflen);
  if (driver.recv(buf, &buflen)) {
    Serial.print("Message: ");
    Serial.println((char*)buf);
    int servo_adjust = String((char*)buf).toInt();
    switch (servo_adjust) {
      case 1:
        servo_position -= 5;
        break;
      case 2:
        servo_position += 5;
        break;
      case 3:
        servo_position = 90;
        break;
    }
  }
//  Serial.print("Servo position: ");
//  Serial.println(servo_position);
  if (last_servo_position != servo_position) {
    set_servo_position(servo_position);
  }
}

void set_servo_position(int position) {
  Serial.print("Setting servo position: ");
  Serial.println(position);
  if (position > 0 && position <= 180) {
    servo.write(position);
    last_servo_position = position;
  }
  delay(15);
}

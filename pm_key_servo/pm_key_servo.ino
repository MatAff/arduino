/*
 
*/

#include <Servo.h>

Servo myservo; 

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  Serial.println(pos);
  if (Serial.available() > 0) {
    Serial.println("in");    
    int inInt = Serial.parseInt();
    Serial.println(inInt);    
    if (inInt > 0 && inInt <= 180) {
      pos = inInt;
    }     
  }  
  myservo.write(pos); 
  delay(15); 
}

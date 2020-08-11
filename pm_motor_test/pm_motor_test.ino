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
 * Calibrate and run motor for 2 seconds then stop
 */

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <Servo.h>

#include <Wire.h>

#define PropPin 9

// prop globals
Servo prop;

// function to run through range of values
void set_prop(Servo prop, int start_val, int end_val, int duration_ms) {

  // define step characteristics
  int step_duration_ms = 100;
  int steps_total = int(duration_ms / step_duration_ms);
  float step_size = (end_val - start_val) / steps_total;

  // loop through values
  int val = start_val;
  for (int step_nr = 0; step_nr < steps_total; step_nr++) {
    Serial.println(val);
    prop.writeMicroseconds(val);
    delay(step_duration_ms);
    val = int(val + step_size);
  }
  
}

// SETUP PHASE
void setup()
{
  
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");
  delay(2000);

  // attach pin
  prop.attach(PropPin, 1000, 2000);
  delay(2000);

  // low
  Serial.println("Low");
  set_prop(prop, 1200, 1200, 2000);

  // high
  Serial.println("High");
  set_prop(prop, 1800, 1800, 2000);

  // low
  Serial.println("Low");
  set_prop(prop, 1200, 1200, 2000);

  // middle
  Serial.println("Middle");
  set_prop(prop, 1500, 1500, 2000);

  // low - indefinite
  Serial.println("Low");
  set_prop(prop, 1200, 1200, 999999999);
  delay(4000);

}

// RUN PHASE
void loop() {

}

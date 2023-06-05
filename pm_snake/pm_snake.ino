/* HOW TO RUN
 * add additional boards managers to preferences
 * https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
 * in board manager search for SparkFun and add board manager for pro micro
 * set board
 * set processor
 * set port
 * add https://github.com/SMFSW/Queue
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

#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h>

#define ServoPin1 18
#define ServoPin2 19
#define ServoPin3 20
#define ServoPin4 21

float servoPosGlobal1 = 90.0;
float servoPosGlobal2 = 90.0;
float servoPosGlobal3 = 90.0;
float servoPosGlobal4 = 90.0;

float delayMs = 50.0;

// START SHARED
// UNCOMMENT FOR CPP #include <cmath>
// UNCOMMENT FOR CPP #include <chrono>
// UNCOMMENT FOR CPP #include <iostream>
#include "cppQueue.h" // arduino alternative for deque

#define IMPLEMENTATION FIFO
#define OVERWRITE true
// UNCOMMENT FOR CPP #define PI 3.14159;

typedef struct {
  float x;
  float y;
} Point;

typedef struct {
  float dir;
} Rec;

inline float degToRad(float deg) { return deg / 360 * 2 * PI; }
inline float degSin(float deg) { return sin(degToRad(deg)); }
inline float degCos(float deg) { return cos(degToRad(deg)); }
inline float pointsToDeg(Point start, Point end) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;
  if (dx == 0.0) {
    if (dy > 0) { return 90.0; } else { return 270; }    
  }
  float deg = atan(dy/dx) / 2.0 / PI; 
  deg = deg * 360.0;
  if (dx < 0) { return deg + 180.0; } else { return deg + 360.0; }
}

class Track {
public:
  Track(int n, int pause) {
    pointCount = n;
    this->pause = pause;
    for (int i=0; i < n; i++) {
      points[i] = { 300 + degSin(2.0 * i / pointCount * 360) * 100,  // 2
                    300 + degCos(1.0 * i / pointCount * 360) * 100 };  // 1
    }
  }
  void tick() {
    pos = pos + 1;
    if (pos > pointCount) { pos -= pointCount; }
  }
  float angle() { 
    int startIndex = pos;
    if (startIndex >= pointCount) { startIndex -= pointCount; }
    int endIndex = pos + 1;
    if (endIndex >= pointCount) { endIndex -= pointCount; }
    return pointsToDeg(points[startIndex], points[endIndex]);
  }
  float diff() { 
    float startAngle = angle();
    int startIndex = pos + pause;
    if (startIndex >= pointCount) { startIndex -= pointCount; }
    int endIndex = pos + 1 + pause;
    if (endIndex >= pointCount) { endIndex -= pointCount; }
    float endAngle = pointsToDeg(points[startIndex], points[endIndex]);
    float angle = startAngle - endAngle;
    if (angle < 0) { angle += 360; }
    //// UNCOMMENT FOR CPP std::cout << "pos: " << pos << " startIndex " << startIndex << " endIndex " << endIndex << " startAngle " << startAngle << " endAngle " << endAngle << " angle " << angle << std::endl;
    return angle;
  }
  Point points[72];
  int pos = 0;
  int pointCount;
  int pause;
};

class SnakeControl {
public: 
  int segments = 5;
  float segment_length = 50.0;  // mm
  float delayDur = 50.0;  // ms
  float cycleSpeed = 0.5;  // cycles per second
  float maxWheelAngle = 30.0;  // degrees
  float cycleProportion = 1.0 * 360.0;  // proportion of complete wave
  
  float stepsPerSecond = 1000.0 / delayDur;
  float stepDeg = cycleSpeed / stepsPerSecond * 360;  // degrees
  float wheelStep = cycleProportion / segments;  // phase difference between wheels
  int queuePause = int(wheelStep / stepDeg);  // steps

  float phase = 0;
  float servoPosArr[4] = {90.0, 90.0, 90.0, 90.0};
  float wheelAngles[5] = {-1.0, -1.0, -1.0, -1.0, -1.0};
  float wheelHeight[5];
  cppQueue q = cppQueue(sizeof(Rec), 99, IMPLEMENTATION, OVERWRITE);
  Track track = Track(72, 1);
  SnakeControl() {
    //// UNCOMMENT FOR CPP std::cout << "Constructor" << " ";
    for (int i = 0; i < 300; i++) {
      track.tick();
      float diff = track.diff();
      //// UNCOMMENT FOR CPP std::cout << "diff: " << diff << std::endl;
    }
    //// UNCOMMENT FOR CPP std::exit(0);
  }
  void control_wave() {
    //// UNCOMMENT FOR CPP std::cout << "control wave" << std::endl;
    phase = phase - stepDeg;
    if (phase < 0) { phase = phase + 360; }
    //// UNCOMMENT FOR CPP std::cout << "phase: " << phase << std::endl;
    float start = degSin(phase) * maxWheelAngle;
    //// UNCOMMENT FOR CPP std::cout << "start: " << start << std::endl;
    float enddd = degSin(phase + wheelStep) * maxWheelAngle;
    //// UNCOMMENT FOR CPP std::cout << "enddd: " << enddd << std::endl;
    float angle = enddd - start;
    //// UNCOMMENT FOR CPP std::cout << "angle: " << angle << std::endl;
    track.tick();
    float diff = track.diff();
    //// UNCOMMENT FOR CPP std::cout << "diff: " << diff << std::endl;
    angle = angle + diff;
    angle = angle + 90.0;
    // UNCOMMENT FOR CPP std::cout << "angle the important one: " << angle << std::endl;
    start = start + track.angle();
    
    // wheelAngles[0] = start;
    Rec r = {angle};
    q.push(&angle);
    for (int i = 0; i < segments - 1; i++) {
      //// UNCOMMENT FOR CPP std::cout << "i: " << i << std::endl;
      //// UNCOMMENT FOR CPP std::cout << "queue pos: " << (i * queuePause +1) << std::endl;
      if (q.getCount() > (i * queuePause +1)) {
        //// UNCOMMENT FOR CPP std::cout << "from queue" << std::endl;
        Rec r;
        q.peekIdx(&r, i * queuePause + 1);
        servoPosArr[i] = r.dir;
        if (i == (segments - 2)) { q.pop(&r); }
      } else {
        //// UNCOMMENT FOR CPP std::cout << "from default" << std::endl;
        servoPosArr[i] = 90.0;
      }
    }
  }
};
// END SHARED

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

unsigned long SECOND = 1000000;
unsigned long setTime = micros();
int prestartDurationSecs = 5;
int runDurationSecs = 20;

SnakeControl sc = SnakeControl();

void setup()
{
  Serial.begin(9600);  // Debugging only
  Serial.println("Initializing");
  servo1.attach(ServoPin1);
  servo2.attach(ServoPin2);
  servo3.attach(ServoPin3);
  servo4.attach(ServoPin4);
  Serial.println("setup complete");

  // wait and reset setTime
  while (micros() < (setTime + (prestartDurationSecs * SECOND))) {
    delay(delayMs);
  }
  setTime = micros();
}

void loop() {
  sc.control_wave();
  servoPosGlobal1 = sc.servoPosArr[0];
  servoPosGlobal2 = sc.servoPosArr[1];
  servoPosGlobal3 = sc.servoPosArr[2];
  servoPosGlobal4 = sc.servoPosArr[3];
  act_servos(true);
  delay(delayMs);
  while (micros() > setTime + (runDurationSecs * SECOND)) {
    delay(delayMs);
  }
}

void control_zero()
{
  servoPosGlobal1 = 90.0;
  servoPosGlobal2 = 90.0;
  servoPosGlobal3 = 90.0;
  servoPosGlobal4 = 90.0;
}

void act_servos(bool verbose)
{
  servo1.write(servoPosGlobal1 + 0.0);
  servo2.write(servoPosGlobal2 + 0.0);
  servo3.write(servoPosGlobal3 + 0.0);
  servo4.write(servoPosGlobal4 + 0.0);

  if (verbose)
  {
    String servoStr = "Servo positions: ";
    servoStr += String(servoPosGlobal1, 2) + "; ";
    servoStr += String(servoPosGlobal2, 2) + "; ";
    servoStr += String(servoPosGlobal3, 2) + "; ";
    servoStr += String(servoPosGlobal4, 2) + "; ";
    Serial.println(servoStr);
  }
}

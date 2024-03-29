// START SHARED
#include <cmath>
#include <chrono>
#include <iostream>
#include "cppQueue.h" // arduino alternative for deque

#define IMPLEMENTATION FIFO
#define OVERWRITE true
#define PI 3.14159;

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
    //std::cout << "pos: " << pos << " startIndex " << startIndex << " endIndex " << endIndex << " startAngle " << startAngle << " endAngle " << endAngle << " angle " << angle << std::endl;
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
    //std::cout << "Constructor" << " ";
    for (int i = 0; i < 300; i++) {
      track.tick();
      float diff = track.diff();
      //std::cout << "diff: " << diff << std::endl;
    }
    //std::exit(0);
  }
  void control_wave() {
    //std::cout << "control wave" << std::endl;
    phase = phase - stepDeg;
    if (phase < 0) { phase = phase + 360; }
    //std::cout << "phase: " << phase << std::endl;
    float start = degSin(phase) * maxWheelAngle;
    //std::cout << "start: " << start << std::endl;
    float enddd = degSin(phase + wheelStep) * maxWheelAngle;
    //std::cout << "enddd: " << enddd << std::endl;
    float angle = enddd - start;
    //std::cout << "angle: " << angle << std::endl;
    track.tick();
    float diff = track.diff();
    //std::cout << "diff: " << diff << std::endl;
    angle = angle + diff;
    angle = angle + 90.0;
    std::cout << "angle the important one: " << angle << std::endl;
    start = start + track.angle();
    
    // wheelAngles[0] = start;
    Rec r = {angle};
    q.push(&angle);
    for (int i = 0; i < segments - 1; i++) {
      //std::cout << "i: " << i << std::endl;
      //std::cout << "queue pos: " << (i * queuePause +1) << std::endl;
      if (q.getCount() > (i * queuePause +1)) {
        //std::cout << "from queue" << std::endl;
        Rec r;
        q.peekIdx(&r, i * queuePause + 1);
        servoPosArr[i] = r.dir;
        if (i == (segments - 2)) { q.pop(&r); }
      } else {
        //std::cout << "from default" << std::endl;
        servoPosArr[i] = 90.0;
      }
    }
  }
};
// END SHARED

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
    int pointCount = n;
    this->pause = pause;
    for (int i; i < n; i++) {
      points[i] = { 300 + degSin(1 * i / pointCount * 360) * 100,  // 2
                    300 + degCos(1 * i / pointCount * 360) * 100 };  // 4
    }
  }
  void tick() { pos = pos + 1; }
  float angle() { return pointsToDeg(points[pos], points[pos+1]); }
  float diff() { return angle() - pointsToDeg(points[pos + pause], points[pos + 1 + pause]); }
  Point points[100];
  int pos = 0;
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
  Track track = Track(36, 5);
  SnakeControl() {
    // UNCOMMENT FOR CPP std::cout << "Constructor" << " ";
  }
  void control_wave() {
    // UNCOMMENT FOR CPP std::cout << "control wave" << std::endl;
    phase = phase - stepDeg;
    // UNCOMMENT FOR CPP std::cout << "phase: " << phase << std::endl;
    if (phase < 0) { phase = phase + 360; }
    float start = degSin(phase) * maxWheelAngle;
    // UNCOMMENT FOR CPP std::cout << "start: " << start << std::endl;
    float enddd = degSin(phase + wheelStep) * maxWheelAngle;
    // UNCOMMENT FOR CPP std::cout << "enddd: " << enddd << std::endl;
    float angle = enddd - start;
    // UNCOMMENT FOR CPP std::cout << "angle: " << angle << std::endl;
    track.tick();
    float diff = track.diff();
    // UNCOMMENT FOR CPP std::cout << "diff: " << diff << std::endl;
    angle = angle + diff;
    // UNCOMMENT FOR CPP std::cout << "angle the important one: " << angle << std::endl;
    start = start + track.angle();
    
    wheelAngles[0] = start;
    Rec r = {angle};
    q.push(&angle);
    for (int i = 0; i < segments - 1; i++) {
      // UNCOMMENT FOR CPP std::cout << "i: " << i << std::endl;
      // UNCOMMENT FOR CPP std::cout << "queue pos: " << (i * queuePause +1) << std::endl;
      if (q.getCount() > (i * queuePause +1)) {
        // UNCOMMENT FOR CPP std::cout << "from queue" << std::endl;
        Rec r;
        q.peekIdx(&r, i * queuePause + 1);
        servoPosArr[i] = r.dir;
        if (i == (segments - 2)) { q.pop(&r); }
      } else {
        // UNCOMMENT FOR CPP std::cout << "from default" << std::endl;
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

//class SnakeControl
//{
//
//private:
//
//    typedef struct dirRec {
//      float dir;
//    } Rec;
//
//    // delay
//    float delayMs = 50.0;
//
//    // wave
//    float waveSpeed = 0.5; // nr of complete cycles per second
//    float waveSize = 40.0; // max turning angle
//    float wavePhase = 0.5; // part of complete sine wave
//    int nrServos = 4.0;
//
//    // derive features
//    float nrStepsPerSecond = 1000.0 / delayMs;
//    float waveTimeStep = waveSpeed / nrStepsPerSecond; // phase difference for 1 step
//    float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos
//    float stepsBetweenServos = waveServoStep / waveTimeStep;
//
//    // track pos
//    cppQueue dirQueue = cppQueue(sizeof(Rec), stepsBetweenServos * nrServos, IMPLEMENTATION, OVERWRITE);
//    float servoPhasePosArr[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
//    // servoPhasePosArr[0] = 0.0; // set head value
//
//    // time
//    unsigned long SECOND = 1000000;
//    unsigned long setTime = micros();
//    int prestartDurationSecs = 5;
//    int runDurationSecs = 20;
//    Rec r;
//
//public:
//
//    float servoPosArr[5];
//
//
////    void show_queue(cppQueue& q)
////    {
////        String s = "q: ";
////        for(int i=0; i < q.getCount(); i++)
////        {
////            Rec r;
////            q.peekIdx(&r, i);
////            s += String(r.dir, 2) + "; ";
////        }
////        Serial.println(s);
////    };
//
//// UNCOMMENT FOR CPP long micros() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
//
//    float unit_to_rad(float unit)
//    {
//        return unit * 6.28318531;
//    }
//
//    void control_wave()
//    {
//        // calculate phase
//        r = { servoPhasePosArr[0] };
//        dirQueue.push(&r); // add position record to queque
//
////        show_queue(dirQueue);
//
//        servoPhasePosArr[0] = (servoPhasePosArr[0] + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
//        // if (servoPhasePosArr[0] > 1.0) {
//        //   servoPhasePosArr[0] = servoPhasePosArr[0] - 1.0;
//        // }
//
//        // calculate phase subsequent servos
//        for(int i=1; i < nrServos; i++) // skip head
//        {
//            int qSize = dirQueue.getCount();
////            Serial.println(qSize);
//            if (qSize > (i * stepsBetweenServos))
//            {
//                int pos = qSize - (i * stepsBetweenServos);
////                Serial.println(pos);
//                Rec r;
//                dirQueue.peekIdx(&r, pos);
//                servoPhasePosArr[i] = r.dir;
//
//                if (i == 3)
//                {
////                    Serial.println("popping");
//                    Rec r;
//                    dirQueue.pop(&r); // r is unused, sole purpose of pop is to limit queue size
//                }
//            }
//        }
//
//        // convert to servo position
//        for (int i=0; i < nrServos; i++)
//        {
//            float rad = unit_to_rad(servoPhasePosArr[i]);
//            float deg = sin(rad);
//            // float deg = sin(rad) + sin(rad * 1.3);
////             float deg = sin(rad/3)/2 + sin(rad);
////             float deg = sin(rad)Zssd ef1g2v  + sin(2*rad)/2;
//            // float deg = sin(rad) + sin(3*rad)/3 + sin(5*rad)/5 + sin(7*rad)/7 + sin(9*rad)/9;
//            servoPosArr[i] = deg * waveSize + 90.0;
//        }
//
////        String servoPhaseStr = "Servo phase positions in class: ";
////        servoPhaseStr += String(servoPhasePosArr[0], 2) + "; ";
////        servoPhaseStr += String(servoPhasePosArr[1], 2) + "; ";
////        servoPhaseStr += String(servoPhasePosArr[2], 2) + "; ";
////        servoPhaseStr += String(servoPhasePosArr[3], 2) + "; ";
////        Serial.println(servoPhaseStr);
////
////        String servoStr = "Servo positions in class: ";
////        servoStr += String(servoPosArr[0], 2) + "; ";
////        servoStr += String(servoPosArr[1], 2) + "; ";
////        servoStr += String(servoPosArr[2], 2) + "; ";
////        servoStr += String(servoPosArr[3], 2) + "; ";
////        Serial.println(servoStr);
//
//    }
//
//};

//if __name__ == '__main__':
//
//    track = Track()
//    # print(track.track)
//
//    snake = Snake()
//    # snake.calc_angles()
//    # print(snake.joint_angles)
//
//    cv2.namedWindow('Sim')
//    running = True
//    count = 1
//    while running:
//
//        angles = snake.calc_angles_queue(track)
//
//        frame = np.zeros((480, 640, 3), np.uint8)
//        frame = snake.display(frame)
//        frame = track.display(frame)
//
//        cv2.imshow('Sim', frame)
//        key=  cv2.waitKey(delay)
//
//        if key != -1:
//            print(key)
//        if key == 115:
//            delay = int(delay * 1.2)
//        if key == 102:
//            delay = int(delay / 1.2)
//
//        count = count + 1
//        if count > 500 or key == 27:
//            running = False
//
//    cv2.destroyAllWindows()
//
//df = pd.DataFrame(snake.log)
//
//for col in df.columns:
//    df[col].plot()
//
//df.columns
//df['start'].plot()
//df['end'].plot()
//df['angle'].plot()
//df['wheel_angle'].plot()
//df['joint_angle_0'].plot()

//    def display(self, frame):
//
//        # # Joints only
//        # for i in range(segments -1):
//        #     cv2.circle(frame, (i * 20 + 100, int(snake.joint_angles[i]) + 100), 5, (0, 255, 0), 5)
//
//        # Level lines
//        points = []
//        dir = self.wheel_angles[0]
//        x = 100
//        y = 100 + self.wheel_height[0]
//        points.append([x, y])
//        x = x + cos(dir) * segment_length
//        y = y + sin(dir) * segment_length
//        points.append([x, y])
//        for angle in self.joint_angles:
//            dir = dir + angle
//            x = x + cos(dir) * segment_length
//            y = y + sin(dir) * segment_length
//            points.append([x, y])
//        for start, end in zip(points[:-1], points[1:]):
//            start = [int(e) for e in start]
//            end = [int(e) for e in end]
//            cv2.line(frame, start, end, (0, 255, 0), 2)
//
//        return frame

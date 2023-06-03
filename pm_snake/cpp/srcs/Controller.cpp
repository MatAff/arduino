// START SHARED

#include <cmath>
#include <chrono>

#include "cppQueue.h" // arduino alternative for deque

#define IMPLEMENTATION FIFO
#define OVERWRITE true

// control class that can be shared between arduino and cpp
class SnakeControl
{

private:

    typedef struct dirRec {
      float dir;
    } Rec;

    // delay
    float delayMs = 50.0;

    // wave
    float waveSpeed = 0.5; // nr of complete cycles per second
    float waveSize = 40.0; // max turning angle
    float wavePhase = 0.5; // part of complete sine wave
    int nrServos = 4.0;

    // derive features
    float nrStepsPerSecond = 1000.0 / delayMs;
    float waveTimeStep = waveSpeed / nrStepsPerSecond; // phase difference for 1 step
    float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos
    float stepsBetweenServos = waveServoStep / waveTimeStep;

    // track pos
    cppQueue dirQueue = cppQueue(sizeof(Rec), stepsBetweenServos * nrServos, IMPLEMENTATION, OVERWRITE);
    float servoPhasePosArr[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    // servoPhasePosArr[0] = 0.0; // set head value

    // time
    unsigned long SECOND = 1000000;
    unsigned long setTime = micros();
    int prestartDurationSecs = 5;
    int runDurationSecs = 20;
    Rec r;

public:

    float servoPosArr[5];


//    void show_queue(cppQueue& q)
//    {
//        String s = "q: ";
//        for(int i=0; i < q.getCount(); i++)
//        {
//            Rec r;
//            q.peekIdx(&r, i);
//            s += String(r.dir, 2) + "; ";
//        }
//        Serial.println(s);
//    };


long micros() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

    float unit_to_rad(float unit)
    {
        return unit * 6.28318531;
    }

    void control_wave()
    {
        // calculate phase
        r = { servoPhasePosArr[0] };
        dirQueue.push(&r); // add position record to queque

//        show_queue(dirQueue);

        servoPhasePosArr[0] = (servoPhasePosArr[0] + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
        // if (servoPhasePosArr[0] > 1.0) {
        //   servoPhasePosArr[0] = servoPhasePosArr[0] - 1.0;
        // }

        // calculate phase subsequent servos
        for(int i=1; i < nrServos; i++) // skip head
        {
            int qSize = dirQueue.getCount();
//            Serial.println(qSize);
            if (qSize > (i * stepsBetweenServos))
            {
                int pos = qSize - (i * stepsBetweenServos);
//                Serial.println(pos);
                Rec r;
                dirQueue.peekIdx(&r, pos);
                servoPhasePosArr[i] = r.dir;

                if (i == 3)
                {
//                    Serial.println("popping");
                    Rec r;
                    dirQueue.pop(&r); // r is unused, sole purpose of pop is to limit queue size
                }
            }
        }

        // convert to servo position
        for (int i=0; i < nrServos; i++)
        {
            float rad = unit_to_rad(servoPhasePosArr[i]);
            float deg = sin(rad);
            // float deg = sin(rad) + sin(rad * 1.3);
//             float deg = sin(rad/3)/2 + sin(rad);
//             float deg = sin(rad)Zssd ef1g2v  + sin(2*rad)/2;
            // float deg = sin(rad) + sin(3*rad)/3 + sin(5*rad)/5 + sin(7*rad)/7 + sin(9*rad)/9;
            servoPosArr[i] = deg * waveSize + 90.0;
        }

//        String servoPhaseStr = "Servo phase positions in class: ";
//        servoPhaseStr += String(servoPhasePosArr[0], 2) + "; ";
//        servoPhaseStr += String(servoPhasePosArr[1], 2) + "; ";
//        servoPhaseStr += String(servoPhasePosArr[2], 2) + "; ";
//        servoPhaseStr += String(servoPhasePosArr[3], 2) + "; ";
//        Serial.println(servoPhaseStr);
//
//        String servoStr = "Servo positions in class: ";
//        servoStr += String(servoPosArr[0], 2) + "; ";
//        servoStr += String(servoPosArr[1], 2) + "; ";
//        servoStr += String(servoPosArr[2], 2) + "; ";
//        servoStr += String(servoPosArr[3], 2) + "; ";
//        Serial.println(servoStr);

    }

};

// END SHARED

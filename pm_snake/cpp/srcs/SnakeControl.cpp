
#include <chrono>
#include <iostream>
#include <cmath>
#include <queue>

class SnakeControl
{

private:

    // delay
    float delayMs = 50.0;

    // wave
    float waveSpeed = 0.5; // nr of complete cycles per second
    float waveSize = 30.0; // max turning angle
    float wavePhase = 0.5; // part of complete sine wave
    int nrServos = 4.0;

    // derive features
    float nrStepsPerSecond = 1000.0 / delayMs;
    float waveTimeStep = waveSpeed / nrStepsPerSecond; // phase difference for 1 step
    float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos
    float stepsBetweenServos = waveServoStep / waveTimeStep;

    // track pos
    std::deque<float> dirQueue;
    float servoPhasePosArr[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    // servoPhasePosArr[0] = 0.0; // set head value
  
    // time
    unsigned long SECOND = 1000000; 
    unsigned long setTime = micros();
    int prestartDurationSecs = 5;
    int runDurationSecs = 20;

public:

    float servoPosArr[5];

    // cpp
    long micros()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    float unit_to_rad(float unit)
    {
        return unit * 6.28318531;
    }

    void control_wave() 
    {
        // calculate phase
        dirQueue.push_back(servoPhasePosArr[0]); // add position to queque
        servoPhasePosArr[0] = (servoPhasePosArr[0] + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
        if (servoPhasePosArr[0] > 1.0) {
          servoPhasePosArr[0] = servoPhasePosArr[0] - 1.0;
        }

        // calculate phase subsequent servos
        for(int i=1; i < nrServos; i++) // skip head
        {
            int qSize = dirQueue.size();
            if (qSize > (i * stepsBetweenServos))
            {
                servoPhasePosArr[i] = dirQueue[qSize - (i * stepsBetweenServos)];
                if (i == 3) 
                {
                    dirQueue.pop_front();
                }
            }
        }
        
        // convert to servo position
        for (int i=0; i < nrServos; i++)
        {
            servoPosArr[i]  = sin(unit_to_rad(servoPhasePosArr[i])) * waveSize + 90.0;
        }

    }

};

    //   // debug
    //   if (wordy) {

    //     String phaseStr = "Phase positions: ";
    //     phaseStr += String(servoPhasePos1, 2) + "; ";
    //     phaseStr += String(servoPhasePos2, 2) + "; ";
    //     phaseStr += String(servoPhasePos3, 2) + "; ";
    //     phaseStr += String(servoPhasePos4, 2) + "; ";

    //     String servoStr = "Servo positions: ";
    //     servoStr += String(servoPos1, 2) + "; ";
    //     servoStr += String(servoPos2, 2) + "; ";
    //     servoStr += String(servoPos3, 2) + "; ";
    //     servoStr += String(servoPos4, 2) + "; ";

    // //    Serial.println(phaseStr);
    //     Serial.println(servoStr);
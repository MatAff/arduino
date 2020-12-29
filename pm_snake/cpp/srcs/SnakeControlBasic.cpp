
#include <chrono>
#include <iostream>
#include <cmath>

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
    float waveTimeStep = waveSpeed / nrStepsPerSecond;
    float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos

    float servoPhasePos1 = 0.0;
    float servoPhasePos2 = 0.0;
    float servoPhasePos3 = 0.0;
    float servoPhasePos4 = 0.0;

    float servoPos1 = 90.0;
    float servoPos2 = 90.0;
    float servoPos3 = 90.0;
    float servoPos4 = 90.0;

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
        servoPhasePos1 = (servoPhasePos1 + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
        if (servoPhasePos1 > 1.0) {
          servoPhasePos1 = servoPhasePos1 - 1.0;
        }
        servoPhasePos2 = servoPhasePos1 + waveServoStep;
        servoPhasePos3 = servoPhasePos2 + waveServoStep;
        servoPhasePos4 = servoPhasePos3 + waveServoStep;

        // convert to servo position
        servoPos1 = sin(unit_to_rad(servoPhasePos1)) * waveSize + 90.0;
        servoPos2 = sin(unit_to_rad(servoPhasePos2)) * waveSize + 90.0;
        servoPos3 = sin(unit_to_rad(servoPhasePos3)) * waveSize + 90.0;
        servoPos4 = sin(unit_to_rad(servoPhasePos4)) * waveSize + 90.0;

        // create array and return
        servoPosArr[0] = servoPos1;
        servoPosArr[1] = servoPos2;
        servoPosArr[2] = servoPos3;
        servoPosArr[3] = servoPos4;

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
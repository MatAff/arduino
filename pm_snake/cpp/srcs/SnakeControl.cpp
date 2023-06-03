
// // uses: https://github.com/SMFSW/Queue

// #include <chrono>
// #include <iostream>
// #include <cmath>
// #include <queue>

// #include "cppQueue.h" // arduino alternative for deque

// #define IMPLEMENTATION FIFO

// // control class that can be shared between arduino and cpp
// class SnakeControl
// {

// private:

//     typedef struct dirRec {
//       float dir;
//     } Rec;

//     // delay
//     float delayMs = 50.0;

//     // wave
//     float waveSpeed = 0.5; // nr of complete cycles per second
//     float waveSize = 40.0; // max turning angle
//     float wavePhase = 0.5; // part of complete sine wave
//     int nrServos = 4.0;

//     // derive features
//     float nrStepsPerSecond = 1000.0 / delayMs;
//     float waveTimeStep = waveSpeed / nrStepsPerSecond; // phase difference for 1 step
//     float waveServoStep = wavePhase / nrServos; // phase difference between consecutive servos
//     float stepsBetweenServos = waveServoStep / waveTimeStep;

//     // track pos
//     cppQueue dirQueue = cppQueue(sizeof(Rec), stepsBetweenServos * nrServos, IMPLEMENTATION);
//     float servoPhasePosArr[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
//     // servoPhasePosArr[0] = 0.0; // set head value

//     // time
//     unsigned long SECOND = 1000000;
//     unsigned long setTime = micros();
//     int prestartDurationSecs = 5;
//     int runDurationSecs = 20;
//     Rec r;

// public:

//     float servoPosArr[5];

//     // cpp
//    long micros()
//    {
//        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    }

//     float unit_to_rad(float unit)
//     {
//         return unit * 6.28318531;
//     }

//     void control_wave()
//     {
//         // calculate phase
//         r = { servoPhasePosArr[0] };
//         dirQueue.push(&r); // add position record to queque
//         servoPhasePosArr[0] = (servoPhasePosArr[0] + waveTimeStep); // take remainer to avoid incrementing indefinitely, and improve interpretability
//         // if (servoPhasePosArr[0] > 1.0) {
//         //   servoPhasePosArr[0] = servoPhasePosArr[0] - 1.0;
//         // }

//         // calculate phase subsequent servos
//         for(int i=1; i < nrServos; i++) // skip head
//         {
//             int qSize = dirQueue.getCount();
//             if (qSize > (i * stepsBetweenServos))
//             {
//                 int pos = qSize - (i * stepsBetweenServos);
//                 Rec r;
//                 dirQueue.peekIdx(&r, pos);
//                 servoPhasePosArr[i] = r.dir;

//                 if (i == 3)
//                 {
//                     Rec r;
//                     dirQueue.pop(&r); // r is unused, sole purpose of pop is to limit queue size
//                 }
//             }
//         }

//         // debug
//         std::cout << servoPhasePosArr[0] << " ";
//         std::cout << servoPhasePosArr[1] << " ";
//         std::cout << servoPhasePosArr[2] << " ";
//         std::cout << servoPhasePosArr[3] << " ";
//         std::cout << std::endl;

//         // convert to servo position
//         for (int i=0; i < nrServos; i++)
//         {
//             float rad = unit_to_rad(servoPhasePosArr[i]);
//             // float deg = sin(rad);
//             // float deg = sin(rad) + sin(rad * 1.3);
//             // float deg = sin(rad/3)/2 + sin(rad);
//             float deg = sin(rad) + sin(2*rad)/2;
//             // float deg = sin(rad) + sin(3*rad)/3 + sin(5*rad)/5 + sin(7*rad)/7 + sin(9*rad)/9;
//             servoPosArr[i] = deg * waveSize + 90.0;
//         }

//     }

// };

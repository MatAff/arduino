#include <iostream>
#include <opencv2/opencv.hpp>
// #include <queue>

// #include "cppQueue.h" // arduino alternative for deque
// #include "Line.h"
// #include "Plot.h"
// #include "Segment.h"

// #include "FPS.cpp"
// #include "SnakeControl.cpp" // Drop
#include "Controller.cpp"


#define KEY_ESC 27

cv::Size size(640, 480);
cv::Mat frame(size, CV_8UC3);
// Plot plot = Plot(size);


void plot(cv::Mat& frame, Controller& controller) {


}


int main(int argc, char ** argv)
{
    bool runStatus = true;
    int pauseTime = 5;

    Controller c = Controller();

    // main loop
    while (runStatus == true) {

        plot(frame, c);

        imshow("Live", frame);

        int key = cv::waitKey(pauseTime);
        if (key==KEY_ESC) { runStatus=false; }

        c.next();

    }

}

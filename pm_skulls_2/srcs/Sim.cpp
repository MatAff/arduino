#include <iostream>
#include <opencv2/opencv.hpp>
// #include <queue>

// #include "cppQueue.h" // arduino alternative for deque
// #include "Line.h"
// #include "Plot.h"
// #include "Segment.h"

// #include "FPS.cpp"
#include "Controller.cpp"

#define KEY_ESC 27

cv::Size size(640, 480);
cv::Mat frame(size, CV_8UC3);

void plot(cv::Mat& frame, Element * elements, int size) {
    for (int i = 0; i < size; i++) {
        Element e = elements[i];
        circle(frame, cv::Point(e.x, e.y), 3, cv::Scalar(e.b, e.g, e.r), 3);
    }
}

int main(int argc, char ** argv)
{
    bool runStatus = true;
    Controller controller = Controller();

    // Debug
    for (int i = 0; i < 48; i++) {
        std::cout << i << " " << controller.lightLocator.x[i] << std::endl;
    }
    int light = 24;
    int skull =  int(light / 24);
    int eye = int((light - skull * 24) / 12);
    light = light - skull * 24 - eye * 12;
    std::cout << light << " " << skull << " " << eye << " " << light << " " << std::endl;

    while (runStatus == true) {
        controller.next();

        int size = 48;
        Element elements[size];
        controller.getElements(elements, size);

        plot(frame, elements, size);
        imshow("Live", frame);

        int key = cv::waitKey(5);
        if (key==KEY_ESC) { runStatus=false; }
    }

}

#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>

#include "cppQueue.h" // arduino alternative for deque
#include "Line.h"
#include "Plot.h"
#include "Segment.h"
#include "Controller.cpp"

#include "FPS.cpp"
// #include "SnakeControl.cpp"
// #include "SnakeControlBasic.cpp"

#define KEY_ESC 27
#define KEY_I 105
#define KEY_O 111
#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_PAGE_UP 85
#define KEY_PAGE_DOWN 86
// #define KEY_

cv::Size size(640, 480);
cv::Mat frame(size, CV_8UC3); // TODO: this should be owned by plot
Plot plot = Plot(size);

int nrSegments = 5;
std::vector<Segment> snake; // global

// function to initialize snake
void initSnake(std::vector<Segment>& snake, int nrSegments)
{
    cv::Point lastPos(0.0, 0.0);
    for (int i = 0; i < nrSegments; i++)
    {
        snake.push_back(Segment(lastPos, 0.0));
        lastPos = snake[i].getOtherRotationPos();
    }
}

void setSnake(std::vector<Segment>& snake, int nrSegments, std::vector<float>& servoDegs)
{
    cv::Point thisPos(0.0, 0.0);
    float thisDeg = 0.0;
    for (int i = 0; i < nrSegments; i++)
    {
        snake[i] =Segment(thisPos, thisDeg);
        thisPos = snake[i].getOtherRotationPos();
        thisDeg = snake[i].getDirDeg() + servoDegs[i] - 90.0;
    }
}

// function to draw snake
void drawSnake(std::vector<Segment>& snake, int nrSegments)
{
    for (int i = 0; i < nrSegments; i++)
    {
        plot.drawAt(frame, snake[i].getLines(), snake[i].getPos(), snake[i].getDirDeg());
    }
}

int main(int argc, char ** argv)
{

    bool runStatus = true;
    bool showScreen = true;
    int pauseTime = 30;

    initSnake(snake, nrSegments);
    std::vector<float> servoDegs{ 5.0, 10.0, 15.0, 20.0, 0.0 }; // first or last is ignored
    setSnake(snake, nrSegments, servoDegs);

    // snake control
    SnakeControl sc = SnakeControl();
    // SnakeControl sc = SnakeControlBasic();
    // SnakeControl sc();

    // FPS
    // FPS fps();
    FPS fps = FPS();

    // // queue tries
    // std::deque<float> dirQueue;
    // dirQueue.push(1.0);
    // dirQueue.push(2.0);
    // std::cout << dirQueue.front() << std::endl;
    // dirQueue.pop();
    // std::cout << dirQueue.front() << std::endl;
    // dirQueue.pop();
    // std::cout << dirQueue.front() << std::endl;
    // dirQueue.pop();




    // main loop
    while (runStatus == true) {

        /* INPUT */

        // pass

        /* CONTROL */

        sc.control_wave();
        std::vector<float> servoDegs(std::begin(sc.servoPosArr), std::end(sc.servoPosArr));

        /* ACT */

        setSnake(snake, nrSegments, servoDegs);

        /* DISPLAY */

        // refresh frame
        plot.clear(frame);
        drawSnake(snake, nrSegments);

        // display
        if (showScreen) { imshow("Live", frame); }

        // user input
        int key = cv::waitKey(pauseTime);
        if (key==KEY_ESC) { runStatus=false; }
        if (key==KEY_I) { plot.adjustScale(1/0.85); }
        if (key==KEY_O) { plot.adjustScale(0.85); }
        if (key==KEY_UP || key==KEY_DOWN || key==KEY_LEFT || key==KEY_RIGHT) { plot.adjustPos(key); }
        if (key==KEY_PAGE_UP || key==KEY_PAGE_DOWN) { plot.adjustDir(key); }

        // fps
        fps.get_fps();

        // // debug
        // std::cout << sc.servoPosArr[0] << " ";
        // std::cout << sc.servoPosArr[1] << " ";
        // std::cout << sc.servoPosArr[2] << " ";
        // std::cout << sc.servoPosArr[3] << " ";
        // std::cout << std::endl;

    }

}


        // if (key==118) { exView = !exView; } // Toggle using 'v' (view)
        // if (key==115) { pauseTime = pauseTime * 1.5; } // 's' (Slower)
        // if (key==102) { pauseTime = pauseTime / 1.5;
        //                 if (pauseTime < 5) { pauseTime = 5; }} // 'f' (Faster)
        // if (key==105) { Plot::scaleExView = Plot::scaleExView / 0.85;
        //                 Plot::scaleInView = Plot::scaleInView / 0.85; } // 'i' (zoom In)
        // if (key==111) { Plot::scaleExView = Plot::scaleExView * 0.85;
        //                 Plot::scaleInView = Plot::scaleInView * 0.85; } // 'o' (zoom Out)
        // if (key>=81 && key<=84) { Plot::updateInCenter(key); } // arrow keys (reposition)
        // if (key==101) { robotFrame= !robotFrame; } // 'e' Extra view

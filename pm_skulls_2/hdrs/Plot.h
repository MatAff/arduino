#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cstdarg>

#include <opencv2/opencv.hpp>


#include "Line.h"

class Plot
{
private:
    cv::Size size;
    float scale;
    cv::Point pos;
    float dirDeg;

public:

    Plot(cv::Size size);
    ~Plot();

    // view
    void adjustScale(float adjust);
    void adjustPos(int key);
    void adjustDir(int key);

    // draw
    void clear(cv::Mat& frame);
    void draw(cv::Mat& frame, Line l);
    void draw(cv::Mat& frame, Line l, cv::Scalar color);
    void draw(cv::Mat& frame, std::vector<Line>& lVec);
    void drawAt(cv::Mat& frame, Line l, cv::Point pos, float dirDeg);
    void drawAt(cv::Mat& frame, Line l, cv::Scalar color, cv::Point pos, float dirDeg);
    void drawAt(cv::Mat& frame, std::vector<Line>& lVec, cv::Point pos, float dirDeg);

    // conversion
    cv::Point at(cv::Point p, cv::Point pos, float dirDeg);
    cv::Point toPix(cv::Point p);
    double degToRad(float deg);

};

// #include "Degree.h"


    // double scaleExView;
    // double scaleInView;
    // bool exView;
    // cv::Point pos;
    // double dirDegree;
    // cv::Point inCenter;
    // bool showScreen;


//     // setters
//     static void setSize(cv::Size sizeVal);
//     static void setScaleExView(double scaleVal);
//     static void setScaleInView(double scaleViewVal);
//     static void setExView(bool exViewVal);
//     static void setExView(bool exViewVal, cv::Point posVal, double dirDegreeVal);
//     static void setShowScreen(bool showScreenVal);
//     static void updateInCenter(int key);

//     // clear


//     // to color
//     static void toColor(cv::Mat& frame);

//     // draw


//     static void draw(cv::Mat& frame, Line l, bool isExternal);
//     static void draw(cv::Mat& frame, Line l, cv::Scalar color, bool isExternal);
//     static void draw(cv::Mat& frame, std::vector<Line>& lVec, bool isExternal);
//     static void drawGrid(cv::Mat& frame, int spacing);
//     static void writeFPS(cv::Mat& frame, std::string fpsText);
//     static void writeText(cv::Mat& frame, int lineNr, std::string text);

//     // conversions
//     static cv::Point exToIn(cv::Point p);
//     static cv::Point inToEx(cv::Point p);
//     static cv::Point toPix(cv::Point p, bool isExternal);

//     static double toPixDist(double dist);

//     // Print
//     static void print();

// };

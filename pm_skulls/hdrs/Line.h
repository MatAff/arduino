#pragma once

#include <opencv2/opencv.hpp>

class Line
{
public:

    Line(cv::Point s, cv::Point e);
    Line(cv::Point s, cv::Point e, cv::Scalar colorVal);
    ~Line();

    // getters
    cv::Point getS() { return s; }
    cv::Point getE() { return e; }
    cv::Scalar getColor() { return color; }

private:
    cv::Point s;
    cv::Point e;
    cv::Scalar color;

};


// #include <iostream>
// #include <math.h>
// #include <stdio.h>
// #include <string>



    // // intersect methods
    // double getSlope();
    // double getDx();
    // double getDy();
    // double getIntercept();
    // double length();
    // bool inRange(cv::Point p);
    // cv::Point intersect(Line other);

    // // generic methods
    // std::string toString();
    // void print();


    //enum IntersectResult { PARALLEL, COINCIDENT, NOT_INTERESECTING, INTERESECTING };

    // // tests
    // static void runTests();



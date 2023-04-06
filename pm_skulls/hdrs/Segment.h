#pragma once

#include <string>
#include <stdio.h>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "Line.h"
#include "Degree.h"

class Segment
{

public:

    Segment(cv::Point pos, float dirDeg);
    Segment(float width, float length);
    ~Segment();

    // getters
    cv::Point getPos();
    float getDirDeg();
    std::vector<Line>& getLines();
    cv::Point getOtherRotationPos();


private:

    float width = 60.0;
    float length = 150.0;
    float rotationPoint = 20.0; // x mm from end
    float otherRotatationPoint = -100.0; // x mm from rotate point

    cv::Point pos; 
    float dirDeg;

    std::vector<Line> lines;
    
    void calcLines();
    
};

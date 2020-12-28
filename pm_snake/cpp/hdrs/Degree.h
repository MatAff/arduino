#pragma once

#include <cmath>

#include <opencv2/opencv.hpp>

class Degree
{
public:
   static double degToRad(double deg);
   static double radToDeg(double rad);
   static cv::Point pointDirDist(const cv::Point& p, double dirDeg, double d);
};

   // Degree();
   // ~Degree();


   // static double err(double errSize);
   // static bool prob(double prob); // Probability

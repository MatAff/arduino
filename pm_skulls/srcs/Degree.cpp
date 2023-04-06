#include "Degree.h"

// Degree to radians
double Degree::degToRad(double deg) {
    return deg / 180 * M_PI;
}

// Radians to degrees
double Degree::radToDeg(double rad) {
    return rad / M_PI * 180;
}

// Method to get point based on source point direction and distance
cv::Point Degree::pointDirDist(const cv::Point& p, double dirDeg, double d)
{
    // std::cout << std::endl << p.x << ' ' << dirDeg << ' ' << d << ' ' << p.y << std::endl;
    return cv::Point(p.x + cos(degToRad(dirDeg)) * d, p.y + sin(degToRad(dirDeg)) * d);
}

// // Constructor
// Degree::Degree() { }

// // Destructor
// Degree::~Degree() { }

// // Returns an error up to the size specified in the arguement
// double Degree::err(double errSize)
// {
//     return errSize * 2 * (double(std::rand()) / RAND_MAX - 0.5);
// }

// // Returns true with the probability specified in the argument
// bool Degree::prob(double prob)
// {
//     return std::rand() < (RAND_MAX * prob);
// }

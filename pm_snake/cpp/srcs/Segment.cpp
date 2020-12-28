
#include "Segment.h"

Segment::Segment(cv::Point pos, float dirDeg)
{
    this->pos = pos;
    this->dirDeg = dirDeg;
    calcLines();
}

Segment::Segment(float width, float length)
{
    Segment::width = width;
    Segment::length = length;
    calcLines();
}

Segment::~Segment() {}

cv::Point Segment::getPos()
{
    return pos;
}

float Segment::getDirDeg()
{
    return dirDeg;
}

std::vector<Line>& Segment::getLines()
{
    return lines;
}

void Segment::calcLines() 
{
    cv::Point frontLeft(Segment::rotationPoint, Segment::width / 2.0);
    cv::Point frontRight(Segment::rotationPoint, -Segment::width / 2.0);
    cv::Point backLeft(Segment::rotationPoint - Segment::length, Segment::width / 2.0);
    cv::Point backRight(Segment::rotationPoint - Segment::length, -Segment::width / 2.0);
    lines.push_back(Line(frontLeft, frontRight));
    lines.push_back(Line(frontRight, backRight));
    lines.push_back(Line(backRight, backLeft));
    lines.push_back(Line(backLeft, frontLeft));
}

cv::Point Segment::getOtherRotationPos()
{
    return Degree::pointDirDist(Segment::pos, Segment::dirDeg, Segment::otherRotatationPoint);
}

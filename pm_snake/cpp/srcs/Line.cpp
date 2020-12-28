#include "Line.h"

Line::Line(cv::Point ss, cv::Point ee)
{
    Line::s = ss;
    Line::e = ee;
    Line::color = cv::Scalar(0,255,0);
}

Line::Line(cv::Point ss, cv::Point ee, cv::Scalar colorVal)
{
    Line::s = ss;
    Line::e = ee;
    Line::color = colorVal;
}

Line::~Line()
{
}

// // Slope
// double Line::getSlope()
// {
//     return (double(e.y - s.y) / double(e.x - s.x));
// }

// // dx
// double Line::getDx()
// {
//     return double(e.x - s.x);
// }

// // dy
// double Line::getDy()
// {
//     return double(e.y - s.y);
// }

// // Intersept
// double Line::getIntercept()
// {
//     return s.y - (double(e.y - s.y) / double(e.x - s.x)) * s.x;
// }

// // Compute length
// double Line::length()
// {
//     return sqrt((e.x - s.x) * (e.x - s.x) + (e.y - s.y) * (e.y - s.y));
// }

// // Intersect (returns intersect of two points)
// cv::Point Line::intersect(Line other)
// {
//     double iy;
//     double ix;
//     cv::Point ip;

//     // No intersect point
//     cv::Point np = cv::Point(-999.0,-999.0);

//     // Both dx zero
//     if (other.getDx() == 0 && getDx() ==0) {
//         return np;
//     }

//     // Other dx zero
//     if (other.getDx() == 0) {
//         ix = other.getS().x; // Get x based on other
//         iy = getIntercept() + ix * getSlope(); // Get y based on this
//         ip = cv::Point(ix,iy);
//     }

//     // This dx zero
//     if (getDx() == 0) {
//         ix = getS().x; // Get x based on this
//         iy = other.getIntercept() + ix * other.getSlope(); // Get y based on other
//         ip = cv::Point(ix,iy);
//     }

//     // Neither dx zero
//     if(getDx() != 0 && other.getDx() != 0) {

//         // Check lines are not parallel
// 	      if(getSlope() == other.getSlope()) {
//             return np;
//         }

//         ix = (other.getIntercept() - getIntercept()) / (getSlope() - other.getSlope());
//         iy = getSlope() * ix + getIntercept();
//         ip = cv::Point(ix,iy);
//     }

//     // Check point is in range for both lines)
//     if (inRange(ip) && other.inRange(ip)) {
//         return ip;
//     }

//     return np;
// }

// bool Line::inRange(cv::Point p) {
//     return (((p.x > s.x) == (p.x < e.x)) && ((p.y > s.y) == (p.y < e.y)));
// }

// // To string method
// std::string Line::toString()
// {
//     std::string text = "Line: "; // << s << " " << e << std::endl;
//     return text;
// }

// void Line::print()
// {
//     std::cout << "Line: " <<  s << " " << e << std::endl;
// }





// // Run test cases
// void Line::runTests() {
//     Line a = Line(cv::Point(10,10),cv::Point(20,20));
//     a.print();
//     //std::cout << "toString" << std::endl;
//     //std::cout << a.toString() << std::endl;
//     std::cout << "getS" << std::endl;
//     std::cout << a.getS() << std::endl;
//     std::cout << "getE" << std::endl;
//     std::cout << a.getE() << std::endl;
//     std::cout << "getDx" << std::endl;
//     std::cout << a.getDx() << std::endl;
//     std::cout << "getDy" << std::endl;
//     std::cout << a.getDy() << std::endl;
//     std::cout << "getSlope" << std::endl;
//     std::cout << a.getSlope() << std::endl;
//     std::cout << "getIntercept" << std::endl;
//     std::cout << a.getIntercept() << std::endl;
//     std::cout << "inRange (true/false)" << std::endl;
//     std::cout << a.inRange(cv::Point(15,15)) << std::endl;
//     std::cout << a.inRange(cv::Point(25,25)) << std::endl;
//     Line b = Line(cv::Point(20,10), cv::Point(10,20));
//     b.print();
//     std::cout << "intersect(b)" << std::endl;
//     std::cout << a.intersect(b) << std::endl;
//     Line c = Line(cv::Point(15,10), cv::Point(15,20));
//     std::cout << "intersect(c)" << std::endl;
//     std::cout << a.intersect(c) << std::endl;
//     std::cout << c.intersect(a) << std::endl;

// }

// # https://www.quora.com/How-do-I-get-the-point-of-intersection-of-two-lines-using-a-cross-product-if-I-know-two-points-of-each-line
    // # Intersect last line segment with first line segment
    // def intersect(self, other):
    //     CA = self.points[-2,:] - other.points[0,:]
    //     AB = self.points[-1,:] - self.points[-2,:]
    //     CD = other.points[1,:] - other.points[0,:]
    //     denom = np.cross(CD, AB)
    //     if denom != 0:
    //         s = np.cross(CA,AB) / denom
    //         i  = other.points[0,:] + s * CD
    //         i = i.round(5)
    //         overlap = self.in_range(i) and other.in_range(i)
    //         return(i, True, overlap)
    //     else:
    //         return None, False, False

    // def in_range(self, p):
    //     # p = p.round(5)
    //     return(((self.points[0,:] <= p)==(p <= self.points[1,:])).all())
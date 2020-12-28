#include "Plot.h"

#define KEY_UP 82
#define KEY_DOWN 84
#define KEY_LEFT 81
#define KEY_RIGHT 83
#define KEY_PAGE_UP 85
#define KEY_PAGE_DOWN 86


Plot::Plot(cv::Size size) {
    Plot::size = size;
    Plot::scale = 0.1;
    Plot::pos = cv::Point(0, 0);
    Plot::dirDeg = 0.0;
}

Plot::~Plot() { }

void Plot::adjustScale(float adjust)
{
    Plot::scale *= adjust;
}

void Plot::adjustPos(int key) 
{
    float adjustStep = 20;
    switch(key) {
        case KEY_UP:
            pos.y += adjustStep;
            break;
        case KEY_DOWN:
            pos.y -= adjustStep;
            break;
        case KEY_LEFT:
            pos.x -= adjustStep;
            break;
        case KEY_RIGHT:
            pos.x += adjustStep;
            break;
    }
}

void Plot::adjustDir(int key)
{
    float adjustDeg = 15.0;
    switch(key) {
        case KEY_PAGE_UP:
            dirDeg += adjustDeg;
            break;
        case KEY_PAGE_DOWN:
            dirDeg -= adjustDeg;
            break;
    }
}

void Plot::clear(cv::Mat& frame)
{
     frame = cv::Scalar(0,0,0);
}

void Plot::draw(cv::Mat& frame, Line l)
{
    draw(frame, l, l.getColor());
}

void Plot::draw(cv::Mat& frame, Line l, cv::Scalar color)
{
    cv::line(frame, toPix(l.getS()), toPix(l.getE()), color, 1);
}

void Plot::draw(cv::Mat& frame, std::vector<Line>& lVec)
{
    for(Line l : lVec)
    {
        draw(frame, l);
    }
}

void Plot::drawAt(cv::Mat& frame, Line l, cv::Point pos, float dirDeg) 
{
    drawAt(frame, l, l.getColor(), pos, dirDeg);
}

void Plot::drawAt(cv::Mat& frame, Line l, cv::Scalar color, cv::Point pos, float dirDeg)
{
    cv::line(frame, 
            toPix(Plot::at(l.getS(), pos, dirDeg)), 
            toPix(Plot::at(l.getE(), pos, dirDeg)), 
            color, 1);
}

void Plot::drawAt(cv::Mat& frame, std::vector<Line>& lVec, cv::Point pos, float dirDeg)
{
    for(Line l : lVec)
    {
        drawAt(frame, l, pos, dirDeg);
    }
}

cv::Point Plot::at(cv::Point p, cv::Point pos, float dirDeg)
{
    // rotate
    double rRad = degToRad(dirDeg);
    cv::Point rp(p.x * cos(rRad) - p.y * sin(rRad),
                 p.y * cos(rRad) + p.x * sin(rRad));

    // transpose and return
    return cv::Point(rp.x + pos.x, rp.y + pos.y);
}

cv::Point Plot::toPix(cv::Point p)
{
    // transpose
    cv::Point tp = cv::Point(p.x - Plot::pos.x, p.y - Plot::pos.y);

    // rotate
    double rRad = degToRad(-Plot::dirDeg);
    cv::Point rp(tp.x * cos(rRad) - tp.y * sin(rRad),
                 tp.y * cos(rRad) + tp.x * sin(rRad));

    // scale and return
    return cv::Point(size.width  / 2 + rp.x * Plot::scale,
                     size.height / 2 - rp.y * Plot::scale);
}

double Plot::degToRad(float deg) {
    return deg / 180 * M_PI;
}











// cv::Point Plot::exToIn(cv::Point p)
// {
//     // Transpose
//     cv::Point tp = cv::Point(p.x - pos.x, p.y - pos.y);

//     // Rotate
//     double rRad = Degree::degToRad(-dirDegree);
//     cv::Point rp(tp.x * cos(rRad) - tp.y * sin(rRad),
//                  tp.y * cos(rRad) + tp.x * sin(rRad));

//     return rp;
// }







// void Plot::draw(cv::Mat& frame, std::vector<Line>& lVec, bool isExternal)
// {
//     for(Line l : lVec)
//     {
//         draw(frame, l, isExternal);
//     }
// }



// #include "Degree.h"


// // initialize static variables
// cv::Size Plot::size = cv::Size(640,480);
// double Plot::scaleExView = 0.1;
// double Plot::scaleInView = 0.1;
// bool Plot::exView = false;
// bool Plot::showScreen = true; 

// // internal view
// cv::Point Plot::pos(0,0);
// double Plot::dirDegree = 0;;
// cv::Point Plot::inCenter(size.width * 0.5, size.height * 0.9);


// void Plot::setSize(cv::Size sizeVal)
// {
//      size = sizeVal;
// }

// void Plot::setScaleExView(double scaleVal)
// {
//     scaleExView = scaleVal;
// }

// void Plot::setScaleInView(double scaleVal)
// {
//     scaleInView = scaleVal;
// }

// void Plot::setExView(bool exViewVal)
// {
//     exView = exViewVal;
// }

// void Plot::setExView(bool exViewVal, cv::Point posVal, double dirDegreeVal)
// {
//     exView = exViewVal;
//     pos = posVal;
//     dirDegree = dirDegreeVal;
// }

// void Plot::setShowScreen(bool showScreenVal)
// {
//     showScreen = showScreenVal;
// }

// void Plot::updateInCenter(int key)
// {
//     //std::cout << key << std::endl;
//     switch(key) {
//         case 82:    // key up
//             inCenter.y = inCenter.y + 10;
//             break;
//         case 84:    // key down
//             inCenter.y = inCenter.y - 10;
//             break;
//         case 83:    // key right
//             inCenter.x = inCenter.x - 10;
//             break;
//         case 81:    // key left
//             inCenter.x = inCenter.x + 10;
//             break;
//     }
// }


// void Plot::toColor(cv::Mat& frame)
// {
//     if (frame.channels()<3)
//     {
//         cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
//     }
// }

// // // // DRAW - ALWAYS INTERNAL PERSPECTIVE // // //




// // // // DRAW - PERSPECTIVE DEPENDENT // // //

// void Plot::draw(cv::Mat& frame, Line l, bool isExternal)
// {
//     draw(frame, l, l.getColor(), isExternal);
// }

// void Plot::draw(cv::Mat& frame, Line l, cv::Scalar color, bool isExternal)
// {
//     cv::line(frame, toPix(l.getS(), isExternal), toPix(l.getE(), isExternal), color, 1);
// }


// void Plot::drawGrid(cv::Mat& frame, int spacing)
// {
//     if (!exView) {
//         int gr = 25; // Gray color
//         int nrLines = (size.width / 2) / (spacing * scaleInView);
//         double pixSpacing = spacing * scaleInView;
//         for(int i = -nrLines; i < nrLines; ++i)
//         {
//            // Vertical lines
//             cv::line(frame, cv::Point(size.width / 2 + i * pixSpacing, 0),
//                             cv::Point(size.width / 2 + i * pixSpacing, size.height),
//                             cv::Scalar(gr,gr,gr),1);
//           // Horizontal lines
//            cv::line(frame, cv::Point(0, size.height / 2 + i * pixSpacing),
//                            cv::Point(size.width, size.height / 2 + i * pixSpacing),
//                            cv::Scalar(gr,gr,gr),1);
//         }
//     }
// }

// void Plot::writeFPS(cv::Mat& frame, std::string fpsText)
// {
//   cv::putText(frame,fpsText, cv::Point(frame.size().width - 250,30),
//       cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,255,0),1, 1);
// }

// void Plot::writeText(cv::Mat& frame, int lineNr, std::string text)
// {
//   cv::putText(frame, text, cv::Point(5, lineNr * 30),
//       cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0,255,0),1, 1);
// }


// // CONVERSIONS //

// cv::Point Plot::exToIn(cv::Point p)
// {
//     // Transpose
//     cv::Point tp = cv::Point(p.x - pos.x, p.y - pos.y);

//     // Rotate
//     double rRad = Degree::degToRad(-dirDegree);
//     cv::Point rp(tp.x * cos(rRad) - tp.y * sin(rRad),
//                  tp.y * cos(rRad) + tp.x * sin(rRad));

//     return rp;
// }

// cv::Point Plot::inToEx(cv::Point p)
// {
//     // Rotate
//     double rRad = Degree::degToRad(dirDegree);
//     cv::Point rp(p.x * cos(rRad) - p.y * sin(rRad),
//                  p.y * cos(rRad) + p.x * sin(rRad));

//     // Transpose
//     cv::Point tp = cv::Point(rp.x + pos.x, rp.y + pos.y);

//     return tp;
// }

// cv::Point Plot::toPix(cv::Point p, bool isExternal)
// {
//     if (exView) {  // EXTERNAL VIEW
//         if (isExternal) {
//             return cv::Point(p.x * scaleExView, size.height - p.y * scaleExView);
//         } else {
//             cv::Point exP = inToEx(p);
//             return cv::Point(exP.x * scaleExView, size.height - exP.y * scaleExView);
//         }
//     } else {  // INTERNAL VIEW
//         if (isExternal) {
//             cv::Point inP = exToIn(p);
//             //return cv::Point(size.width  / 2 + inP.x * scaleInView,
//             //                 size.height / 2 - inP.y * scaleInView);
//             return cv::Point(inCenter.x + inP.x * scaleInView,
//                              inCenter.y - inP.y * scaleInView);
//         } else {
//             //return cv::Point(size.width  / 2 + p.x * scaleInView,
//             //                 size.height / 2 - p.y * scaleInView);
//             return cv::Point(inCenter.x + p.x * scaleInView,
//                              inCenter.y - p.y * scaleInView);
//         }
//     }
// }


// double Plot::toPixDist(double dist)
// {
//     if (exView) {  // EXTERNAL VIEW
//         return dist * scaleExView;
//     } else {  // INTERNAL VIEW
//         return dist * scaleInView;
//     }
// }

// void Plot::print()
// {
//   std::cout << "Size: "  << size << "; " <<
//                "exView: " << exView << "; " <<
//                "scaleExView: " << scaleExView << "; " <<
//                "scaleInView: " << scaleInView << "; " <<
//                std::endl;
// }

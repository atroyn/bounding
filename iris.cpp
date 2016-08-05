#include "opencv2/opencv.hpp"
#include "opencv2/features2d.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

bool getTarget(cv::Point &point, std::ifstream &coords) {

    if(!coords.is_open()) {
        return false;
    }

    std::string line;
    if(!std::getline(coords, line)) {
        return false;
    }
    std::istringstream sstream(line);

    double x, y;

    sstream >> x;
    sstream >> y;

    point = cv::Point(x,y);
    return true;
}

int main(int argc, char** argv)
{
    // Assume correctness of args.
    cv::VideoCapture cap(argv[1]);
    if(!cap.isOpened()) {
        return -1;
    }

    std::ifstream coords(argv[2]);
    if(!coords.is_open()) {
        return -1;
    }

    cv::namedWindow("frame",1);
    cv::namedWindow("edges",2);
    cv::Point target;

    while(getTarget(target, coords))
    {
        cv::Mat frame, grayframe, roiframe;
        cap >> frame;

        // Sequence 2 has more centers than frames
        if(frame.empty())
        {
            break;
        }

        cv::cvtColor(frame, grayframe, cv::COLOR_BGR2GRAY);

        // Some magic numbers for now
        cv::GaussianBlur(grayframe, grayframe, cv::Size(3,3), 1.5, 1.5);
        cv::Canny(grayframe, grayframe, 40, 150, 3);

        cv::Rect roi(target.x - 100, target.y - 100, 200, 200);
        roiframe = grayframe(roi);

        cv::Mat nonzero;
        cv::findNonZero(roiframe,nonzero);

        cv::Rect bounding = cv::boundingRect(nonzero);

        cv::rectangle(frame,bounding.tl() + target - cv::Point(100,100), bounding.br() + target - cv::Point(100,100), cv::Scalar(255,0,0),2);

        imshow("frame", frame);
        imshow("edges", roiframe);


        cv::waitKey();
    }
    return 0;
}

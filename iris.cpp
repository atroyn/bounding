#include "opencv2/opencv.hpp"
#include "opencv2/features2d.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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
        cv::Mat frame, grayframe;
        cap >> frame;


        cv::cvtColor(frame, grayframe, cv::COLOR_BGR2GRAY);

        cv::GaussianBlur(grayframe, grayframe, cv::Size(7,7), 1.5, 1.5);
        cv::Canny(grayframe, grayframe, 30, 120, 3);

        cv::circle(frame, target,10,cv::Scalar(0,255,0),2);


        imshow("frame", frame);
        imshow("edges", grayframe);


        cv::waitKey();
    }
    return 0;
}

#include "opencv2/opencv.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv)
{
    // Assume correctness of args etc.
    cv::VideoCapture cap(argv[1]);
    if(!cap.isOpened()) {
        return -1;
    }

    std::ifstream coords(argv[2]);
    if(!coords.is_open()) {
        return -1;
    }

    cv::namedWindow("frame",1);
    for(;;)
    {
        cv::Mat frame;
        cap >> frame;

        std::string line;
        std::getline(coords, line);
        std::istringstream sstream(line);

        double x, y;

        sstream >> x;
        sstream >> y;

        cv::circle(frame, cv::Point(x,y),10,cv::Scalar(0,255,0),2);
        imshow("frame", frame);

        cv::waitKey();
    }
    return 0;
}

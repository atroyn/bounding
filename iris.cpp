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
    cv::Point target;
    cv::Rect bounding;

    bool initialized;
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

        // Extract a region of interest around the target
        cv::Rect roi(target.x - 100, target.y - 100, 200, 200);
        roiframe = grayframe(roi);

        // Apply a gaussian blur
        cv::GaussianBlur(roiframe, roiframe, cv::Size(3,3), 3.0, 3.0);

        // Threshold below the average in the region of interest
        // Ideally: Threshold more closer to the center, rather than uniformly
        cv::Mat thresholded;
        cv::threshold(roiframe, thresholded, cv::mean(roiframe).val[0] - 0.1*cv::mean(roiframe).val[0], 255, cv::THRESH_BINARY_INV);

        // Get the components of the image
        cv::Mat labels, stats, centroids;
        int n_labels = cv::connectedComponentsWithStats(thresholded, labels, stats, centroids);
        int target_component = labels.at<int>(100,100);

        // Extract the target component
        cv::compare(labels, target_component, thresholded, cv::CMP_EQ);

        // Set the bounding box if we have a good segmentation
        // Leveraged assumption: smoothness between frames in scale
        if(target_component != 0) {
            initialized = true;

            int top = stats.at<int>(target_component, cv::CC_STAT_TOP);
            int left = stats.at<int>(target_component, cv::CC_STAT_LEFT);
            int height = stats.at<int>(target_component, cv::CC_STAT_HEIGHT);
            int width = stats.at<int>(target_component, cv::CC_STAT_WIDTH);

            bounding = cv::Rect(left + target.x - 100, top + target.y - 100, width, height);
        }
        if(initialized) {
            cv::rectangle(frame, bounding, cv::Scalar(0,255,0),1);
        }

        imshow("frame", frame);

        cv::waitKey();
    }
    return 0;
}

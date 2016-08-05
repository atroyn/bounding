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
    const std::string input = argv[1];

    cv::VideoCapture cap(input);
    if(!cap.isOpened()) {
        return -1;
    }

    std::ifstream coords(argv[2]);
    if(!coords.is_open()) {
        return -1;
    }

    // Set up video output writing
    cv::Size S = cv::Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),
                  (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    std::string::size_type pAt = input.find_last_of('.');
    const std::string outname = input.substr(0, pAt) + ".out.mp4";
    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),
                  (char)((ex & 0XFF000000) >> 24), 0};

    cv::VideoWriter output;
    output.open(outname, ex, cap.get(CV_CAP_PROP_FPS), S, true);

    if (!output.isOpened())
    {
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
        output << frame;

        cv::waitKey(1);
    }
    return 0;
}

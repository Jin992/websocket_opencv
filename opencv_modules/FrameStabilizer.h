//
// Created by jin on 03.04.19.
//

#ifndef WEBSOCKET_TEST_IMGSTABILIZER_H
#define WEBSOCKET_TEST_IMGSTABILIZER_H

#include <opencv2/opencv.hpp>


class FrameStabilizer {
public:
    FrameStabilizer();
    ~FrameStabilizer();
    void stabialize(cv::Mat &frame);

private:
    cv::Mat curr_;
    cv::Mat curr_gray_;
    cv::Mat prev_;
    cv::Mat prev_gray_;
};


#endif //WEBSOCKET_TEST_IMGSTABILIZER_H

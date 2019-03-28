#include <iostream>
#include <string>
#include <sstream>
#include <client_src/websocket_endpoint.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>

#define DEBUG true

#define FRAME_HEIGHT 480
#define FRAME_WIDTH 640
#define ENCODE_QUALITY 80


namespace {
    void print_capture_device_details(cv::VideoCapture &cap) {
        std::cout << "Capture device info:" << std::endl;
        std::cout << "Status: " << cap.isOpened() <<std::endl;
        std::cout << "Frame width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) <<std::endl;
        std::cout << "Frame height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) <<std::endl;
        std::cout << "FPS:" << cap.get(cv::CAP_PROP_FPS) <<std::endl;
    }

}

/*
 *  argv[1] - capture device id(name, something like "/dev/video0 or 0")
 *  argv[2] - server address and port example ws://localhost:9002
 *  argv[3] - capture framerate (camera fps)
 *
 */
int main(int argc, char **argv) {

    if (argc != 4){
        std::cout << "Wrong arguments quantity" << std::endl;
        return 1;
    }

    websocket_endpoint  endpoint;
    std::vector <uchar> encoded;
    int jpegqual = ENCODE_QUALITY;
    cv::VideoCapture cap(argv[1]);

    cap.set(CV_CAP_PROP_FPS, std::strtol(argv[3], nullptr, 10));

    if (!cap.isOpened()) {
        std::cout << "Can't open video device." << std::endl;
        return 1;
    }
    if (DEBUG) {
        // Print capture device properties
        print_capture_device_details(cap);

        char answer = 'y';
        std::cout << "Continue? [y/n]";
        std::cin >> answer;
        if (answer == 'n') {
            return 1;
        }
    }

    // Connect to server
    int id = endpoint.connect(argv[2]);
    if (id != -1) {
        std::cout << "> Created connection with id " << id << std::endl;
    } else {
        std::cout << "Can't connect to remote endpoint: " << argv[2] << std::endl;
         return 1;
    }

    while (true) {
        cv::Mat send, frame = cv::Mat::zeros(480, 640, CV_8UC3);
        std::vector<int> compression_params;
        std::vector<uchar> array;

        // Capture frame
        cap >> frame;
        // Check for not valid frames
        if (frame.empty()) {
            std::cout << "Frame is empty skipping it..." << std::endl;
            continue;
        }

        // resize initial frame
        cv::resize(frame, send, cv::Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, cv::INTER_LINEAR);
        // add compression flags to vector
        compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
        compression_params.push_back(jpegqual);

        // Compress cv::Mat with .jpg codec and store it to std::vector<uchar>
        imencode(".jpg", send, encoded, compression_params);
        // Store std::vector<uchar> to std::string
        std::string frame_str2(encoded.begin(), encoded.end());

        // Send frame to server
        endpoint.send((void *) frame_str2.data(), frame_str2.size());

        // Some debug info
        if (DEBUG) {
            std::cout << "mat size: " << frame.rows * frame.cols << std::endl;
            std::cout << "mat channels: " << frame.channels() << std::endl;
            std::cout << "vector size: " << encoded.size() << std::endl;
        }
    }
    return 0;
}
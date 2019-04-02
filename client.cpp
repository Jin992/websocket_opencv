#include "h.264codec/x264encoder.h"
#include <opencv2/opencv.hpp>
#include <client_src/websocket_endpoint.h>


struct DataPacked{
    uint32_t frame_id_;
    uint32_t color_size_;
    char     data_[640*480*3];
}__attribute__((packed));


//Pack the NALs created by x264 into a single packet.
unsigned int pack_rgb_data(DataPacked &data, x264Encoder &x264_encoder_){
    unsigned int tmp_size = 0;

    x264_nal_t nal;
    while(x264_encoder_.isNalsAvailableInOutputQueue()){
        nal = x264_encoder_.getNalUnit();
        memcpy(&(data.data_[tmp_size]), nal.p_payload, nal.i_payload);
        tmp_size += nal.i_payload;
    }
    data.color_size_ = tmp_size;
    //Size of DataPacked after data insert
    return sizeof(uint32_t) * 2 + data.color_size_;
}


int main(){

    char *rgb_buffer;
    websocket_endpoint endpoint;
    DataPacked data;
    x264Encoder x264_encoder;
    x264_encoder.initialize(640, 480);

    if (endpoint.connect("ws://localhost:9002") < 0) {
        std::cerr << "Cannot connect to the server." << std::endl;
        return 1;
    }
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open capture device." << std::endl;
        return 1;
    }

    while(true){
        cv::Mat frame;
        cap >> frame;
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        //Any grabber to get raw rgb data from camera.
        rgb_buffer = (char *)frame.data;

        //Encode the frame with x264
        x264_encoder.encodeFrame(rgb_buffer, 640*480*3);
        unsigned int message_size = pack_rgb_data(data, x264_encoder);

        //Send the data to the server
        char *video_frame = new char[message_size];
        memcpy ((void *) video_frame, &data, message_size);
        endpoint.send(video_frame, message_size);
        delete [] video_frame;
    }
}
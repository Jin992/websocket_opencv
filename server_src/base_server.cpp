//
// Created by roda on 27.03.19.
//

#include "base_server.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <h.264codec/x264decoder.h>


extern "C" {
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

struct DataPacked{
    uint32_t frame_id_;
    uint32_t color_size_;
    char     data_[640*480*3];
}__attribute__((packed));

base_server::base_server() : is_connected(false), counter(0) {
    m_server.init_asio();
    m_server.set_open_handler(bind(&base_server::on_open,this,::_1));
    m_server.set_close_handler(bind(&base_server::on_close,this,::_1));
    m_server.set_validate_handler(bind(&base_server::on_validate,this,::_1));
    m_server.set_message_handler(bind(&base_server::on_message,this, ::_1,::_2));
    x264_decoder.initialize(640, 480);
}

void base_server::on_open(connection_hdl hdl) {
    is_connected = true;
}

void base_server::on_close(connection_hdl hdl) {
    is_connected = false;
}

void base_server::on_message(connection_hdl hdl, server::message_ptr msg) {

    if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
        DataPacked data;
        char *bgr_buffer = new char[640*480*3];

        x264_decoder.decodeFrame(&(((DataPacked *)msg->get_payload().data())->data_[0]),
                ((DataPacked *)msg->get_payload().data())->color_size_, bgr_buffer);
        std::cout << "received " << data.color_size_ << std::endl;
        cv::Mat frame(480, 640, CV_8UC3, bgr_buffer);
        cv::imshow("server frame", frame);
        cv::waitKey(10);
        delete [] bgr_buffer;
    }
}

bool base_server::on_validate(connection_hdl hdl) {
    if (is_connected) {
        return false;
    }
    return true;
}

void base_server::run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}
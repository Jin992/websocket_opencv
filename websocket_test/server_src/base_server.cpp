//
// Created by roda on 27.03.19.
//

#include "base_server.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

base_server::base_server() : is_connected(false) {
    m_server.init_asio();
    m_server.set_open_handler(bind(&base_server::on_open,this,::_1));
    m_server.set_close_handler(bind(&base_server::on_close,this,::_1));
    m_server.set_validate_handler(bind(&base_server::on_validate,this,::_1));
    m_server.set_message_handler(bind(&base_server::on_message,this, ::_1,::_2));
}

void base_server::on_open(connection_hdl hdl) {
    is_connected = true;
}

void base_server::on_close(connection_hdl hdl) {
    is_connected = false;
}

void base_server::on_message(connection_hdl hdl, server::message_ptr msg) {
    if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
       cv::Mat rawData = cv::Mat(1, msg->get_payload().size(), CV_8UC1, (char*)msg->get_payload().data());
       cv::Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);

       if (!frame.isContinuous()) {
            frame = frame.clone();
       }
       std::cout << "received " << msg->get_payload().size() << " bytes from client." << std::endl;
       cv::imshow("server-cam", frame);
       cv::waitKey(10);
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
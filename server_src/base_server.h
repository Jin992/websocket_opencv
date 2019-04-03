//
// Created by roda on 27.03.19.
//

#ifndef WEBSOCKET_TEST_BASE_SERVER_H
#define WEBSOCKET_TEST_BASE_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <opencv2/opencv.hpp>
#include <h.264codec/x264decoder.h>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class base_server {
public:
    base_server();

    void run(uint16_t port);
    // Handlers
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    bool on_validate(connection_hdl hdl);

private:
    x264Decoder x264_decoder;
    server m_server;
    bool is_connected;
    int counter;
    int size_of_25;
};

#endif //WEBSOCKET_TEST_BASE_SERVER_H

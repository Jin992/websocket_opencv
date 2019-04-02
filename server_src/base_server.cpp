//
// Created by roda on 27.03.19.
//

#include "base_server.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


extern "C" {
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}



AVFrame cvmat_to_avframe(cv::Mat* frame)
{

    AVFrame dst;
    cv::Size frameSize = frame->size();
    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_RAWVIDEO);
    AVFormatContext* outContainer = avformat_alloc_context();
    AVStream *outStream = avformat_new_stream(outContainer, encoder);
    avcodec_get_context_defaults3(outStream->codec, encoder);

    outStream->codec->pix_fmt = AV_PIX_FMT_BGR24;
    outStream->codec->width = frame->cols;
    outStream->codec->height = frame->rows;
    avpicture_fill((AVPicture*)&dst, frame->data, AV_PIX_FMT_BGR24, outStream->codec->width, outStream->codec->height);
    dst.width = frameSize.width;
    dst.height = frameSize.height;

    return dst;
}


cv::Mat avframe_to_cvmat(AVFrame *frame)
{
    AVFrame dst;
    cv::Mat m;

    memset(&dst, 0, sizeof(dst));

    int w = frame->width, h = frame->height;
    m = cv::Mat(h, w, CV_8UC3);
    dst.data[0] = (uint8_t *)m.data;
    avpicture_fill( (AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);

    struct SwsContext *convert_ctx=NULL;
    enum AVPixelFormat src_pixfmt = AV_PIX_FMT_BGR24;
    enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;
    convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt,
                                 SWS_FAST_BILINEAR, NULL, NULL, NULL);

    sws_scale(convert_ctx, frame->data, frame->linesize, 0, h,
              dst.data, dst.linesize);
    sws_freeContext(convert_ctx);

    return m;
}


base_server::base_server() : is_connected(false), counter(0) {
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

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location, double opacity = 1.0)
{
}




void base_server::on_message(connection_hdl hdl, server::message_ptr msg) {
    /*if (msg->get_opcode() == websocketpp::frame::opcode::binary) {
        cv::Mat cv_frame = avframe_to_cvmat((AVFrame *)msg->get_payload().data());
        cv::imshow("server", cv_frame);
        cv::waitKey(10);
    }*/


    if (msg->get_opcode() == websocketpp::frame::opcode::binary) {

        AVPacket *p = (AVPacket *)msg->get_payload().data();
        std::cout << p->size << std::endl;

      // std::string base64_decode = websocketpp::base64_decode(msg->get_payload());
       /*cv::Mat rawData = cv::Mat(1, msg->get_payload().size(), CV_8UC1, (char*)msg->get_payload().data());
       cv::Mat frame = imdecode(rawData, CV_LOAD_IMAGE_COLOR);

       std::cout << "received " << msg->get_payload().size() << " bytes from client." << std::endl;
       cv::imshow("server-cam", frame );
       cv::waitKey(10);*/
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
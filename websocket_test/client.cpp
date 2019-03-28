//#include <opencv2/core/mat.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <client_src/websocket_endpoint.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <libavcodec/avcodec.h>
//#include <libswscale/swscale.h>
//#include <libavformat/avformat.h>


//void ffmpegToCV(const AVFrame &av_frame, cv::Mat &cv_frame) {
//    cv_frame = cv::Mat(av_frame.height, av_frame.width, CV_8UC3, av_frame.data[0], av_frame.linesize[0]);
//}

int main(int argc, char **argv) {

    if (argc != 3){
        std::cout << "Wrong arguments quantity" << std::endl;
        return 1;
    }
    std::string input;
    websocket_endpoint endpoint;

    cv::VideoCapture cap(argv[1]);
    cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('H', '2', '6', '4'));

    if (!cap.isOpened()) {
        std::cout << "Can't open video device." << std::endl;
        return 1;
    }

    int id = endpoint.connect(argv[2]);
    if (id != -1) {
        std::cout << "> Created connection with id " << id << std::endl;
    } else {
        std::cout << "Can't connect to remote endpoint: " << argv[2] << std::endl;
         return 1;
    }

    while (true) {
       cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);
       std::vector<uchar> array;

       cap >> frame;
       if (frame.empty()) {
           std::cout << "Frame is empty skipping it..." << std::endl;
           continue;
       }
       if (frame.isContinuous()) {
           array.assign((uchar*)frame.datastart, (uchar*)frame.dataend);
       } else {
           for (int i = 0; i < frame.rows; ++i) {
               array.insert(array.end(), frame.ptr<uchar>(i), frame.ptr<uchar>(i)+frame.cols);
           }
       }
       std::cout << "mat size: " << frame.rows * frame.cols << std::endl;
       std::cout << "mat channels: " << frame.channels() << std::endl;
       std::cout << "vector size: " << array.size() << std::endl;

       std::string frame_str(array.begin(), array.end());
       endpoint.send((void*)frame_str.data(), frame_str.size());
       std::cout << "sended " << frame.total() * frame.elemSize() << " bytes to server." << std::endl;
    }
    return 0;
}
/*
#include <opencv2/opencv.hpp>
#include <highgui.h>

extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

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

int main() {
    cv::VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('H', '2', '6', '4'));

    if (!cap.open(0)) {
        return 1;
    }

    while (true) {
        cv::Mat frame;

        cap >> frame;

        imshow("client-cam", frame);

        if (cv::waitKey(10) == 27 )
            break;
    }


}

*/

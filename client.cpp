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
#include <opencv2/core/core.hpp>

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

/*
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

        AVFrame av_frame = cvmat_to_avframe(&frame);
        endpoint.send((void *)av_frame.data[0], av_frame.size);

        */
/*//*
/ resize initial frame
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
        *//*



        // Some debug info
        if (DEBUG) {
            std::cout << "mat size: " << frame.rows * frame.cols << std::endl;
            std::cout << "mat channels: " << frame.channels() << std::endl;
            std::cout << "vector size: " << encoded.size() << std::endl;
        }
        break;
    }
    return 0;
}*/


int encode(AVCodecContext *avctx, AVPacket *pkt, int *got_packet, AVFrame *frame)
{
    int ret = 0;

    *got_packet = 0;

    ret = avcodec_send_frame(avctx, frame);
    if (ret < 0)
        return ret;

    ret = avcodec_receive_packet(avctx, pkt);
    if (!ret)
        *got_packet = 1;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}

int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)
{
    int ret;

    *got_frame = 0;

    if (pkt) {
        ret = avcodec_send_packet(avctx, pkt);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0)
            return ret == AVERROR_EOF ? 0 : ret;
    }

    ret = avcodec_receive_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        return ret;
    if (ret >= 0)
        *got_frame = 1;

    return 0;
}

static void encode12(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
 {
     int ret;

     /* send the frame to the encoder */
     if (frame)
         printf("Send frame %3d\n", frame->pts);
     ret = avcodec_send_frame(enc_ctx, frame);
     if (ret < 0) {
         fprintf(stderr, "Error sending a frame for encoding\n");
         exit(1);
     }

     while (ret >= 0) {
         ret = avcodec_receive_packet(enc_ctx, pkt);
         if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
             return;
         else if (ret < 0) {
             fprintf(stderr, "Error during encoding\n");
             exit(1);
         }

         printf("Write packet %3d (size=%5d)\n", pkt->pts, pkt->size);
     }
 }





int main() {

    cv::VideoCapture cap(0);

    AVCodec *codec;
    AVCodecContext *c= NULL;
    int  got_output;


    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 640;
    c->height = 480;
    /* frames per second */
    c->time_base = (AVRational){1,20};
    c->gop_size = 10; /* emit one intra frame every ten frames */
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(c->priv_data, "preset", "slow", 0);
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    cv::Mat prev_frame;
    while (true) {
        cv::Mat frame;
        cap.read(frame);

        cv::Mat dif = frame - prev_frame;
        imshow("difference", dif);

        // you can also use absdiff
        //absdiff(frame, prev_frame, dif);
        cv::waitKey(10);
        prev_frame = frame.clone();
    }
}

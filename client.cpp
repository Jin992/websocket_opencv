/* Base model
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
#define ENCODE_QUALITY 60
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



*/
/*
 *  argv[1] - capture device id(name, something like "/dev/video0 or 0")
 *  argv[2] - server address and port example ws://localhost:9002
 *  argv[3] - capture framerate (camera fps)
 *
*//*



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
    }*/
/*
    if (DEBUG) {
        // Print capture device properties
        print_capture_device_details(cap);

        char answer = 'y';
        std::cout << "Continue? [y/n]";
        std::cin >> answer;
        if (answer == 'n') {
            return 1;
        }
    }*//*


    // Connect to server
    int id = endpoint.connect(argv[2]);
    if (id != -1) {
        std::cout << "> Created connection with id " << id << std::endl;
    } else {
        std::cout << "Can't connect to remote endpoint: " << argv[2] << std::endl;
         return 1;
    }
    while (true) {
        cv::Mat send, frame;
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
        //std::string base64_encode = websocketpp::base64_encode(frame_str2);

        // Send frame to server
        endpoint.send((void *) encoded.data(), encoded.size());


        // Some debug info
        if (DEBUG) {
            std::cout << "mat size: " << frame.rows * frame.cols << std::endl;
            std::cout << "mat channels: " << frame.channels() << std::endl;
            std::cout << "vector size: " << encoded.size() << std::endl;
        }

    }
    return 0;
}
*/
/*
int main() {

    cv::VideoCapture cap(0);

    AVCodec *codec;
    AVCodecContext *c= NULL;
    int  got_output;


 find the mpeg1 video encoder

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
 put sample parameters

    c->bit_rate = 400000;
 resolution must be a multiple of two

    c->width = 640;
    c->height = 480;
 frames per second

    c->time_base = (AVRational){1,20};
    c->gop_size = 10;
 emit one intra frame every ten frames

    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(c->priv_data, "preset", "slow", 0);
 open it

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
}*/


/* File h264 encoder */
/*
#include <client_src/websocket_endpoint.h>
#include <iostream>
#include <vector>
// FFmpeg
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: cv2ff <outfile>" << std::endl;
        return 1;
    }
    websocket_endpoint  endpoint;
    endpoint.connect("ws://localhost:9002");
    const char* outfile = argv[1];

    // initialize FFmpeg library
    av_register_all();
//  av_log_set_level(AV_LOG_DEBUG);
    int ret;

    const int dst_width = 640;
    const int dst_height = 480;
    const AVRational dst_fps = {30, 1};


    // initialize OpenCV capture as input frame generator
    cv::VideoCapture cvcap(0);
    if (!cvcap.isOpened()) {
        std::cerr << "fail to open cv::VideoCapture";
        return 2;
    }
    cvcap.set(cv::CAP_PROP_FRAME_WIDTH, dst_width);
    cvcap.set(cv::CAP_PROP_FRAME_HEIGHT, dst_height);

    // allocate cv::Mat with extra bytes (required by AVFrame::data)
    std::vector<uint8_t> imgbuf(dst_height * dst_width * 3 + 16);
    cv::Mat image(dst_height, dst_width, CV_8UC3, imgbuf.data(), dst_width * 3);

    // open output format context
    AVFormatContext* outctx = nullptr;
    ret = avformat_alloc_output_context2(&outctx, nullptr, nullptr, outfile);
    if (ret < 0) {
        std::cerr << "fail to avformat_alloc_output_context2(" << outfile << "): ret=" << ret;
        return 2;
    }

    // open output IO context
    ret = avio_open2(&outctx->pb, outfile, AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avio_open2: ret=" << ret;
        return 2;
    }

    // create new video stream
    AVCodec* vcodec = avcodec_find_encoder(outctx->oformat->video_codec);
    AVStream* vstrm = avformat_new_stream(outctx, vcodec);
    if (!vstrm) {
        std::cerr << "fail to avformat_new_stream";
        return 2;
    }
    avcodec_get_context_defaults3(vstrm->codec, vcodec);
    vstrm->codec->width = dst_width;
    vstrm->codec->height = dst_height;
    vstrm->codec->pix_fmt = vcodec->pix_fmts[0];
    vstrm->codec->time_base = vstrm->time_base = av_inv_q(dst_fps);
    vstrm->r_frame_rate = vstrm->avg_frame_rate = dst_fps;
    if (outctx->oformat->flags & AVFMT_GLOBALHEADER)
        vstrm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // open video encoder
    ret = avcodec_open2(vstrm->codec, vcodec, nullptr);
    if (ret < 0) {
        std::cerr << "fail to avcodec_open2: ret=" << ret;
        return 2;
    }

    std::cout
            << "outfile: " << outfile << "\n"
            << "format:  " << outctx->oformat->name << "\n"
            << "vcodec:  " << vcodec->name << "\n"
            << "size:    " << dst_width << 'x' << dst_height << "\n"
            << "fps:     " << av_q2d(dst_fps) << "\n"
            << "pixfmt:  " << av_get_pix_fmt_name(vstrm->codec->pix_fmt) << "\n"
            << std::flush;

    // initialize sample scaler
    SwsContext* swsctx = sws_getCachedContext(nullptr, dst_width, dst_height, AV_PIX_FMT_BGR24,
                                              dst_width, dst_height, vstrm->codec->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsctx) {
        std::cerr << "fail to sws_getCachedContext";
        return 2;
    }

    // allocate frame buffer for encoding
    AVFrame* frame = av_frame_alloc();
    std::vector<uint8_t> framebuf(avpicture_get_size(vstrm->codec->pix_fmt, dst_width, dst_height));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), vstrm->codec->pix_fmt, dst_width, dst_height);
    frame->width = dst_width;
    frame->height = dst_height;
    frame->format = static_cast<int>(vstrm->codec->pix_fmt);

    // encoding loop
    avformat_write_header(outctx, nullptr);
    int64_t frame_pts = 0;
    unsigned nb_frames = 0;
    bool end_of_stream = false;
    int got_pkt = 0;
    do {
        if (!end_of_stream) {
            // retrieve source image
            cvcap >> image;
            cv::imshow("press ESC to exit", image);
            if (cv::waitKey(33) == 0x1b)
                end_of_stream = true;
        }
        if (!end_of_stream) {
            // convert cv::Mat(OpenCV) to AVFrame(FFmpeg)
            const int stride[] = { static_cast<int>(image.step[0]) };
            sws_scale(swsctx, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
            frame->pts = frame_pts++;
        }
        // encode video frame
        AVPacket pkt;
        pkt.data = nullptr;
        pkt.size = 0;
        av_init_packet(&pkt);
        ret = avcodec_encode_video2(vstrm->codec, &pkt, end_of_stream ? nullptr : frame, &got_pkt);
        if (ret < 0) {
            std::cerr << "fail to avcodec_encode_video2: ret=" << ret << "\n";
            break;
        }
        if (got_pkt) {
            // rescale packet timestamp
            pkt.duration = 1;
            av_packet_rescale_ts(&pkt, vstrm->codec->time_base, vstrm->time_base);
            // write packet

            av_write_frame(outctx, &pkt);
            std::cout << "pkt size" << pkt.size << std::endl;
            endpoint.send(pkt.data, pkt.size);
            std::cout << nb_frames << '\r' << std::flush;  // dump progress
            ++nb_frames;
        }
        av_free_packet(&pkt);
    } while (!end_of_stream || got_pkt);
    av_write_trailer(outctx);
    std::cout << nb_frames << " frames encoded" << std::endl;

    av_frame_free(&frame);
    avcodec_close(vstrm->codec);
    avio_close(outctx->pb);
    avformat_free_context(outctx);
    return 0;
}

*/
/*
#include <opencv2/videoio.hpp>
#include <iostream>
#include "client_src/x264encoder.h"
#include <x264.h>
#include <highgui.h>
#include <cv.hpp>

// Working encoder

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <x264.h>
}
*/



/*
void decoding(uint8_t *data, int size)
{

    AVCodecContext* m_pContext;
    AVCodec *codec;
    AVFrame *picture;

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_pContext = avcodec_alloc_context3(codec);
    picture = av_frame_alloc();
    avcodec_get_context_defaults3(m_pContext, codec);
    m_pContext->flags    |= AV_CODEC_FLAG_4MV;
    m_pContext->flags     |=AV_CODEC_FLAG_TRUNCATED;
    m_pContext->flags2  |=  AV_CODEC_FLAG2_CHUNKS;
    m_pContext->pix_fmt   =   AV_PIX_FMT_YUV420P  //It can be RGB also*
    m_pContext->skip_frame = AVDISCARD_DEFAULT;
    m_pContext->error_concealment = 3;
    m_pContext->err_recognition = 1;
    m_pContext->skip_loop_filter = AVDISCARD_DEFAULT;
    m_pContext->workaround_bugs = 1;
    m_pContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_pContext->codec_id = AV_CODEC_ID_H264;

    AVPacket *pkt;
    pkt = new AVPacket();
    int gotpicture;

    av_init_packet(pkt);

    //avcodec_get_frame_defaults(picture);

    pkt->data = data;
    pkt->size = size;
    int outSize = avcodec_decode_video2(m_pContext, picture, &gotpicture,
                                        pkt);
    cv::imshow("video", avframe_to_cvmat(picture));
    cv::waitKey(10);
}




int main() {
    //X264Encoder encoder;
    cv::VideoCapture cap(0);
    AVPicture pic_raw;
    memset((char*)&pic_raw, 0, sizeof(pic_raw));

    if (!cap.isOpened()) {
        std::cerr << "Cannot open webcam" << std::endl;
        return 1;
    }


    /// Initialize encoder params
    x264_param_t param;
    x264_t *encoder;
    x264_picture_t pic_in;
    x264_picture_t pic_out;

    x264_param_default(&param);
    if(x264_param_default_preset(&param, "veryfast", "zerolatency") < 0)
    {
        std::cout <<"Error on preset";
    }
    param.i_threads = 1;
    param.i_width = 640;
    param.i_height = 480;
    param.i_fps_num = (int)cap.get(cv::CAP_PROP_FPS);
    param.i_fps_den = 1;

    // Intra refres:
    param.i_keyint_max = (int)cap.get(cv::CAP_PROP_FPS);
    param.b_intra_refresh = 1;
    //Rate control:
    param.rc.i_rc_method = X264_RC_CRF;        //X264_RC_CRFX264_ANALYSE_BSUB16x16        X264_ANALYSE_BSUB16x16
    param.i_csp = X264_CSP_I420;            //X264_CSP_I420 X264_CSP_YV16
    param.rc.f_rf_constant = 25;//25
    param.rc.f_rf_constant_max = 25;//35
    //For streaming:
    param.b_repeat_headers =1;
    param.b_annexb = 1;                //1

    x264_param_apply_profile(&param, "baseline");
    struct SwsContext* convertCtx;

    encoder = x264_encoder_open(&param);
    x264_encoder_parameters(encoder, &param );
    x264_nal_t *nals;
    int i_nal = 0;
    int i_frame_size = 0;
    int row = 640, col = 480;

    convertCtx =  sws_getContext (640, 480 , AV_PIX_FMT_BGR24, 640,
                                  480 , AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    x264_picture_alloc(&pic_in, X264_CSP_I420, 640, 480);

    while (true) {

        cv::Mat frame;
        cap >> frame;
        avpicture_fill(&pic_raw, &frame.data[0], AV_PIX_FMT_BGR24 ,640, 480);

        sws_scale ( convertCtx, pic_raw.data, pic_raw.linesize, 0, 480, pic_in.img.plane, pic_in.img.i_stride);
        i_frame_size = x264_encoder_encode( encoder, &nals, &i_nal, &pic_in, &pic_out );
        decoding(nals[0].p_payload, i_frame_size);

    }
}
*/

#include <opencv2/opencv.hpp>
#include <client_src/x264encoder.h>
//#include <client_src/x264decoder.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
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

/*void decode_handler(AVFrame* frame, AVPacket* pkt, void* user) {
    cv::imshow("frame", avframe_to_cvmat(frame));
    cv::waitKey(10);
}*/


/*void decode(void *data, int size)
{
    IntPtr pOut = IntPtr.Zero;
    int outLen = 0;

    Marshal.Copy(data, 0, inBuffer, size);

    int gotPicture = 0;

    var rs = FFmpeg.av_parser_parse2(avparser, avcontext, ref pOut, ref outLen, inBuffer, size, 0, 0, 0);
    if (outLen <= 0 || pOut.ToInt32() <= 0)
    {
        //no enough data to construct a frame, return and receive next NAL unit.
        return;
    }
    avpacket.data = pOut;
    avpacket.size = outLen;
    avpacket.flags |= PacketFlags.Key;
    var len = FFmpeg.avcodec_decode_video2(avcontext, avframe, ref gotPicture, ref avpacket);
    Console.WriteLine("avcodec_decode_video2 returned " + len);
    if (gotPicture != 0)
    {
        //some YUV to RGB stuff
    }
}*/


int main() {
    X264Encoder encoder;
    char user[] = "Mike";
    //H264_Decoder decoder(&decode_handler, (void *)user);


    cv::VideoCapture cap(0);

    encoder.in_height =  static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));;
    encoder.in_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    encoder.in_pixel_format = AV_PIX_FMT_BGR24;
    encoder.fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
    encoder.out_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));;
    encoder.out_width =  static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    encoder.out_pixel_format = AV_PIX_FMT_YUV420P;

    encoder.open();

    while (true) {
        cv::Mat frame;
        cap >> frame;
        encoder.encode((char*)frame.data);
        //decoder.readFrame();
    }

}


//
// Created by jin on 02.04.19.
//

#ifndef WEBSOCKET_TEST_X264ENCODER_H
#define WEBSOCKET_TEST_X264ENCODER_H


#include <inttypes.h>
#include <stdio.h>
#include <string>
#include <opencv2/videoio.hpp>

extern "C" {
#  include <x264.h>
#  include <libswscale/swscale.h>
#  include <libavcodec/avcodec.h>
}

class X264Encoder {
public:
    X264Encoder();
    ~X264Encoder();
    bool open();               /* open for encoding */
    bool encode(char* pixels);                                    /* encode the given data */
    bool close();                                                 /* close the encoder and file, frees all memory */
private:
    bool validateSettings();                                      /* validates if all params are set correctly, like width,height, etc.. */
    void setParams();                                             /* sets the x264 params */

public:
    /* params the user should set */
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    int fps;                                                       /* e.g. 25, 60, etc.. */
    AVPixelFormat in_pixel_format;
    AVPixelFormat out_pixel_format;

    /* x264 */
    AVPicture pic_raw;                                            /* used for our "raw" input container */
    x264_picture_t pic_in;
    x264_picture_t pic_out;
    x264_param_t params;
    x264_nal_t* nals;
    x264_t* encoder;
    int num_nals;

    /* input / output */
    int pts;
    struct SwsContext* sws;
};


#endif //WEBSOCKET_TEST_X264ENCODER_H

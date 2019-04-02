//
// Created by jin on 02.04.19.
//

#include "x264encoder.h"
#include <iostream>

X264Encoder::X264Encoder()
        :in_width(0)
        ,in_height(0)
        ,in_pixel_format(AV_PIX_FMT_NONE)
        ,out_width(0)
        ,out_height(0)
        ,out_pixel_format(AV_PIX_FMT_NONE)
        ,fps(25)
        ,encoder(NULL)
        ,sws(NULL)
        ,num_nals(0)
        ,pts(0)
{
    memset((char*)&pic_raw, 0, sizeof(pic_raw));
}

X264Encoder::~X264Encoder() {
    if(sws) {
        close();
    }
}

bool X264Encoder::open() {

    if(!validateSettings()) {
        return false;
    }

    int r = 0;
    int nheader = 0;
    int header_size = 0;

    // @todo add validate which checks if all params are set (in/out width/height, fps,etc..);
    if(encoder) {
        std::cerr << "Already opened. first call close()" << std::endl;
        return false;
    }

    if(out_pixel_format != AV_PIX_FMT_YUV420P) {
        std::cerr << "At this moment the output format must be AV_PIX_FMT_YUV420P" << std::endl;
        return false;
    }

    sws = sws_getContext(in_width, in_height, in_pixel_format,
                         out_width, out_height, out_pixel_format,
                         SWS_FAST_BILINEAR, NULL, NULL, NULL);

    if(!sws) {
        std::cerr << "Cannot create SWS context" << std::endl;
        ::exit(EXIT_FAILURE);
    }


    x264_picture_alloc(&pic_in, X264_CSP_I420, out_width, out_height);

    setParams();

    // create the encoder using our params
    encoder = x264_encoder_open(&params);
    if(!encoder) {
        std::cerr << "Cannot open the encoder" << std::endl;
        close();
        return false;
    }

    // write headers

    r = x264_encoder_headers(encoder, &nals, &nheader);
    if(r < 0) {
        std::cerr << "x264_encoder_headers() failed" << std::endl;
        close();
        return false;
    }

    header_size = nals[0].i_payload + nals[1].i_payload +nals[2].i_payload;
    if(!fwrite(nals[0].p_payload, header_size, 1, stdout)) {
        std::cerr << "Cannot write headers" << std::endl;
        close();
        return false;
    }

    pts = 0;

    return true;
}

bool X264Encoder::encode(char* pixels) {
    if(!sws) {
        std::cerr << "Not initialized, so cannot encode" << std::endl;
        return false;
    }

    // copy the pixels into our "raw input" container.
    int bytes_filled = avpicture_fill(&pic_raw, (uint8_t*)pixels, in_pixel_format, in_width, in_height);
    if(!bytes_filled) {
        std::cerr << "Cannot fill the raw input buffer" << std::endl;
        return false;
    }

    // convert to I420 for x264
    int h = sws_scale(sws, pic_raw.data, pic_raw.linesize, 0,
                      in_height, pic_in.img.plane, pic_in.img.i_stride);

    if(h != out_height) {
        std::cerr << "scale failed: " <<  h << std::endl;
        return false;
    }

    // and encode and store into pic_out
    pic_in.i_pts = pts;

    int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, &pic_in, &pic_out);
    // write size to nul
    
    nals[0].p_payload[0] = (frame_size>> 24) & 0xFF;
    nals[0].p_payload[1] = (frame_size >> 16) & 0xFF;
    nals[0].p_payload[2] = (frame_size >> 8) & 0xFF;
    nals[0].p_payload[3] = frame_size & 0xFF;

    //std::cout << std::string((char *)nals[0].p_payload, 5) << std::endl;
    printf("[%d] %d %d %d %d %d %d %d %d %d %d\n", frame_size, nals[0].p_payload[0], nals[0].p_payload[1], nals[0].p_payload[2],
            nals[0].p_payload[3], nals[0].p_payload[4], nals[0].p_payload[5], nals[0].p_payload[6], nals[0].p_payload[7], nals[0].p_payload[8], nals[0].p_payload[9]);
    /*if(frame_size) {
        if(!fwrite(nals[0].p_payload, frame_size, 1, stdout)) {
            std::cerr << "Error while trying to write nal" << std::endl;
            return false;
        }
    }*/
    ++pts;

    return true;
}

bool X264Encoder::close() {
    if(encoder) {
        x264_picture_clean(&pic_in);
        memset((char*)&pic_in, 0, sizeof(pic_in));
        memset((char*)&pic_out, 0, sizeof(pic_out));

        x264_encoder_close(encoder);
        encoder = NULL;
    }

    if(sws) {
        sws_freeContext(sws);
        sws = NULL;
    }

    memset((char*)&pic_raw, 0, sizeof(pic_raw));

    return true;
}

void X264Encoder::setParams() {
    x264_param_default_preset(&params, "ultrafast", "zerolatency");
    params.i_threads = 1;
    params.i_width = out_width;
    params.i_height = out_height;
    params.i_fps_num = fps;
    params.i_fps_den = 1;
}

bool X264Encoder::validateSettings() {
    if(!in_width) {
        std::cerr << "No in_width set" << std::endl;
        return false;
    }
    if(!in_height) {
        std::cerr << "No in_height set" << std::endl;
        return false;
    }
    if(!out_width) {
        std::cerr << "No out_width set" << std::endl;
        return false;
    }
    if(!out_height) {
        std::cerr << "No out_height set" << std::endl;
        return false;
    }
    if(in_pixel_format == AV_PIX_FMT_NONE) {
        std::cerr << "No in_pixel_format set" << std::endl;
        return false;
    }
    if(out_pixel_format == AV_PIX_FMT_NONE) {
        std::cerr << "No out_pixel_format set" << std::endl;
        return false;
    }
    return true;
}
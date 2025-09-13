#ifndef ROHIER_POINTER_MANAGEMENT_H
#define ROHIER_POINTER_MANAGEMENT_H

#include "multimedia/player_framework/native_avcodec_videodecoder.h"
#include "multimedia/player_framework/native_avformat.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libavcodec/packet.h"
#include "libswscale/swscale.h"
}

struct AVFormatContextReleaser { 
    void operator()(AVFormatContext *ptr) const {
        if (ptr) {
            if (!(ptr->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&ptr->pb);
            }
            avformat_free_context(ptr);            
        }
    }
};

struct AVCodecContextReleaser {
    void operator() (AVCodecContext *ptr) const {
        if (ptr) {
            avcodec_free_context(&ptr);
        }
    }
};

struct AVFrameReleaser {
    void operator() (AVFrame *ptr) const { 
        if (ptr) {
            av_frame_free(&ptr);
        }
    }
};

struct AVPacketReleaser {
    void operator() (AVPacket *ptr) const { 
        if (ptr) {
            av_packet_free(&ptr);
        }
    }
};

struct OH_AVFormatReleaser {
    void operator() (OH_AVFormat *ptr) const { 
        if (ptr) {
            OH_AVFormat_Destroy(ptr);
        }
    }
};

struct SwsContextReleaser {
    void operator() (SwsContext* sws) const {
        if (sws) {
            sws_freeContext(sws);
        }
    }
};

struct OH_VideoDecoderReleaser {
    void operator() (OH_AVCodec *ptr) const { 
        if (ptr) {
            OH_VideoDecoder_Destroy(ptr);
        }
    }
};

#endif //ROHIER_POINTER_MANAGEMENT_H

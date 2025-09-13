#ifndef ROHIER_POINTER_MANAGEMENT_H
#define ROHIER_POINTER_MANAGEMENT_H

#include "multimedia/player_framework/native_avcodec_videodecoder.h"
#include "multimedia/player_framework/native_avformat.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

struct AVCodecContextReleaser {
  void operator()(AVCodecContext *ptr) const { avcodec_free_context(&ptr); }
};

struct AVFrameReleaser {
  void operator()(AVFrame *ptr) const { av_frame_free(&ptr); }
};

struct OH_AVFormatReleaser {
  void operator()(OH_AVFormat *ptr) const { OH_AVFormat_Destroy(ptr); }
};

struct OH_VideoDecoderReleaser {
  void operator()(OH_AVCodec *ptr) const { OH_VideoDecoder_Destroy(ptr); }
};

#endif //ROHIER_POINTER_MANAGEMENT_H

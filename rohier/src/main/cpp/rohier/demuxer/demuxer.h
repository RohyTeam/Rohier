//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_DEMUXER_H
#define ROHIER_DEMUXER_H

#include "rohier/common/status.h"
#include "rohier/decoder/buffer.h"
#include "rohier/metadata/video_metadata.h"
#include <multimedia/player_framework/native_avsource.h>

extern "C" {
#include "libavformat/avformat.h"
}

class Demuxer {
public:
    virtual ~Demuxer() {};
    virtual RohierStatus prepare(AVFormatContext* context, OH_AVSource* source, VideoMetadata &metadata) = 0;
    virtual RohierStatus read_sample(int32_t trackId, CodecBuffer &buffer) = 0;
    virtual RohierStatus seek(int64_t position) = 0;
    virtual RohierStatus release() = 0;
};

#endif // ROHIER_DEMUXER_H

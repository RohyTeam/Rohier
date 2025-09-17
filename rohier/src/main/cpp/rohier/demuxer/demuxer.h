//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_DEMUXER_H
#define ROHIER_DEMUXER_H

#include "libavformat/avformat.h"
#include "rohier/common/status.h"
#include "rohier/metadata/video_metadata.h"
#include <multimedia/player_framework/native_avsource.h>

class Demuxer {
public:
    Demuxer();
    virtual ~Demuxer() {};
    virtual RohierStatus create(AVFormatContext* context, OH_AVSource* source, VideoMetadata &metadata) = 0;
    virtual RohierStatus read_sample(int32_t trackId, OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr) = 0;
    virtual RohierStatus release() = 0;
    virtual int32_t get_video_track_id() = 0;
    virtual int32_t get_audio_track_id() = 0;
};

#endif //ROHIER_DEMUXER_H

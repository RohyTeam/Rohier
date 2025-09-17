//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_OHCODEC_DEMUXER_H
#define ROHIER_OHCODEC_DEMUXER_H

#include <multimedia/player_framework/native_avbuffer.h>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>

#include "rohier/demuxer/demuxer.h"

class OHCodecDemuxer : public Demuxer {
public:
    OHCodecDemuxer();
    ~OHCodecDemuxer() override;
    RohierStatus create(AVFormatContext* context, OH_AVSource* source, VideoMetadata &metadata) override;
    RohierStatus read_sample(int32_t trackId, OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr) override;
    RohierStatus release() override;
    int32_t get_video_track_id() override;
    int32_t get_audio_track_id() override;
    
private:
    OH_AVSource *source_;
    OH_AVDemuxer *demuxer_;
    int32_t video_track_id_;
    int32_t audio_track_id_;
};

#endif //ROHIER_OHCODEC_DEMUXER_H

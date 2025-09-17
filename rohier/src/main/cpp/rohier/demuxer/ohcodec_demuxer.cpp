//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "ohcodec_demuxer.h"
#include <multimedia/player_framework/native_averrors.h>

OHCodecDemuxer::OHCodecDemuxer() { 
}

OHCodecDemuxer::~OHCodecDemuxer() { 
    this->release(); 
}

RohierStatus OHCodecDemuxer::create(AVFormatContext* context, OH_AVSource* source, VideoMetadata &metadata) {
    if (!source)
        return RohierStatus::RohierStatus_SourceNotAccessible;
    this->demuxer_ = OH_AVDemuxer_CreateWithSource(source);
    if (!this->demuxer_)
        return RohierStatus::RohierStatus_FailedToCreateDemuxer;
    this->source_ = source;

    auto sourceFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetSourceFormat(source_), OH_AVFormat_Destroy);
    
    if (!sourceFormat)
        return RohierStatus::RohierStatus_FailedToGetSourceFormat;

    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecDemuxer::read_sample(int32_t trackId, OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr) {
    if (!this->demuxer_)
        return RohierStatus::RohierStatus_DemuxerNotFound;
    if (OH_AVDemuxer_ReadSampleBuffer(demuxer_, trackId, buffer) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToReadSampleFromDemuxer;
    if (OH_AVBuffer_GetBufferAttr(buffer, &attr) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToGetBufferAttrFromDemuxer;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecDemuxer::release() {
    if (demuxer_ != nullptr) {
        OH_AVDemuxer_Destroy(demuxer_);
        demuxer_ = nullptr;
    }
    return RohierStatus::RohierStatus_Success;
}

int32_t OHCodecDemuxer::get_video_track_id() { return this->video_track_id_; }
int32_t OHCodecDemuxer::get_audio_track_id() { return this->audio_track_id_; }
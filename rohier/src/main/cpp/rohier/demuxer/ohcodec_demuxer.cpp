//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "ohcodec_demuxer.h"
#include "rohier/utils/rohier_logger.h"
#include "rohier/utils/safe_utils.h"
#include <cstdint>
#include <multimedia/player_framework/native_averrors.h>

OHCodecDemuxer::OHCodecDemuxer() { 
}

OHCodecDemuxer::~OHCodecDemuxer() { 
    this->release(); 
}

RohierStatus OHCodecDemuxer::prepare(AVFormatContext* context, OH_AVSource* source, VideoMetadata &metadata) {
    ROHIER_INFO("OHCodecDemuxer", "Preparing OHCodec demuxer");
    if (!source) {
        ROHIER_ERROR("OHCodecDemuxer", "OH_AVSource not found");
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
    this->demuxer_ = OH_AVDemuxer_CreateWithSource(source);
    if (!this->demuxer_) {
        ROHIER_ERROR("OHCodecDemuxer", "Failed to create OHCodec demuxer");
        return RohierStatus::RohierStatus_FailedToCreateDemuxer;
    }
    this->source_ = source;

    auto sourceFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetSourceFormat(source_), OH_AVFormat_Destroy);

    if (!sourceFormat) {
        ROHIER_ERROR("OHCodecDemuxer", "Failed to get OHCodec source format");
        return RohierStatus::RohierStatus_FailedToGetSourceFormat;
    }
    for (auto track : metadata.tracks) {
        uint32_t track_index = safe_convert_int32_to_uint32(track.index);
        if (track.track_type == TrackType::TrackType_Video) {
            ROHIER_INFO("OHCodecDemuxer", "Selected track %{public}ud", track_index);
            OH_AVDemuxer_SelectTrackByID(this->demuxer_, track_index);
        } else if (track.track_type == TrackType::TrackType_Audio) {
            ROHIER_INFO("OHCodecDemuxer", "Selected track %{public}ud", track_index);
            OH_AVDemuxer_SelectTrackByID(this->demuxer_, track_index);
        }
    }

    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecDemuxer::read_sample(int32_t trackId, CodecBuffer &buffer) {
    ROHIER_INFO("OHCodecDemuxer", "Reading sample for track %{public}d", trackId);
    if (!this->demuxer_)
        return RohierStatus::RohierStatus_DemuxerNotFound;
    OH_AVBuffer* oh_buffer = reinterpret_cast<OH_AVBuffer*>(buffer.buffer);
    if (OH_AVDemuxer_ReadSampleBuffer(demuxer_, trackId, oh_buffer) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToReadSampleFromDemuxer;
    if (OH_AVBuffer_GetBufferAttr(oh_buffer, &buffer.attr) != OH_AVErrCode::AV_ERR_OK)
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
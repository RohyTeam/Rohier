//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_TRACK_METADATA_H
#define ROHIER_TRACK_METADATA_H

#include <cstdint>
#include <multimedia/native_audio_channel_layout.h>
#include <string>

extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
}

enum TrackType {
    TrackType_Unknown = -1,
    TrackType_Video = 0,
    TrackType_Audio = 1,
    TrackType_Subtitle = 2,
    TrackType_Attachment = 3
};

enum HdrType {
    HdrType_None = 0,
    HdrType_Dolby_Vision = 1,
    HdrType_HDRVivid = 2,
    HdrType_HDR10_Plus = 3,
    HdrType_HDR10_or_HLG = 4
};

struct TrackMetadata {
    int index;
    std::string title;
    std::string codec;
    std::string codec_full;
    std::string language;
    
    int width;
    int height;
    int64_t duration;
    int bitrate; // video and audio
    
    // video
    HdrType hdr;
    
    // audio
    int sample_rate;
    int channels;
    
    // attachment
    std::string filename;
    std::string mimetype;
    
    TrackType track_type = TrackType::TrackType_Unknown;
};

#endif //ROHIER_TRACK_METADATA_H

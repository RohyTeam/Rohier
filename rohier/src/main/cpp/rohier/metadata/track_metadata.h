//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_TRACK_METADATA_H
#define ROHIER_TRACK_METADATA_H

#include <cstdint>
#include <string>

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
    int index = -1;
    std::string title;
    std::string codec;
    std::string codec_full;
    std::string language;
    
    int width = 0;
    int height = 0;
    int64_t duration = 0;
    
    // video
    double averageFrameRate = -1;
    int bitrate = 0;
    HdrType hdr = HdrType::HdrType_None;
    
    // audio
    int samplerate = 0;
    
    // attachment
    std::string filename;
    std::string mimetype;
    
    TrackType track_type = TrackType::TrackType_Unknown;
};

#endif //ROHIER_TRACK_METADATA_H

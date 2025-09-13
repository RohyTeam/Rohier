//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_VIDEO_METADATA_H
#define ROHIER_VIDEO_METADATA_H

#include "rohier/metadata/chapter_metadata.h"
#include "rohier/metadata/track_metadata.h"
#include <vector>

struct VideoMetadata {
    int64_t duration = 0;
    int bitrate = 0;
    int width = 0;
    int height = 0;
    double averageFrameRate = -1;
    HdrType hdr = HdrType::HdrType_None;
    std::string codec;
    std::string codec_full;
    
    std::vector<ChapterMetadata> chapters;
    std::vector<TrackMetadata> tracks;
    
    std::vector<uint8_t> cover; // could be nullptr
};

#endif // ROHIER_VIDEO_METADATA_H

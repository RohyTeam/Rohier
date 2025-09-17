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

extern "C" {
#include "libavutil/rational.h"
}

struct VideoMetadata {
    int64_t duration;
    int bitrate;
    int width;
    int height;
    double averageFrameRate;
    HdrType hdr;
    std::string codec;
    std::string codec_full;
    
    std::vector<ChapterMetadata> chapters;
    std::vector<TrackMetadata> tracks;
    
    std::vector<uint8_t> cover; // could be nullptr
};

int64_t time_base_to_ms(AVRational time_base, int64_t value);

#endif // ROHIER_VIDEO_METADATA_H

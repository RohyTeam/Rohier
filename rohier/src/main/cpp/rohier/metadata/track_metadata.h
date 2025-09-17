//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_TRACK_METADATA_H
#define ROHIER_TRACK_METADATA_H

#include "libavutil/channel_layout.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
#include <cstdint>
#include <multimedia/native_audio_channel_layout.h>
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

enum ChannelLayout {
    ChannelLayout_Unknown = -1,
    ChannelLayout_Mono = 0,
    ChannelLayout_Stereo = 1,
    ChannelLayout_2Point1 = 2,
    ChannelLayout_Surround = 3,
    ChannelLayout_3Point1 = 4,
    ChannelLayout_4Point0 = 5,
    ChannelLayout_Quad = 6,
    ChannelLayout_5Point0 = 7,
    ChannelLayout_5Point0_Back = 8,
    ChannelLayout_5Point1 = 9,
    ChannelLayout_5Point1_Back = 10,
    ChannelLayout_6Point0 = 11,
    ChannelLayout_3Point1Point2 = 12,
    ChannelLayout_6Point0_Front = 13,
    ChannelLayout_Hexagonal = 14,
    ChannelLayout_6Point1 = 15,
    ChannelLayout_6Point1_Back = 16,
    ChannelLayout_6Point1_Front = 17,
    ChannelLayout_7Point0 = 18,
    ChannelLayout_7Point0_Front = 19,
    ChannelLayout_7Point1 = 20,
    ChannelLayout_7Point1_Wide = 21,
    ChannelLayout_7Point1_Wide_Back = 22,
    ChannelLayout_Octagonal = 23,
    ChannelLayout_5Point1Point2 = 24,
    ChannelLayout_7Point1Point2 = 25,
    ChannelLayout_9Point1Point6 = 26,
    ChannelLayout_Hexadecagonal = 27,
    ChannelLayout_22Point2 = 28,
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
    double averageFrameRate;
    HdrType hdr;
    AVPixelFormat pixel_format;
    
    // audio
    int sample_rate;
    int channels;
    ChannelLayout channel_layout;
    AVSampleFormat sample_format;
    
    // attachment
    std::string filename;
    std::string mimetype;
    
    TrackType track_type = TrackType::TrackType_Unknown;
};

ChannelLayout ChannelLayout_OHCodec2Rohier(OH_AudioChannelLayout channelLayout);
ChannelLayout ChannelLayout_FFmpeg2Rohier(AVChannelLayout* channelLayout);

OH_AudioChannelLayout ChannelLayout_Rohier2OHCodec(ChannelLayout channelLayout);
AVChannelLayout* ChannelLayout_Rohier2FFmpeg(ChannelLayout channelLayout);

#endif //ROHIER_TRACK_METADATA_H

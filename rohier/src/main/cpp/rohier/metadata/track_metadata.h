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

ChannelLayout ChannelLayout_OHCodec2Rohier(OH_AudioChannelLayout channelLayout) {
    switch (channelLayout) {
        case OH_AudioChannelLayout::CH_LAYOUT_MONO:
            return ChannelLayout::ChannelLayout_Mono;
        case OH_AudioChannelLayout::CH_LAYOUT_STEREO:
            return ChannelLayout::ChannelLayout_Stereo;
        case OH_AudioChannelLayout::CH_LAYOUT_2POINT1:
            return ChannelLayout::ChannelLayout_2Point1;
        case OH_AudioChannelLayout::CH_LAYOUT_SURROUND:
            return ChannelLayout::ChannelLayout_Surround;
        case OH_AudioChannelLayout::CH_LAYOUT_3POINT1:
            return ChannelLayout::ChannelLayout_3Point1;
        case OH_AudioChannelLayout::CH_LAYOUT_4POINT0:
            return ChannelLayout::ChannelLayout_4Point0;
        case OH_AudioChannelLayout::CH_LAYOUT_QUAD:
            return ChannelLayout::ChannelLayout_Quad;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT0:
            return ChannelLayout::ChannelLayout_5Point0;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT0_BACK:
            return ChannelLayout::ChannelLayout_5Point0_Back;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT1:
            return ChannelLayout::ChannelLayout_5Point1;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT1_BACK:
            return ChannelLayout::ChannelLayout_5Point1_Back;
        case OH_AudioChannelLayout::CH_LAYOUT_6POINT0:
            return ChannelLayout::ChannelLayout_6Point0;
        case OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2:
            return ChannelLayout::ChannelLayout_3Point1Point2;
        case OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT:
            return ChannelLayout::ChannelLayout_6Point0_Front;
        case OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL:
            return ChannelLayout::ChannelLayout_Hexagonal;
        case OH_AudioChannelLayout::CH_LAYOUT_6POINT1:
            return ChannelLayout::ChannelLayout_6Point1;
        case OH_AudioChannelLayout::CH_LAYOUT_6POINT1_BACK:
            return ChannelLayout::ChannelLayout_6Point1_Back;
        case OH_AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT:
            return ChannelLayout::ChannelLayout_6Point1_Front;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT0:
            return ChannelLayout::ChannelLayout_7Point0;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT:
            return ChannelLayout::ChannelLayout_7Point0_Front;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1:
            return ChannelLayout::ChannelLayout_7Point1;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE:
            return ChannelLayout::ChannelLayout_7Point1_Wide;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK:
            return ChannelLayout::ChannelLayout_7Point1_Wide_Back;
        case OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL:
            return ChannelLayout::ChannelLayout_Octagonal;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT1POINT2:
            return ChannelLayout::ChannelLayout_5Point1Point2;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT2:
            return ChannelLayout::ChannelLayout_7Point1Point2;
        case OH_AudioChannelLayout::CH_LAYOUT_9POINT1POINT6:
            return ChannelLayout::ChannelLayout_9Point1Point6;
        case OH_AudioChannelLayout::CH_LAYOUT_HEXADECAGONAL:
            return ChannelLayout::ChannelLayout_Hexadecagonal;
        case OH_AudioChannelLayout::CH_LAYOUT_22POINT2:
            return ChannelLayout::ChannelLayout_22Point2;
        default:
            return ChannelLayout::ChannelLayout_Unknown;
    }
}

ChannelLayout ChannelLayout_FFmpeg2Rohier(AVChannelLayout* channelLayout) {
    if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_MONO)) == 0) {
        return ChannelLayout_Mono;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_STEREO)) == 0) {
        return ChannelLayout_Stereo;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_2POINT1)) == 0) {
        return ChannelLayout_2Point1;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_SURROUND)) == 0) {
        return ChannelLayout_Surround;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_3POINT1)) == 0) {
        return ChannelLayout_3Point1;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_4POINT0)) == 0) {
        return ChannelLayout_4Point0;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_QUAD)) == 0) {
        return ChannelLayout_Quad;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT0)) == 0) {
        return ChannelLayout_5Point0;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT0_BACK)) == 0) {
        return ChannelLayout_5Point0_Back;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1)) == 0) {
        return ChannelLayout_5Point1;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1_BACK)) == 0) {
        return ChannelLayout_5Point1_Back;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT0)) == 0) {
        return ChannelLayout_6Point0;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_3POINT1POINT2)) == 0) {
        return ChannelLayout_3Point1Point2;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT0_FRONT)) == 0) {
        return ChannelLayout_6Point0_Front;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_HEXAGONAL)) == 0) {
        return ChannelLayout_Hexagonal;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1)) == 0) {
        return ChannelLayout_6Point1;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1_BACK)) == 0) {
        return ChannelLayout_6Point1_Back;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1_FRONT)) == 0) {
        return ChannelLayout_6Point1_Front;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT0)) == 0) {
        return ChannelLayout_7Point0;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT0_FRONT)) == 0) {
        return ChannelLayout_7Point0_Front;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1)) == 0) {
        return ChannelLayout_7Point1;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1_WIDE)) == 0) {
        return ChannelLayout_7Point1_Wide;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK)) == 0) {
        return ChannelLayout_7Point1_Wide_Back;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_OCTAGONAL)) == 0) {
        return ChannelLayout_Octagonal;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1POINT2)) == 0) {
        return ChannelLayout_5Point1Point2;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1POINT2)) == 0) {
        return ChannelLayout_7Point1Point2;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_9POINT1POINT6)) == 0) {
        return ChannelLayout_9Point1Point6;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_HEXADECAGONAL)) == 0) {
        return ChannelLayout_Hexadecagonal;
    } else if (av_channel_layout_compare(channelLayout, new AVChannelLayout(AV_CHANNEL_LAYOUT_22POINT2)) == 0) {
        return ChannelLayout_22Point2;
    }
    return ChannelLayout::ChannelLayout_Unknown;
}

OH_AudioChannelLayout ChannelLayout_Rohier2OHCodec(ChannelLayout channelLayout) {
    switch (channelLayout) {
        case ChannelLayout::ChannelLayout_Mono:
            return OH_AudioChannelLayout::CH_LAYOUT_MONO;
        case ChannelLayout::ChannelLayout_Stereo:
            return OH_AudioChannelLayout::CH_LAYOUT_STEREO;
        case ChannelLayout::ChannelLayout_2Point1:
            return OH_AudioChannelLayout::CH_LAYOUT_2POINT1;
        case ChannelLayout::ChannelLayout_Surround:
            return OH_AudioChannelLayout::CH_LAYOUT_SURROUND;
        case ChannelLayout::ChannelLayout_3Point1:
            return OH_AudioChannelLayout::CH_LAYOUT_3POINT1;
        case ChannelLayout::ChannelLayout_4Point0:
            return OH_AudioChannelLayout::CH_LAYOUT_4POINT0;
        case ChannelLayout::ChannelLayout_Quad:
            return OH_AudioChannelLayout::CH_LAYOUT_QUAD;
        case ChannelLayout::ChannelLayout_5Point0:
            return OH_AudioChannelLayout::CH_LAYOUT_5POINT0;
        case ChannelLayout::ChannelLayout_5Point0_Back:
            return OH_AudioChannelLayout::CH_LAYOUT_5POINT0_BACK;
        case ChannelLayout::ChannelLayout_5Point1:
            return OH_AudioChannelLayout::CH_LAYOUT_5POINT1;
        case ChannelLayout::ChannelLayout_5Point1_Back:
            return OH_AudioChannelLayout::CH_LAYOUT_5POINT1_BACK;
        case ChannelLayout::ChannelLayout_6Point0:
            return OH_AudioChannelLayout::CH_LAYOUT_6POINT0;
        case ChannelLayout::ChannelLayout_3Point1Point2:
            return OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2;
        case ChannelLayout::ChannelLayout_6Point0_Front:
            return OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT;
        case ChannelLayout::ChannelLayout_Hexagonal:
            return OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL;
        case ChannelLayout::ChannelLayout_6Point1:
            return OH_AudioChannelLayout::CH_LAYOUT_6POINT1;
        case ChannelLayout::ChannelLayout_6Point1_Back:
            return OH_AudioChannelLayout::CH_LAYOUT_6POINT1_BACK;
        case ChannelLayout::ChannelLayout_6Point1_Front:
            return OH_AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT;
        case ChannelLayout::ChannelLayout_7Point0:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT0;
        case ChannelLayout::ChannelLayout_7Point0_Front:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT;
        case ChannelLayout::ChannelLayout_7Point1:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT1;
        case ChannelLayout::ChannelLayout_7Point1_Wide:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE;
        case ChannelLayout::ChannelLayout_7Point1_Wide_Back:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK;
        case ChannelLayout::ChannelLayout_Octagonal:
            return OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL;
        case ChannelLayout::ChannelLayout_5Point1Point2:
            return OH_AudioChannelLayout::CH_LAYOUT_5POINT1POINT2;
        case ChannelLayout::ChannelLayout_7Point1Point2:
            return OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT2;
        case ChannelLayout::ChannelLayout_9Point1Point6:
            return OH_AudioChannelLayout::CH_LAYOUT_9POINT1POINT6;
        case ChannelLayout::ChannelLayout_Hexadecagonal:
            return OH_AudioChannelLayout::CH_LAYOUT_HEXADECAGONAL;
        case ChannelLayout::ChannelLayout_22Point2:
            return OH_AudioChannelLayout::CH_LAYOUT_22POINT2;
        default:
            return OH_AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    }
}

AVChannelLayout* ChannelLayout_Rohier2FFmpeg(ChannelLayout channelLayout) {
    switch (channelLayout) {
        case ChannelLayout::ChannelLayout_Mono:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_MONO);
        case ChannelLayout::ChannelLayout_Stereo:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_STEREO);
        case ChannelLayout::ChannelLayout_2Point1:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_2POINT1);
        case ChannelLayout::ChannelLayout_Surround:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_SURROUND);
        case ChannelLayout::ChannelLayout_3Point1:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_3POINT1);
        case ChannelLayout::ChannelLayout_4Point0:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_4POINT0);
        case ChannelLayout::ChannelLayout_Quad:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_QUAD);
        case ChannelLayout::ChannelLayout_5Point0:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT0);
        case ChannelLayout::ChannelLayout_5Point0_Back:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT0_BACK);
        case ChannelLayout::ChannelLayout_5Point1:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1);
        case ChannelLayout::ChannelLayout_5Point1_Back:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1_BACK);
        case ChannelLayout::ChannelLayout_6Point0:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT0);
        case ChannelLayout::ChannelLayout_3Point1Point2:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_3POINT1POINT2);
        case ChannelLayout::ChannelLayout_6Point0_Front:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT0_FRONT);
        case ChannelLayout::ChannelLayout_Hexagonal:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_HEXAGONAL);
        case ChannelLayout::ChannelLayout_6Point1:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1);
        case ChannelLayout::ChannelLayout_6Point1_Back:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1_BACK);
        case ChannelLayout::ChannelLayout_6Point1_Front:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_6POINT1_FRONT);
        case ChannelLayout::ChannelLayout_7Point0:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT0);
        case ChannelLayout::ChannelLayout_7Point0_Front:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT0_FRONT);
        case ChannelLayout::ChannelLayout_7Point1:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1);
        case ChannelLayout::ChannelLayout_7Point1_Wide:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1_WIDE);
        case ChannelLayout::ChannelLayout_7Point1_Wide_Back:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK);
        case ChannelLayout::ChannelLayout_Octagonal:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_OCTAGONAL);
        case ChannelLayout::ChannelLayout_5Point1Point2:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_5POINT1POINT2);
        case ChannelLayout::ChannelLayout_7Point1Point2:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_7POINT1POINT2);
        case ChannelLayout::ChannelLayout_9Point1Point6:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_9POINT1POINT6);
        case ChannelLayout::ChannelLayout_Hexadecagonal:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_HEXADECAGONAL);
        case ChannelLayout::ChannelLayout_22Point2:
            return new AVChannelLayout(AV_CHANNEL_LAYOUT_22POINT2);
        default:
            return nullptr; 
    }
}

#endif //ROHIER_TRACK_METADATA_H

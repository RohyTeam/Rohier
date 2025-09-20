//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_BUFFER_H
#define ROHIER_BUFFER_H

#include "rohier/metadata/video_metadata.h"
#include "stdint.h"
#include <cstdint>
#include <multimedia/player_framework/native_avbuffer.h>
#include <multimedia/player_framework/native_avbuffer_info.h>
#include <mutex>
#include <queue>

struct CodecBuffer {
    uint32_t bufferIndex = 0;
    uintptr_t *buffer = nullptr; // for OHCodec is OH_AVBuffer, for FFmpeg is AVFrame or sth?
    uint8_t *bufferAddr = nullptr;
    uint8_t *bufferSize = nullptr;
    OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

    explicit CodecBuffer(uint8_t *addr) : bufferAddr(addr){};
    CodecBuffer(uint8_t *addr, int32_t bufferSize)
        : bufferAddr(addr), attr({0, bufferSize, 0, AVCODEC_BUFFER_FLAGS_NONE}){};
    CodecBuffer(uint32_t argBufferIndex, OH_AVBuffer *argBuffer)
        : bufferIndex(argBufferIndex), buffer(reinterpret_cast<uintptr_t *>(argBuffer))
    {
        OH_AVBuffer_GetBufferAttr(argBuffer, &attr);
    };
};

struct VideoCodecContext {
    OH_AVSource* oh_src;
    AVFormatContext* av_fmt;

    VideoMetadata* metadata = nullptr;

    uint32_t current_track_index;
    double framerate;

    uint32_t inputFrameCount = 0;
    std::mutex inputMutex;
    std::condition_variable inputCond;
    std::queue<CodecBuffer> inputBufferInfoQueue;

    uint32_t outputFrameCount = 0;
    std::mutex outputMutex;
    std::condition_variable outputCond;
    std::mutex renderMutex;
    std::condition_variable renderCond;
    std::queue<CodecBuffer> outputBufferInfoQueue;

    std::queue<unsigned char> renderQueue;

    std::vector<char> cache;
    int32_t remainlen = 0; 

    void clear_cache() {
        cache.clear();
        remainlen = 0;
    }

    void write_cache(void *buffer, int32_t bufferLen) {
        if (bufferLen + remainlen > cache.size()) {
            cache.resize(remainlen + bufferLen);
        }
        std::memcpy(cache.data() + remainlen, buffer, bufferLen);
        remainlen += bufferLen;
    }

    bool read_cache(void *buffer, int32_t bufferLen) {
        if (remainlen < bufferLen) {
            return false;
        }
        std::memcpy(buffer, cache.data(), bufferLen);
        remainlen = remainlen - bufferLen;
        if (remainlen > 0) {
            std::memmove(cache.data(), cache.data() + bufferLen, remainlen);
        }
        return true;
    }
};

struct AudioCodecContext {
    OH_AVSource* oh_src;
    AVFormatContext* av_fmt;

    VideoMetadata* metadata = nullptr;

    uint32_t current_track_index;
    double framerate;

    uint32_t inputFrameCount = 0;
    std::mutex inputMutex;
    std::condition_variable inputCond;
    std::queue<CodecBuffer> inputBufferInfoQueue;

    uint32_t outputFrameCount = 0;
    std::mutex outputMutex;
    std::condition_variable outputCond;
    std::mutex renderMutex;
    std::condition_variable renderCond;
    std::queue<CodecBuffer> outputBufferInfoQueue;

    std::queue<unsigned char> renderQueue;
    std::queue<unsigned char> renderMetadataQueue;

    std::vector<char> cache;
    int32_t remainlen = 0; 

    void clear_cache() {
        cache.clear();
        remainlen = 0;
    }

    void write_cache(void *buffer, int32_t bufferLen) {
        if (bufferLen + remainlen > cache.size()) {
            cache.resize(remainlen + bufferLen);
        }
        std::memcpy(cache.data() + remainlen, buffer, bufferLen);
        remainlen += bufferLen;
    }

    bool read_cache(void *buffer, int32_t bufferLen) {
        if (remainlen < bufferLen) {
            return false;
        }
        std::memcpy(buffer, cache.data(), bufferLen);
        remainlen = remainlen - bufferLen;
        if (remainlen > 0) {
            std::memmove(cache.data(), cache.data() + bufferLen, remainlen);
        }
        return true;
    }
};

#endif //ROHIER_BUFFER_H

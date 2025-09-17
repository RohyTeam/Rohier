//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_VIDEO_DECODER_H
#define ROHIER_VIDEO_DECODER_H

#include "rohier/common/status.h"
#include "rohier/decoder/buffer.h"
#include "rohier/metadata/video_metadata.h"
#include "rohier/native_window/rohier_window_manager.h"
#include <cstdint>
#include <string>

class VideoDecoder {
public:
    virtual ~VideoDecoder() {};
    virtual RohierStatus prepare(RohierNativeWindow* window, CodecContext* context) = 0;
    virtual RohierStatus start() = 0;
    virtual RohierStatus stop() = 0;
    virtual RohierStatus release() = 0;
    virtual RohierStatus push_buffer(CodecBuffer &buffer) = 0;
    virtual RohierStatus free_buffer(uint32_t bufferIndex, bool render) = 0;
    virtual CodecContext* get_context() = 0;
};

#endif //ROHIER_VIDEO_DECODER_H

//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef ROHIER_AUDIO_DECODER_H
#define ROHIER_AUDIO_DECODER_H

#include "rohier/common/status.h"
#include "rohier/decoder/buffer.h"

class AudioDecoder {
public:
    virtual ~AudioDecoder() {};
    virtual RohierStatus prepare(AudioCodecContext* context) = 0;
    virtual RohierStatus start() = 0;
    virtual RohierStatus stop() = 0;
    virtual RohierStatus release() = 0;
    virtual RohierStatus push_buffer(CodecBuffer &buffer) = 0;
    virtual RohierStatus free_buffer(uint32_t bufferIndex) = 0;
};

#endif //ROHIER_AUDIO_DECODER_H

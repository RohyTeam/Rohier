#ifndef ROHIER_OHCODEC_VIDEO_DECODER_H
#define ROHIER_OHCODEC_VIDEO_DECODER_H

#include "rohier/decoder/video/video_decoder.h"
#include <multimedia/player_framework/native_avcapability.h>

class OHCodecVideoDecoder : public VideoDecoder {
    
public:
    OHCodecVideoDecoder(OH_AVCapability* capability);
    ~OHCodecVideoDecoder() override;
    
    RohierStatus prepare(RohierNativeWindow* window, VideoCodecContext* context) override;
    RohierStatus start() override;
    RohierStatus stop() override;
    RohierStatus release() override;
    RohierStatus push_buffer(CodecBuffer &buffer) override;
    RohierStatus free_buffer(uint32_t bufferIndex, bool render) override;
    VideoCodecContext* get_context() override;
    
private:
    OH_AVCodec* codec_;
    VideoCodecContext* context_;
    
};

#endif //ROHIER_OHCODEC_VIDEO_DECODER_H

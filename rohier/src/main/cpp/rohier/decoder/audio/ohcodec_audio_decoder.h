#ifndef ROHIER_OHCODEC_AUDIO_DECODER_H
#define ROHIER_OHCODEC_AUDIO_DECODER_H

#include "rohier/decoder/audio/audio_decoder.h"
#include <multimedia/player_framework/native_avcapability.h>
#include <ohaudio/native_audiostream_base.h>
#include "multimedia/player_framework/native_avcodec_audiocodec.h"
#include "multimedia/player_framework/native_avbuffer_info.h"

class OHCodecAudioDecoder : public AudioDecoder {
public:
    OHCodecAudioDecoder(OH_AVCapability* capability);
    ~OHCodecAudioDecoder() override;
    
    RohierStatus prepare(AudioCodecContext* context) override;
    RohierStatus start() override;
    RohierStatus stop() override;
    RohierStatus release() override;
    RohierStatus push_buffer(CodecBuffer &buffer) override;
    RohierStatus render(CodecBuffer &buffer) override;
    RohierStatus free_buffer(CodecBuffer &buffer) override;
    
private:
    OH_AVCodec* codec_;
    AudioCodecContext* context_;
    
    OH_AudioStreamBuilder* audio_stream_builder_ = nullptr;
    OH_AudioRenderer* audio_renderer_ = nullptr;

};

#endif //ROHIER_OHCODEC_AUDIO_DECODER_H

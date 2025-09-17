#ifndef ROHIER_FFMPEG_VIDEO_DECODER_H
#define ROHIER_FFMPEG_VIDEO_DECODER_H

#include "rohier/decoder/video/video_decoder.h"

extern "C" {
#include "libavformat/avformat.h"
}

/*class FFmpegVideoDecoder : public VideoDecoder {
    
public:
    FFmpegVideoDecoder();
    ~FFmpegVideoDecoder() override;
    RohierStatus prepare(RohierNativeWindow* window, CodecContext* context) override;
    RohierStatus start() override;
    RohierStatus stop() override;
    RohierStatus release() override;
    RohierStatus push_buffer(CodecBuffer &buffer) override;
    RohierStatus free_buffer(uint32_t bufferIndex, bool render) override;
    virtual CodecContext* get_context() override;

};*/

#endif //ROHIER_FFMPEG_VIDEO_DECODER_H

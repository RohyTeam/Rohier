#ifndef ROHIER_FFMPEG_VIDEO_DECODER_H
#define ROHIER_FFMPEG_VIDEO_DECODER_H

#include "rohier/decoder/video/video_decoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

class FFmpegVideoDecoder : public VideoDecoder {
    
public:
    FFmpegVideoDecoder(AVFormatContext* context);
    ~FFmpegVideoDecoder() override;
    RohierStatus prepare(RohierNativeWindow* window, VideoCodecContext* context) override;
    RohierStatus start() override;
    RohierStatus stop() override;
    RohierStatus release() override;
    RohierStatus push_buffer(CodecBuffer &buffer) override;
    RohierStatus free_buffer(uint32_t bufferIndex, bool render) override;
    VideoCodecContext* get_context() override;
private:
    AVFormatContext* format_context_;
    VideoCodecContext* context_;
    const AVCodec* codec_;
    AVCodecContext* codec_context_;

};

#endif //ROHIER_FFMPEG_VIDEO_DECODER_H

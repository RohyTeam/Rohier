//
// Created on 2025/9/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "ffmpeg_video_decoder.h"
#include "libavcodec/avcodec.h"
#include "rohier/utils/rohier_logger.h"

FFmpegVideoDecoder::FFmpegVideoDecoder(AVFormatContext* context) : format_context_(context) {
    if (!context) {
        ROHIER_ERROR("FFmpegVideoDecoder", "Failed to create FFmpeg decoder");
    }
}

FFmpegVideoDecoder::~FFmpegVideoDecoder() {
    this->release();
}

RohierStatus FFmpegVideoDecoder::prepare(RohierNativeWindow* window, VideoCodecContext* context) {
    ROHIER_INFO("FFmpegVideoDecoder", "Preparing FFmpeg video decoder");
    // 先释放之前已经准备好的内容，重新准备
    // this->release();
    // 如果没有窗口则无法渲染视频内容，就不继续准备
    if (!window) {
        ROHIER_ERROR("FFmpegVideoDecoder", "Failed to prepare decoder because window not found");
        return RohierStatus::RohierStatus_WindowNotFound;
    }
    // 如果没有上下文或元数据就无法正常设置视频的长宽，就不继续播放视频
    if (!context) {
        ROHIER_ERROR("FFmpegVideoDecoder", "Failed to prepare decoder because metadata not found");
        return RohierStatus::RohierStatus_IllegalMetadata;
    }
    this->codec_ = avcodec_find_decoder_by_name(context->metadata->tracks[context->current_track_index].codec.c_str());
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Cannot find decoder for target codec name");
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    this->context_ = context;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus FFmpegVideoDecoder::start() {
    this->codec_context_ = avcodec_alloc_context3(this->codec_);
    avcodec_parameters_to_context(this->codec_context_, this->format_context_->streams[this->context_->current_track_index]->codecpar);
    avcodec_open2(this->codec_context_, this->codec_, nullptr);
    return RohierStatus::RohierStatus_Success;
}

RohierStatus FFmpegVideoDecoder::stop() {
    avcodec_free_context(&this->codec_context_);
    return RohierStatus::RohierStatus_Success;
}

RohierStatus FFmpegVideoDecoder::release() {
    return RohierStatus::RohierStatus_Success;
}

RohierStatus FFmpegVideoDecoder::push_buffer(CodecBuffer &buffer) {
    // TODO
    return RohierStatus::RohierStatus_Success;
}

RohierStatus FFmpegVideoDecoder::free_buffer(CodecBuffer &buffer, bool render) {
    
    // TODO
    return RohierStatus::RohierStatus_Success;
}
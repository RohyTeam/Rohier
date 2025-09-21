#include "ohcodec_video_decoder.h"
#include "rohier/utils/rohier_logger.h"
#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <multimedia/player_framework/native_averrors.h>
#include <native_window/external_window.h>

static void ohcodec_video_decoder_callback_on_error(OH_AVCodec *codec, int32_t errorCode, void *userData) {
    VideoCodecContext* context = (VideoCodecContext*) userData;
    // TODO: 返回给 ArkTS 侧处理
}

static void ohcodec_video_decoder_callback_on_stream_changed(OH_AVCodec *codec, OH_AVFormat *format, void *userData) {
    VideoCodecContext* context = (VideoCodecContext*) userData;
    // TODO：给 ArkTS 一个事件即可
}

static void ohcodec_video_decoder_callback_on_need_input_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    if (userData == nullptr)
        return;
    VideoCodecContext* context = (VideoCodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->inputMutex);
    context->inputBufferInfoQueue.emplace(index, buffer);
    context->inputCond.notify_all();
}

static void ohcodec_video_decoder_callback_on_new_output_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    VideoCodecContext* context = (VideoCodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->outputMutex);
    context->outputBufferInfoQueue.emplace(index, buffer);
    context->outputCond.notify_all();
}

OHCodecVideoDecoder::OHCodecVideoDecoder(OH_AVCapability* capability) {
    // 根据解码能力创建解码器
    auto name = OH_AVCapability_GetName(capability);
    ROHIER_INFO("OHCodecVideoDecoder", "Creating %{public}s decoder", name);
    this->codec_ = OH_VideoDecoder_CreateByName(name);
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to create OHCodec decoder");
    }
}

OHCodecVideoDecoder::~OHCodecVideoDecoder() {
    this->release();
}

RohierStatus OHCodecVideoDecoder::prepare(RohierNativeWindow* window, VideoCodecContext* context) {
    ROHIER_INFO("OHCodecVideoDecoder", "Preparing OHCodec video decoder");
    // 先释放之前已经准备好的内容，重新准备
    // this->release();
    // 如果没有窗口则无法渲染视频内容，就不继续准备
    if (!window) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to prepare decoder because window not found");
        return RohierStatus::RohierStatus_WindowNotFound;
    }
    // 如果没有上下文或元数据就无法正常设置视频的长宽，就不继续播放视频
    if (!context) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to prepare decoder because metadata not found");
        return RohierStatus::RohierStatus_IllegalMetadata;
    }
    // 如果没有系统的解码器，说明创建此解码器时出现问题，不继续准备
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Cannot find decoder for target codec name");
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    this->context_ = context;
    // 创建解码器回调
    OH_AVCodecCallback callback = {
        &ohcodec_video_decoder_callback_on_error,
        &ohcodec_video_decoder_callback_on_stream_changed,
        &ohcodec_video_decoder_callback_on_need_input_buffer,
        &ohcodec_video_decoder_callback_on_new_output_buffer
    };
    // 注册回调
    ROHIER_INFO("OHCodecVideoDecoder", "Registering decoder callbacks");
    OH_AVErrCode ret = OH_VideoDecoder_RegisterCallback(this->codec_, callback, context);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to set decoder callbacks with err code %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToSetDecoderCallbacks;
    }
    
    ROHIER_INFO("OHCodecVideoDecoder", "Getting track info");
    // 设置视频长宽，因为我们使用 Surface 模式播放视频，所以 PixelFormat 可以直接设置成 AV_PIXEL_FORMAT_SURFACE_FORMAT
    double frame_rate;
    OH_AVFormat* track_info = context->metadata->get_ohcodec_track(context->oh_src, context->current_track_index);
    OH_AVFormat_GetDoubleValue(track_info, OH_MD_KEY_FRAME_RATE, &frame_rate);
    // TODO: extra data
    OH_AVFormat_Destroy(track_info);
    
    context->framerate = frame_rate;
    
    // 创建 OH_AVFormat 设置解码器参数
    ROHIER_INFO("OHCodecVideoDecoder", "Creating configure parameters");
    auto format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_WIDTH, context->metadata->tracks[context->current_track_index].width);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_HEIGHT, context->metadata->tracks[context->current_track_index].height);
    // OH_AVFormat_SetIntValue(format, OH_MD_KEY_PIXEL_FORMAT, OH_AVPixelFormat::AV_PIXEL_FORMAT_SURFACE_FORMAT);
    
    // 配置解码器
    ROHIER_INFO("OHCodecVideoDecoder", "Configuring decoder");
    ret = OH_VideoDecoder_Configure(this->codec_, format);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to configure decoder with err code %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToConfigureDecoder;
    }
    
    // 配置完解码器后释放配置
    ROHIER_INFO("OHCodecVideoDecoder", "Releasing parameters");
    OH_AVFormat_Destroy(format);
    
    // 设置渲染 Surface，OHCodec 的好处就是解码后可以自动帮我们直接在 Surface 上渲染，如果是 FFmpeg，我们还得手动操作 OpenGL
    ROHIER_INFO("OHCodecVideoDecoder", "Setting decoder surface");
    ret = OH_VideoDecoder_SetSurface(this->codec_, window->nativeWindow);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to set decoder surface with err code %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToSetSurface;
    }
    
    // 设置画面填充模式，为了方便，我们直接让画面铺满窗口，比例问题交给 ArkTS 的 UI 侧解决
    ROHIER_INFO("OHCodecVideoDecoder", "Setting native window scaling mode");
    if (OH_NativeWindow_NativeWindowSetScalingModeV2(window->nativeWindow, OHScalingModeV2::OH_SCALING_MODE_SCALE_TO_WINDOW_V2) != 0) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to native window scaling mode");
        return RohierStatus::RohierStatus_FailedToSetNativeWindowScalingMode;
    }
    
    // 准备解码器
    ROHIER_INFO("OHCodecVideoDecoder", "Preparing decoder");
    ret = OH_VideoDecoder_Prepare(this->codec_);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to prepare video decoder");
        return RohierStatus::RohierStatus_FailedToPrepareDecoder;
    }
    
    ROHIER_INFO("OHCodecVideoDecoder", "Successfully prepared OHCodec video decoder");
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::start() {
    ROHIER_INFO("OHCodecVideoDecoder", "Starting decoder");
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to start decoder because decoder not found");
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    
    OH_AVErrCode ret = OH_VideoDecoder_Start(this->codec_);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecVideoDecoder", "Failed to start decoder with err code: %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToStartDecoder;
    }
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::stop() {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    if (OH_VideoDecoder_Stop(this->codec_) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToStopDecoder;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::push_buffer(CodecBuffer &buffer) {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    if (OH_VideoDecoder_PushInputBuffer(this->codec_, buffer.buffer_index) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToPushBufferToDecoder;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::free_buffer(CodecBuffer &buffer, bool render) {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    OH_AVErrCode ret;
    if (render) {
        ret = OH_VideoDecoder_RenderOutputBuffer(this->codec_, buffer.buffer_index);
    } else {
        ret = OH_VideoDecoder_FreeOutputBuffer(this->codec_, buffer.buffer_index);
    }
    if (ret != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToFreeBufferFromDecoder;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::release() {
    // 释放系统解码器
    if (this->codec_) {
        OH_VideoDecoder_Flush(this->codec_);
        OH_VideoDecoder_Stop(this->codec_);
        OH_VideoDecoder_Destroy(this->codec_);
    }
    return RohierStatus::RohierStatus_Success;
}

VideoCodecContext* OHCodecVideoDecoder::get_context() {
    return this->context_;
}
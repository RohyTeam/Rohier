#include "ohcodec_video_decoder.h"
#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <multimedia/player_framework/native_averrors.h>
#include <native_window/external_window.h>

static void callback_on_error(OH_AVCodec *codec, int32_t errorCode, void *userData) {
    CodecContext* context = (CodecContext*) userData;
    // TODO: 返回给 ArkTS 侧处理
}

static void callback_on_stream_changed(OH_AVCodec *codec, OH_AVFormat *format, void *userData) {
    CodecContext* context = (CodecContext*) userData;
    // TODO：给 ArkTS 一个事件即可
}

static void callback_on_need_input_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    if (userData == nullptr)
        return;
    CodecContext* context = (CodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->inputMutex);
    context->inputBufferInfoQueue.emplace(index, buffer);
    context->inputCond.notify_all();
}

static void callback_on_new_output_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    CodecContext* context = (CodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->outputMutex);
    context->outputBufferInfoQueue.emplace(index, buffer);
    context->outputCond.notify_all();
}

OHCodecVideoDecoder::OHCodecVideoDecoder(OH_AVCapability* capability) {
    // 根据解码能力创建解码器
    this->codec_ = OH_VideoDecoder_CreateByMime(OH_AVCapability_GetName(capability));
}

OHCodecVideoDecoder::~OHCodecVideoDecoder() {
    this->release();
}

RohierStatus OHCodecVideoDecoder::prepare(RohierNativeWindow* window, CodecContext* context) {
    // 先释放之前已经准备好的内容，重新准备
    this->release();
    // 如果没有窗口则无法渲染视频内容，就不继续准备
    if (!window || !window->nativeWindow)
        return RohierStatus::RohierStatus_WindowNotFound;
    // 如果没有上下文或元数据就无法正常设置视频的长宽，就不继续播放视频
    if (!context || !context->metadata)
        return RohierStatus::RohierStatus_IllegalMetadata;
    // 如果没有系统的解码器，说明创建此解码器时出现问题，不继续准备
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    this->context_ = context;
    // 创建解码器回调
    OH_AVCodecCallback callback = {
        &callback_on_error,
        &callback_on_stream_changed,
        &callback_on_need_input_buffer,
        &callback_on_new_output_buffer
    };
    // 注册回调
    OH_VideoDecoder_RegisterCallback(this->codec_, callback, context);
    // 创建 OH_AVFormat 设置解码器参数
    auto format = std::shared_ptr<OH_AVFormat>(OH_AVFormat_Create(), OH_AVFormat_Destroy);
    // 设置视频长宽，因为我们使用 Surface 模式播放视频，所以 PixelFormat 可以直接设置成 AV_PIXEL_FORMAT_SURFACE_FORMAT
    OH_AVFormat_SetIntValue(format.get(), OH_MD_KEY_WIDTH, context->metadata->width);
    OH_AVFormat_SetIntValue(format.get(), OH_MD_KEY_HEIGHT, context->metadata->height);
    OH_AVFormat_SetIntValue(format.get(), OH_MD_KEY_PIXEL_FORMAT, OH_AVPixelFormat::AV_PIXEL_FORMAT_SURFACE_FORMAT);
    
    // 配置解码器
    if (OH_VideoDecoder_Configure(this->codec_, format.get()) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToConfigureDecoder;
    
    // 配置完解码器后释放配置
    format.reset();
    
    // 设置渲染 Surface，OHCodec 的好处就是解码后可以自动帮我们直接在 Surface 上渲染，如果是 FFmpeg，我们还得手动操作 OpenGL
    if (OH_VideoDecoder_SetSurface(this->codec_, window->nativeWindow) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToSetSurface;
    
    // 设置画面填充模式，为了方便，我们直接让画面铺满窗口，比例问题交给 ArkTS 的 UI 侧解决
    OH_NativeWindow_NativeWindowSetScalingModeV2(window->nativeWindow, OHScalingModeV2::OH_SCALING_MODE_SCALE_TO_WINDOW_V2);
    
    // 准备解码器
    if (OH_VideoDecoder_Prepare(this->codec_) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToPrepareDecoder;
    
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::start() {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    if (OH_VideoDecoder_Start(this->codec_) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToStartDecoder;
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
    if (OH_VideoDecoder_PushInputBuffer(this->codec_, buffer.bufferIndex) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToPushBufferToDecoder;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecVideoDecoder::free_buffer(uint32_t bufferIndex, bool render) {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    OH_AVErrCode ret;
    if (render) {
        ret = OH_VideoDecoder_RenderOutputBuffer(this->codec_, bufferIndex);
    } else {
        ret = OH_VideoDecoder_FreeOutputBuffer(this->codec_, bufferIndex);
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

CodecContext* OHCodecVideoDecoder::get_context() {
    return this->context_;
}
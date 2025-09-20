#include "ohcodec_audio_decoder.h"
#include "rohier/utils/rohier_logger.h"
#include <cstdint>
#include <ohaudio/native_audiostreambuilder.h>

static void ohcodec_audio_decoder_callback_on_error(OH_AVCodec *codec, int32_t errorCode, void *userData) {
    AudioCodecContext* context = (AudioCodecContext*) userData;
    // TODO: 返回给 ArkTS 侧处理
}

static void ohcodec_audio_decoder_callback_on_stream_changed(OH_AVCodec *codec, OH_AVFormat *format, void *userData) {
    AudioCodecContext* context = (AudioCodecContext*) userData;
    // TODO：给 ArkTS 一个事件即可
}

static void ohcodec_audio_decoder_callback_on_need_input_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    if (userData == nullptr)
        return;
    AudioCodecContext* context = (AudioCodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->inputMutex);
    context->inputBufferInfoQueue.emplace(index, buffer);
    context->inputCond.notify_all();
}

static void ohcodec_audio_decoder_callback_on_new_output_buffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    AudioCodecContext* context = (AudioCodecContext*) userData;
    std::unique_lock<std::mutex> lock(context->outputMutex);
    context->outputBufferInfoQueue.emplace(index, buffer);
    context->outputCond.notify_all();
}

static int32_t ohcodec_audio_decoder_callback_on_render_write_data(OH_AudioRenderer *renderer, void *userData, void *buffer, int32_t length) {
    AudioCodecContext *context = static_cast<AudioCodecContext*>(userData);

    uint8_t *dest = (uint8_t*) buffer;
    size_t index = 0;
    std::unique_lock<std::mutex> lock(context->outputMutex);
    while (!context->renderQueue.empty() && index < length) {
        dest[index++] = context->renderQueue.front();
        context->renderQueue.pop();
    }
    if (context->renderQueue.size() < length) {
        context->renderCond.notify_all();
    }
    return 0;
}

static int32_t ohcodec_audio_decoder_callback_write_data_with_metadata(OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize, void* metadata, int32_t metadataSize) {
    AudioCodecContext *context = static_cast<AudioCodecContext*>(userData);

    uint8_t *audioDest = (uint8_t*) audioData;
    uint8_t *metadataDest = (uint8_t*) metadata;
    size_t index = 0;
    std::unique_lock<std::mutex> lock(context->outputMutex);
    while (!context->renderQueue.empty() && index < audioDataSize) {
        audioDest[index++] = context->renderQueue.front();
        context->renderQueue.pop();
    }
    while (!context->renderMetadataQueue.empty() && index < metadataSize) {
        metadataDest[index++] = context->renderMetadataQueue.front();
        context->renderMetadataQueue.pop();
    }
    if (context->renderQueue.size() < audioDataSize && context->renderMetadataQueue.size() < metadataSize) {
        context->renderCond.notify_all();
    }
    
    return 0;
}

int32_t ohcodec_audio_decoder_callback_on_render_stream_event(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Event event) {
    return 0;
}

int32_t ohcodec_audio_decoder_callback_on_render_interrupt_event(OH_AudioRenderer *renderer, void *userData, OH_AudioInterrupt_ForceType type, OH_AudioInterrupt_Hint hint) {
    return 0;
}

int32_t ohcodec_audio_decoder_callback_on_render_error(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error) {
    return 0;
}

OHCodecAudioDecoder::OHCodecAudioDecoder(OH_AVCapability* capability) {
    auto name = OH_AVCapability_GetName(capability);
    ROHIER_INFO("OHCodecAudioDecoder", "Creating %{public}s decoder", name);
    this->codec_ = OH_AudioCodec_CreateByName(name);
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to create OHCodec decoder");
    }
}

OHCodecAudioDecoder::~OHCodecAudioDecoder() { 
    this->release();
}

RohierStatus OHCodecAudioDecoder::prepare(AudioCodecContext* context) {
    ROHIER_INFO("OHCodecAudioDecoder", "Preparing OHCodec audio decoder");
    // 先释放之前已经准备好的内容，重新准备
    // this->release();
    // 如果没有上下文或元数据就无法正常设置视频的长宽，就不继续播放视频
    if (!context) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to prepare decoder because metadata not found");
        return RohierStatus::RohierStatus_IllegalMetadata;
    }
    // 如果没有系统的解码器，说明创建此解码器时出现问题，不继续准备
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Cannot find decoder for target codec name");
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    this->context_ = context;
    // 创建解码器回调
    OH_AVCodecCallback callback = {
        &ohcodec_audio_decoder_callback_on_error,
        &ohcodec_audio_decoder_callback_on_stream_changed,
        &ohcodec_audio_decoder_callback_on_need_input_buffer,
        &ohcodec_audio_decoder_callback_on_new_output_buffer
    };
    
    ROHIER_INFO("OHCodecAudioDecoder", "Registering decoder callbacks");
    OH_AVErrCode ret = OH_AudioCodec_RegisterCallback(this->codec_, callback, context);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to set decoder callbacks with err code %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToSetDecoderCallbacks;
    }
    
    ROHIER_INFO("OHCodecAudioDecoder", "Getting track info with track index %{public}d", context->current_track_index);
    int32_t sample_format, channel_count, sample_rate;
    int64_t channel_layout, bitrate;
    uint8_t* addr;
    size_t addr_size;
    auto track_info = context->metadata->get_ohcodec_track(context->oh_src, context->current_track_index);
    if (!track_info) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to get track info");
    }
    OH_AVFormat_GetIntValue(track_info, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, &sample_format);
    OH_AVFormat_GetIntValue(track_info, OH_MD_KEY_AUD_CHANNEL_COUNT, &channel_count);
    OH_AVFormat_GetIntValue(track_info, OH_MD_KEY_AUD_SAMPLE_RATE, &sample_rate);
    OH_AVFormat_GetLongValue(track_info, OH_MD_KEY_CHANNEL_LAYOUT, &channel_layout);
    if (context->metadata->tracks[context->current_track_index].codec == "av3a") {
        OH_AVFormat_GetLongValue(track_info, OH_MD_KEY_BITRATE, &bitrate);
        OH_AVFormat_GetBuffer(track_info, OH_MD_KEY_CODEC_CONFIG, &addr, &addr_size);
    }
    OH_AVFormat_Destroy(track_info);
    ROHIER_INFO("OHCodecAudioDecoder", 
        "sample format %{public}d\n"
        "channel count %{public}d\n"
        "sample rate %{public}d\n"
        "channel layout %{public}ld\n",
        sample_format, channel_count, sample_rate, channel_layout
    );
    
    // 创建 OH_AVFormat 设置解码器参数
    ROHIER_INFO("OHCodecAudioDecoder", "Creating configure parameters");
    auto format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, sample_format);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT, channel_count);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE, sample_rate);
    OH_AVFormat_SetLongValue(format, OH_MD_KEY_CHANNEL_LAYOUT, channel_layout);
    if (context->metadata->tracks[context->current_track_index].codec == "av3a") {
        OH_AVFormat_SetLongValue(format, OH_MD_KEY_BITRATE, bitrate);
        OH_AVFormat_SetBuffer(format, OH_MD_KEY_CODEC_CONFIG, addr, addr_size);
    }
    
    // 配置解码器
    ROHIER_INFO("OHCodecAudioDecoder", "Configuring decoder");
    ret = OH_AudioCodec_Configure(this->codec_, format);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to configure decoder with err code %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToConfigureDecoder;
    }
    
    // 配置完解码器后释放配置
    ROHIER_INFO("OHCodecAudioDecoder", "Releasing parameters");
    OH_AVFormat_Destroy(format);
    
    // 准备解码器
    ROHIER_INFO("OHCodecAudioDecoder", "Preparing decoder");
    ret = OH_AudioCodec_Prepare(this->codec_);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to prepare audio decoder");
        return RohierStatus::RohierStatus_FailedToPrepareDecoder;
    }
    
    OH_AudioStreamBuilder_Create(&this->audio_stream_builder_, OH_AudioStream_Type::AUDIOSTREAM_TYPE_RENDERER);
    OH_AudioStreamBuilder_SetLatencyMode(this->audio_stream_builder_, OH_AudioStream_LatencyMode::AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioStreamBuilder_SetSamplingRate(this->audio_stream_builder_, sample_rate);
    OH_AudioStreamBuilder_SetChannelCount(this->audio_stream_builder_, channel_count);
    OH_AudioStreamBuilder_SetChannelLayout(this->audio_stream_builder_, (OH_AudioChannelLayout) channel_layout);
    if (0 <= sample_format && sample_format <= 4) {
        OH_AudioStreamBuilder_SetSampleFormat(this->audio_stream_builder_, (OH_AudioStream_SampleFormat) sample_format);        
    }
    OH_AudioStreamBuilder_SetEncodingType(
        this->audio_stream_builder_,
        context->metadata->tracks[context->current_track_index].codec == "av3a" ? 
            OH_AudioStream_EncodingType::AUDIOSTREAM_ENCODING_TYPE_AUDIOVIVID : 
            OH_AudioStream_EncodingType::AUDIOSTREAM_ENCODING_TYPE_RAW
    );
    OH_AudioStreamBuilder_SetRendererInfo(audio_stream_builder_, OH_AudioStream_Usage::AUDIOSTREAM_USAGE_MOVIE);
    
    OH_AudioRenderer_Callbacks callbacks;
    if (context->metadata->tracks[context->current_track_index].codec == "av3a") {
        callbacks.OH_AudioRenderer_OnWriteData = nullptr;        
    } else {
        callbacks.OH_AudioRenderer_OnWriteData = ohcodec_audio_decoder_callback_on_render_write_data;
    }
    callbacks.OH_AudioRenderer_OnStreamEvent = ohcodec_audio_decoder_callback_on_render_stream_event;
    callbacks.OH_AudioRenderer_OnInterruptEvent = ohcodec_audio_decoder_callback_on_render_interrupt_event;
    callbacks.OH_AudioRenderer_OnError = ohcodec_audio_decoder_callback_on_render_error;
    
    OH_AudioStream_Result renderer_ret = OH_AudioStreamBuilder_SetRendererCallback(this->audio_stream_builder_, callbacks, context);
    if (renderer_ret != OH_AudioStream_Result::AUDIOSTREAM_SUCCESS) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to set renderer callback with err code %{public}d", renderer_ret);
    }
    
    if (context->metadata->tracks[context->current_track_index].codec == "av3a") {
        OH_AudioRenderer_WriteDataWithMetadataCallback callback = ohcodec_audio_decoder_callback_write_data_with_metadata;
        renderer_ret = OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback(this->audio_stream_builder_, callback, context);
        if (renderer_ret != OH_AudioStream_Result::AUDIOSTREAM_SUCCESS) {
            ROHIER_ERROR("OHCodecAudioDecoder", "Failed to set audio data with metadata writer callback with err code %{public}d", renderer_ret);
        }
    }
    
    renderer_ret = OH_AudioStreamBuilder_GenerateRenderer(this->audio_stream_builder_, &this->audio_renderer_);
    if (renderer_ret != OH_AudioStream_Result::AUDIOSTREAM_SUCCESS) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to create audio renderer with err code %{public}d", renderer_ret);
    }
    
    ROHIER_INFO("OHCodecAudioDecoder", "Successfully prepared OHCodec audio decoder");
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::start() {
    ROHIER_INFO("OHCodecAudioDecoder", "Starting decoder");
    if (!this->codec_) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to start decoder because decoder not found");
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    
    OH_AVErrCode ret = OH_AudioCodec_Start(this->codec_);
    if (ret != OH_AVErrCode::AV_ERR_OK) {
        ROHIER_ERROR("OHCodecAudioDecoder", "Failed to start decoder with err code: %{public}d", ret);
        return RohierStatus::RohierStatus_FailedToStartDecoder;
    }
    OH_AudioRenderer_Start(this->audio_renderer_);
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::stop() {
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::release() {
    if (this->audio_renderer_) {
        OH_AudioRenderer_Release(this->audio_renderer_);
    }
    if (this->audio_stream_builder_) {
        OH_AudioStreamBuilder_Destroy(this->audio_stream_builder_);
    }
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::push_buffer(CodecBuffer &buffer) {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    OH_AVBuffer_SetBufferAttr(reinterpret_cast<OH_AVBuffer *>(buffer.buffer), &buffer.attr);
    if (OH_AudioCodec_PushInputBuffer(this->codec_, buffer.bufferIndex) != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToPushBufferToDecoder;
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::render(CodecBuffer &buffer) {
    uint8_t* source = OH_AVBuffer_GetAddr(reinterpret_cast<OH_AVBuffer *>(buffer.buffer));
    OH_AVFormat* format = OH_AVBuffer_GetParameter(reinterpret_cast<OH_AVBuffer *>(buffer.buffer));
    uint8_t* metadata;
    size_t metadata_size;
    OH_AVFormat_GetBuffer(format, OH_MD_KEY_AUDIO_VIVID_METADATA, &metadata, &metadata_size); 
    for (int i = 0; i < buffer.attr.size; i++) {
        this->context_->renderQueue.push(*(source + i));
    } 
    for (int i = 0; i < metadata_size; i++) {
        this->context_->renderMetadataQueue.push(*(metadata + i));
    }
    return RohierStatus::RohierStatus_Success;
}

RohierStatus OHCodecAudioDecoder::free_buffer(CodecBuffer &buffer) {
    if (!this->codec_)
        return RohierStatus::RohierStatus_DecoderNotFound;
    OH_AVErrCode ret = OH_AudioCodec_FreeOutputBuffer(this->codec_, buffer.bufferIndex);
    if (ret != OH_AVErrCode::AV_ERR_OK)
        return RohierStatus::RohierStatus_FailedToFreeBufferFromDecoder;
    return RohierStatus::RohierStatus_Success;
}
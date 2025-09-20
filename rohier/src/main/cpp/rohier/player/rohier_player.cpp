#include "rohier/player/rohier_player.h"
#include "rohier/decoder/audio/ohcodec_audio_decoder.h"
#include "rohier/decoder/video/ffmpeg_video_decoder.h"
#include "rohier/decoder/video/ohcodec_video_decoder.h"
#include "rohier/demuxer/ohcodec_demuxer.h"
#include "rohier/utils/rohier_logger.h"
#include <unistd.h>

using namespace std::chrono_literals;

std::shared_ptr<VideoMetadata> fetch_metadata(AVFormatContext* fmt_ctx, OH_AVSource* oh_src) {
    ROHIER_INFO("RohierPlayer", "Fetching metadata");
    std::shared_ptr<VideoMetadata> meta = std::make_shared<VideoMetadata>();
    try {
        if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
            ROHIER_ERROR("RohierPlayer", "Cannot find stream info with FFmpeg");
            return nullptr;
        }
        
        if (fmt_ctx->duration != AV_NOPTS_VALUE) {
            meta->duration = fmt_ctx->duration * 1000 / AV_TIME_BASE;
        }
        meta->bitrate = fmt_ctx->bit_rate;
        
        int video_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (video_stream_idx < 0) {
            ROHIER_ERROR("RohierPlayer", "Cannot find video track");
            return nullptr;
        }
        
        if (fmt_ctx->nb_chapters > 0) {
            meta->chapters.reserve(fmt_ctx->nb_chapters);
            
            for (int i = 0; i < fmt_ctx->nb_chapters; i++) {
                AVChapter* chapter = fmt_ctx->chapters[i];
                ChapterMetadata cm;
                
                cm.start = time_base_to_ms(chapter->time_base, chapter->start);
                cm.end = time_base_to_ms(chapter->time_base, chapter->end);
                
                AVDictionaryEntry* tag = av_dict_get(chapter->metadata, "title", nullptr, 0);
                if (tag) 
                    cm.title = tag->value;
                
                meta->chapters.push_back(std::move(cm));
            }
        }
        
        meta->tracks.reserve(fmt_ctx->nb_streams);
        
        for (int i = 0; i < fmt_ctx->nb_streams; i++) {
            AVStream* stream = fmt_ctx->streams[i];
            
            OH_AVFormat* oh_avFormat;
            
            oh_avFormat = OH_AVSource_GetTrackFormat(oh_src, i);
            
            if (i == video_stream_idx) {
                AVCodecParameters* codecpar = stream->codecpar;
                
                meta->width = codecpar->width;
                meta->height = codecpar->height;
                
                meta->averageFrameRate = av_q2d(stream->avg_frame_rate);
            }
            
            TrackMetadata track;
            
            track.index = i;
            if (stream->duration != AV_NOPTS_VALUE) {
                track.duration = time_base_to_ms(stream->time_base, stream->duration);
            }
            
            AVDictionaryEntry* tag = av_dict_get(stream->metadata, "title", nullptr, 0);
            if (tag) track.title = tag->value;
            
            tag = av_dict_get(stream->metadata, "language", nullptr, 0);
            if (tag) track.language = tag->value;
            
            const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (stream->codecpar->codec_id == AV_CODEC_ID_AVS3DA) {
                // TODO: this is a dirty hack
                track.codec = "av3a";
                track.codec_full = "Audio Vivid";
            } else if (codec) {
                if (i == video_stream_idx) {    
                    meta->codec = codec->name ? codec->name : "";
                    meta->codec_full = codec->long_name ? codec->long_name : "";
                }
                track.codec = codec->name ? codec->name : "";
                track.codec_full = codec->long_name ? codec->long_name : "";
            }
            
            switch (stream->codecpar->codec_type) {
                case AVMEDIA_TYPE_VIDEO:
                    track.track_type = TrackType::TrackType_Video;
                    track.width = stream->codecpar->width;
                    track.height = stream->codecpar->height;
                    track.bitrate = stream->codecpar->bit_rate;
                
                    {
                        int32_t isHDRVivid = 0;
                        OH_AVFormat_GetIntValue(oh_avFormat, OH_MD_KEY_VIDEO_IS_HDR_VIVID, &isHDRVivid); 
                        if (stream->codecpar->coded_side_data) {
                            if (stream->codecpar->coded_side_data->type == AV_PKT_DATA_DOVI_CONF) {
                                if (i == video_stream_idx) {
                                    meta->hdr = HdrType::HdrType_Dolby_Vision;
                                }
                                track.hdr = HdrType::HdrType_Dolby_Vision;
                            } else if (stream->codecpar->coded_side_data->type == AV_PKT_DATA_DYNAMIC_HDR10_PLUS) {
                                if (i == video_stream_idx) {
                                    meta->hdr = HdrType::HdrType_HDR10_Plus;
                                }
                                track.hdr = HdrType::HdrType_HDR10_Plus;
                            } else if (isHDRVivid) {
                                if (i == video_stream_idx) {
                                    meta->hdr = HdrType::HdrType_HDRVivid;
                                }
                                track.hdr = HdrType::HdrType_HDRVivid;
                            } else if (stream->codecpar->coded_side_data->type == AV_PKT_DATA_MASTERING_DISPLAY_METADATA
                                    || stream->codecpar->coded_side_data->type == AV_PKT_DATA_CONTENT_LIGHT_LEVEL
                                    || stream->codecpar->color_range == AVCOL_RANGE_JPEG 
                                    || stream->codecpar->color_trc == AVCOL_TRC_SMPTE2084 
                                    || stream->codecpar->color_primaries == AVCOL_PRI_BT2020)   {
                                if (i == video_stream_idx) {
                                    meta->hdr = HdrType::HdrType_HDR10_or_HLG;
                                }
                                track.hdr = HdrType::HdrType_HDR10_or_HLG;
                            } else {
                                if (i == video_stream_idx) {
                                    meta->hdr = HdrType::HdrType_None;
                                }
                                track.hdr = HdrType::HdrType_None;
                            }
                        } else if (isHDRVivid) {
                            if (i == video_stream_idx) {
                                meta->hdr = HdrType::HdrType_HDRVivid;
                            }
                            track.hdr = HdrType::HdrType_HDRVivid;
                        }
                    }
                    break;
                    
                case AVMEDIA_TYPE_AUDIO:
                    track.track_type = TrackType::TrackType_Audio;
                    track.sample_rate = stream->codecpar->sample_rate;
                    track.bitrate = stream->codecpar->bit_rate;
                    track.channels = stream->codecpar->ch_layout.nb_channels;
                    break;
                    
                case AVMEDIA_TYPE_SUBTITLE:
                    track.track_type = TrackType::TrackType_Subtitle;
                    break;
                    
                case AVMEDIA_TYPE_ATTACHMENT:
                    track.track_type = TrackType::TrackType_Attachment;
                    tag = av_dict_get(stream->metadata, "filename", nullptr, 0);
                    if (tag) track.filename = HdrType::HdrType_HDRVivid;
                    
                    tag = av_dict_get(stream->metadata, "mimetype", nullptr, 0);
                    if (tag) track.mimetype = HdrType::HdrType_HDRVivid;
                    break;
                    
                default:
                    track.track_type = TrackType::TrackType_Unknown;
                    break;
            }
        
            OH_AVFormat_Destroy(oh_avFormat);
            meta->tracks.push_back(std::move(track));
        }
    } catch (const std::exception& e) {
        ROHIER_ERROR("RohierPlayer", "Error occurs when fetching metadata");
        return nullptr;
    } catch (...) {
        ROHIER_ERROR("RohierPlayer", "Error occurs when fetching metadata");
        return nullptr;
    }
    
    return meta;
}

RohierPlayer::RohierPlayer() {
}

RohierPlayer::~RohierPlayer() {
    this->release();
}

RohierStatus RohierPlayer::init(RohierNativeWindow* window) {
    ROHIER_INFO("RohierPlayer", "Initializing player");
    if (!window || !window->nativeWindow || !window->nativeXComponent) {
        ROHIER_ERROR("RohierPlayer", "Window not found");
        return RohierStatus::RohierStatus_WindowNotFound;
    }
    ROHIER_INFO("RohierPlayer", "Window is valid");
    this->window_ = window;
    ROHIER_INFO("RohierPlayer", "Window stored");
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::prepare(VideoSource* source) {
    ROHIER_INFO("RohierPlayer", "Preparing player");
    // 释放之前准备好的内容
    // this->release();
    
    // 初始化 FFmpeg 访问网络
    avformat_network_init();
    
    OH_AVSource* oh_src;
    
    AVFormatContext* fmt_ctx = nullptr;
    AVDictionary* options = nullptr;
    
    if (source->isUrl) {
        // 如果源是网络文件，则配置部分 FFmpeg 的网络设置，例如超时时间
        av_dict_set(&options, "rw_timeout", "5000000", 0);
        av_dict_set(&options, "reconnect", "1", 0);
        av_dict_set(&options, "reconnect_at_eof", "1", 0);
        av_dict_set(&options, "reconnect_streamed", "1", 0);
        
        // ohos 媒体解析直接通过 uri 创建
        oh_src = OH_AVSource_CreateWithURI(const_cast<char*>(source->path.c_str()));
    } else {
        // ohos 媒体解析本地文件时需要手动开启文件
        int fd = open(source->path.c_str(), O_RDONLY);
        struct stat fileStatus {};
        size_t fileSize = 0;
        // 判断文件是否打开成功，打开成功后创建访问，打开失败则停止
        if (stat(source->path.c_str(), &fileStatus) == 0) {
            this->fd = &fd;
            fileSize = static_cast<size_t>(fileStatus.st_size);
            oh_src = OH_AVSource_CreateWithFD(fd, 0, fileSize);
        } else {
            close(fd);
            ROHIER_ERROR("RohierPlayer", "Cannot open source file with fs");
            return RohierStatus::RohierStatus_SourceNotAccessible;
        }
    }
    
    // 如果系统的媒体解析无法打开源文件则不继续
    if (!oh_src) {
        ROHIER_ERROR("RohierPlayer", "Cannot access source file with OHCodec");
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
        
    // 如果 FFmpeg 的媒体解析无法打开源文件则不继续
    if (avformat_open_input(&fmt_ctx, source->path.c_str(), nullptr, &options) < 0) {
        ROHIER_ERROR("RohierPlayer", "Cannot access source file with FFmpeg");
        if (options) {
            av_dict_free(&options);
        }
        // 同时需要释放之前打开的文件和系统解析
        OH_AVSource_Destroy(oh_src);
        if (this->fd) {
            close(*this->fd);
            this->fd = nullptr;
        }
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
    
    if (options) {
        av_dict_free(&options);
    }
    
    this->av_format_context_ptr = std::shared_ptr<AVFormatContext>(fmt_ctx, AVFormatContextReleaser());
    this->oh_avsource_context_ptr = std::shared_ptr<OH_AVSource>(oh_src, OH_AVSourceReleaser());
    
    std::shared_ptr<VideoMetadata> video_metadata = fetch_metadata(fmt_ctx, oh_src);
    
    if (!video_metadata) {
        ROHIER_ERROR("RohierPlayer", "Fetched metadata is null");
        avformat_free_context(fmt_ctx);
        OH_AVSource_Destroy(oh_src);
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
    
    this->video_metadata_ = video_metadata;
    
    this->video_context_ = new VideoCodecContext;
    this->video_context_->metadata = video_metadata.get();
    this->video_context_->oh_src = oh_src;
    this->video_context_->av_fmt = fmt_ctx;
    
    this->audio_context_ = new AudioCodecContext;
    this->audio_context_->metadata = video_metadata.get();
    this->audio_context_->oh_src = oh_src;
    this->audio_context_->av_fmt = fmt_ctx;
    
    this->demuxer_ = std::make_shared<OHCodecDemuxer>();
    if (this->demuxer_->prepare(this->av_format_context_ptr.get(), this->oh_avsource_context_ptr.get(), *video_metadata) != RohierStatus::RohierStatus_Success) {
        ROHIER_INFO("RohierPlayer", "Failed to prepare demuxer");
        return RohierStatus::RohierStatus_FailedToPrepareDemuxer;
    }
    ROHIER_INFO("RohierPlayer", "Demuxer prepared");
    auto best_video_track = video_metadata->find_best_video_track();
    auto best_audio_track = video_metadata->find_best_audio_track();
    if (best_video_track) {
        ROHIER_INFO("RohierPlayer", "Best video track is %{public}d", best_video_track->index);
        this->video_context_->current_track_index = best_video_track->index;
    } else {
        ROHIER_ERROR("RohierPlayer", "Cannot find best video track");
    }
    if (best_audio_track) {
        ROHIER_INFO("RohierPlayer", "Best audio track is %{public}d", best_audio_track->index);
        this->audio_context_->current_track_index = best_audio_track->index;
    } else {
        ROHIER_ERROR("RohierPlayer", "Cannot find best audio track");
    }
    
    auto video_codec = this->video_metadata_->tracks[this->video_context_->current_track_index].codec;
    auto audio_codec = this->video_metadata_->tracks[this->audio_context_->current_track_index].codec;
    
    ROHIER_INFO("RohierPlayer", "Choosing video decoder");
    if (video_codec == "h264" || video_codec == "avc") {
        ROHIER_INFO("RohierPlayer", "Detected codec is H.264, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_AVC, false, HARDWARE);
        // TODO: 设置摧毁器
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else if (video_codec == "h265" || video_codec == "hevc") {
        ROHIER_INFO("RohierPlayer", "Detected codec is H.265, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_HEVC, false, HARDWARE);
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else if (video_codec == "h266" || video_codec == "vvc") {
        ROHIER_INFO("RohierPlayer", "Detected codec is H.266, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_VVC, false, HARDWARE);
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else {
        ROHIER_INFO("RohierPlayer", "Detected codec is not supported by OHCodec, using FFmpeg");
        // this->video_decoder_ = std::make_shared<FFmpegVideoDecoder>();
    }
    
    ROHIER_INFO("RohierPlayer", "Choosing video decoder");
    if (audio_codec == "av3a") {
        ROHIER_INFO("RohierPlayer", "Detected codec is AV3A, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_VIVID, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    } else if (audio_codec == "aac") {
        ROHIER_INFO("RohierPlayer", "Detected codec is AAC, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_AAC, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    } else if (audio_codec == "flac") {
        ROHIER_INFO("RohierPlayer", "Detected codec is FLAC, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_FLAC, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    } else if (audio_codec == "mpeg" || audio_codec == "mp3") {
        ROHIER_INFO("RohierPlayer", "Detected codec is MPEG, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_MPEG, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    } else if (audio_codec == "vorbis") {
        ROHIER_INFO("RohierPlayer", "Detected codec is Vorbis, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_VORBIS, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    } else if (audio_codec == "opus") {
        ROHIER_INFO("RohierPlayer", "Detected codec is OPUS, using OHCodec");
        OH_AVCapability *capability= OH_AVCodec_GetCapability(OH_AVCODEC_MIMETYPE_AUDIO_OPUS, false);
        this->audio_decoder_ = std::make_shared<OHCodecAudioDecoder>(capability);
    }
    
    if (this->video_decoder_->prepare(this->window_, this->video_context_) != RohierStatus::RohierStatus_Success) {
        ROHIER_INFO("RohierPlayer", "Failed to prepare video decoder");
        return RohierStatus::RohierStatus_FailedToPrepareDecoder;
    }
    if (this->audio_decoder_->prepare(this->audio_context_) != RohierStatus::RohierStatus_Success) {
        ROHIER_INFO("RohierPlayer", "Failed to prepare audio decoder");
        return RohierStatus::RohierStatus_FailedToPrepareDecoder;
    }
    ROHIER_INFO("RohierPlayer", "Decoder prepared");
    
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::release() {
    ROHIER_INFO("RohierPlayer", "Releasing player");
    this->window_ = nullptr;
    // 释放线程
    if (this->video_decode_input_thread_ && this->video_decode_input_thread_->joinable()) {
        this->video_decode_input_thread_->detach();
        this->video_decode_input_thread_.reset();
    }
    if (this->video_decode_output_thread_ && this->video_decode_output_thread_->joinable()) {
        this->video_decode_output_thread_->detach();
        this->video_decode_output_thread_.reset();
    }
    if (this->audio_decode_input_thread_ && this->audio_decode_input_thread_->joinable()) {
        this->audio_decode_input_thread_->detach();
        this->audio_decode_input_thread_.reset();
    }
    if (this->audio_decode_output_thread_ && this->audio_decode_output_thread_->joinable()) {
        this->audio_decode_output_thread_->detach();
        this->audio_decode_output_thread_.reset();
    }
    if (this->video_context_) {
        delete this->video_context_;
        this->video_context_ = nullptr;
    }
    if (this->audio_context_) {
        delete this->audio_context_;
        this->audio_context_ = nullptr;
    }
    // 如果是本地文件，同时需要关闭本地文件的 fd
    if (this->fd) {
        close(*this->fd);
        this->fd = nullptr;
    }
    // 释放视频元数据
    if (this->video_metadata_) {
        this->video_metadata_.reset();
        this->video_metadata_ = nullptr;
    }
    // 释放 FFmpeg 媒体源
    if (this->av_format_context_ptr) {
        this->av_format_context_ptr.reset();
    }
    // 释放 ohos 媒体源
    if (this->oh_avsource_context_ptr) {
        this->oh_avsource_context_ptr.reset();
    }
    // 释放视频解码器
    if (this->video_decoder_) {
        this->video_decoder_.reset();
    }
    // 释放音频解码器
    if (this->audio_decoder_) {
        this->audio_decoder_.reset();
    }
    // 释放解封装器
    if (this->demuxer_) {
        this->demuxer_.reset();
    }
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::start() {
    ROHIER_INFO("RohierPlayer", "Starting player");
    std::unique_lock<std::mutex> lock(this->mutex_);
    if (this->started) {
        ROHIER_ERROR("RohierPlayer", "Player already started");
        return RohierStatus::RohierStatus_AlreadyStarted;
    }
    if (!this->video_decoder_) {
        ROHIER_ERROR("RohierPlayer", "Video decoder not found");
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    if (this->video_decoder_->start() != RohierStatus::RohierStatus_Success) {
        ROHIER_ERROR("RohierPlayer", "Failed to start video decoder");
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_FailedToStartDecoder;
    }
    if (!this->audio_decoder_) {
        ROHIER_ERROR("RohierPlayer", "Audio decoder not found");
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    if (this->audio_decoder_->start() != RohierStatus::RohierStatus_Success) {
        ROHIER_ERROR("RohierPlayer", "Failed to start audio decoder");
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_FailedToStartDecoder;
    }
    this->started = true;
    this->video_decode_input_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_video_decode_input, this);
    this->video_decode_output_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_video_decode_output, this);
    this->audio_decode_input_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_audio_decode_input, this);
    this->audio_decode_output_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_audio_decode_output, this);
    if (this->video_decode_input_thread_ == nullptr || this->video_decode_output_thread_ == nullptr 
        || this->audio_decode_input_thread_ == nullptr || this->audio_decode_output_thread_ == nullptr) {
        ROHIER_ERROR("RohierPlayer", "Failed to start decoder thread");
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_FailedToCreateThread;
    }
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::stop() {
    return RohierStatus::RohierStatus_Success;
}

std::shared_ptr<VideoMetadata> RohierPlayer::get_metadata() {
    return this->video_metadata_;
}

AVFormatContext* RohierPlayer::get_ffmpeg_avformat_context() {
    return this->av_format_context_ptr.get();
}

OH_AVSource* RohierPlayer::get_ohcodec_avsource() {
    return this->oh_avsource_context_ptr.get();
}

void RohierPlayer::thread_video_decode_input() {
    while (true) {
        if (!this->started)
            break;
        std::unique_lock<std::mutex> lock(this->video_context_->inputMutex);
        this->video_context_->inputCond.wait_for(lock, 5s, [this]() { return !this->started || !this->video_context_->inputBufferInfoQueue.empty(); });
        if (!this->started)
            break;
        if (this->video_context_->inputBufferInfoQueue.empty())
            continue;
        
        CodecBuffer buffer = this->video_context_->inputBufferInfoQueue.front();
        this->video_context_->inputBufferInfoQueue.pop();
        this->video_context_->inputFrameCount++;
        lock.unlock();
        
        this->demuxer_->read_sample(this->video_context_->current_track_index, reinterpret_cast<OH_AVBuffer*>(buffer.buffer), buffer.attr);
        
        if (this->video_decoder_->push_buffer(buffer) != RohierStatus::RohierStatus_Success) 
            break;
        
        if (buffer.attr.flags & AVCODEC_BUFFER_FLAGS_EOS)
            break;
    }
}

void RohierPlayer::thread_video_decode_output() {
    this->frameInterval = MICROSECOND / this->video_context_->framerate;
    while (true) {
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        if (!this->started)
            break;
        std::unique_lock<std::mutex> lock(this->video_context_->outputMutex);
        this->video_context_->outputCond.wait_for(lock, 5s, [this]() { return !this->started || !this->video_context_->outputBufferInfoQueue.empty(); });
        if (!this->started)
            break;
        if (this->video_context_->outputBufferInfoQueue.empty())
            continue;
        CodecBuffer buffer = this->video_context_->outputBufferInfoQueue.front();
        this->video_context_->outputBufferInfoQueue.pop();
        if (buffer.attr.flags & AVCODEC_BUFFER_FLAGS_EOS)
            break;
        this->video_context_->outputFrameCount++;
        
        lock.unlock();
        
        if (this->video_decoder_->free_buffer(buffer.bufferIndex, true) != RohierStatus::RohierStatus_Success)
            break;

        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(this->frameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
}

void RohierPlayer::thread_audio_decode_input() {
    while (true) {
        if (!this->started)
            break;
        std::unique_lock<std::mutex> lock(this->audio_context_->inputMutex);
        this->audio_context_->inputCond.wait_for(lock, 5s, [this]() { return !this->started || !this->audio_context_->inputBufferInfoQueue.empty(); });
        if (!this->started)
            break;
        if (this->audio_context_->inputBufferInfoQueue.empty())
            continue;

        CodecBuffer buffer = this->audio_context_->inputBufferInfoQueue.front();
        this->audio_context_->inputBufferInfoQueue.pop();
        this->audio_context_->inputFrameCount++;
        lock.unlock();

        demuxer_->read_sample(this->audio_context_->current_track_index, reinterpret_cast<OH_AVBuffer *>(buffer.buffer), buffer.attr);

        if (this->audio_decoder_->push_buffer(buffer) != RohierStatus::RohierStatus_Success) 
            break;
        if (buffer.attr.flags & AVCODEC_BUFFER_FLAGS_EOS)
            break;
    }
}

void RohierPlayer::thread_audio_decode_output() {
    while (true) {
        if (!this->started)
            break;
        std::unique_lock<std::mutex> lock(this->audio_context_->outputMutex);
        this->audio_context_->outputCond.wait_for(lock, 5s, [this]() { return !this->started || !this->audio_context_->outputBufferInfoQueue.empty(); });
        if (!this->started)
            break;
        if (this->audio_context_->outputBufferInfoQueue.empty())
            continue;

        CodecBuffer buffer = this->audio_context_->outputBufferInfoQueue.front();
        this->audio_context_->outputBufferInfoQueue.pop();
        if (buffer.attr.flags & AVCODEC_BUFFER_FLAGS_EOS)
            break;
        this->audio_context_->outputFrameCount++;
        uint8_t* source = OH_AVBuffer_GetAddr(reinterpret_cast<OH_AVBuffer *>(buffer.buffer));
        OH_AVFormat* format = OH_AVBuffer_GetParameter(reinterpret_cast<OH_AVBuffer *>(buffer.buffer));
        uint8_t* metadata;
        size_t metadata_size;
        OH_AVFormat_GetBuffer(format, OH_MD_KEY_AUDIO_VIVID_METADATA, &metadata, &metadata_size); 
        for (int i = 0; i < buffer.attr.size; i++) {
            this->audio_context_->renderQueue.push(*(source + i));
        } 
        for (int i = 0; i < metadata_size; i++) {
            this->audio_context_->renderMetadataQueue.push(*(metadata + i));
        }
        lock.unlock();

        if (this->audio_decoder_->free_buffer(buffer.bufferIndex) != RohierStatus::RohierStatus_Success)
            break;

        std::unique_lock<std::mutex> lockRender(this->audio_context_->renderMutex);
        this->audio_context_->renderCond.wait_for(lockRender, 20ms, [this, buffer]() {
            return this->audio_context_->renderQueue.size() < BALANCE_VALUE * buffer.attr.size;
        });
    }
    std::unique_lock<std::mutex> lockRender(this->audio_context_->renderMutex);
    this->audio_context_->renderCond.wait_for(lockRender, 500ms, [this](){
        return this->audio_context_->renderQueue.size() < 1;
    });
}


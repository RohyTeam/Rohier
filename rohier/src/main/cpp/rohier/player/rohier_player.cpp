#include "rohier/player/rohier_player.h"
#include "rohier/decoder/video/ffmpeg_video_decoder.h"
#include "rohier/decoder/video/ohcodec_video_decoder.h"
#include "rohier/demuxer/ohcodec_demuxer.h"
#include <unistd.h>

using namespace std::chrono_literals;

std::shared_ptr<VideoMetadata> fetch_metadata(AVFormatContext* fmt_ctx, OH_AVSource* oh_src) {
    std::shared_ptr<VideoMetadata> meta = std::make_shared<VideoMetadata>();
    try {
        if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
            return nullptr;
        }
        
        if (fmt_ctx->duration != AV_NOPTS_VALUE) {
            meta->duration = fmt_ctx->duration * 1000 / AV_TIME_BASE;
        }
        meta->bitrate = fmt_ctx->bit_rate;
        
        int video_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (video_stream_idx < 0) {
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
                if (tag) cm.title = tag->value;
                
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
                    track.pixel_format = (AVPixelFormat) stream->codecpar->format;
                    track.averageFrameRate = av_q2d(stream->avg_frame_rate);
                
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
                    track.sample_format = (AVSampleFormat) stream->codecpar->format;
                    track.bitrate = stream->codecpar->bit_rate;
                    track.channels = stream->codecpar->ch_layout.nb_channels;
                    track.channel_layout = ChannelLayout_FFmpeg2Rohier(&stream->codecpar->ch_layout);
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
        return nullptr;
    } catch (...) {
        return nullptr;
    }
    
    return meta;
}

RohierPlayer::RohierPlayer() {
}

RohierPlayer::~RohierPlayer() {
}

RohierStatus RohierPlayer::init(RohierNativeWindow* window) {
    if (this->window_) {
        return RohierStatus::RohierStatus_AlreadyInitialized;
    }
    if (!window || !window->nativeWindow || !window->nativeXComponent) {
        return RohierStatus::RohierStatus_WindowNotFound;
    }
    this->window_ = window;
    
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::prepare(VideoSource* source) {
    // 如果没有设置窗口则表明没有初始化
    if (!this->window_) {
        return RohierStatus::RohierStatus_NotInitialized;
    }
    // 释放之前准备好的内容
    this->release();
    
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
            return RohierStatus::RohierStatus_SourceNotAccessible;
        }
    }
    
    // 如果系统的媒体解析无法打开源文件则不继续
    if (!oh_src) {
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
        
    // 如果 FFmpeg 的媒体解析无法打开源文件则不继续
    if (avformat_open_input(&fmt_ctx, source->path.c_str(), nullptr, &options) < 0) {
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
        avformat_free_context(fmt_ctx);
        OH_AVSource_Destroy(oh_src);
        return RohierStatus::RohierStatus_SourceNotAccessible;
    }
    
    this->video_metadata_ = video_metadata;
    
    this->video_context_ = new CodecContext;
    this->video_context_->metadata = video_metadata.get();
    
    auto videoCodec = this->video_metadata_->codec;
    
    this->demuxer_ = std::make_shared<OHCodecDemuxer>();
    
    if (videoCodec == "h264" || videoCodec == "avc") {
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_AVC, false, HARDWARE);
        // TODO: 设置摧毁器
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else if (videoCodec == "h265" || videoCodec == "hevc") {
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_HEVC, false, HARDWARE);
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else if (videoCodec == "h266" || videoCodec == "vvc") {
        OH_AVCapability *capability= OH_AVCodec_GetCapabilityByCategory(OH_AVCODEC_MIMETYPE_VIDEO_VVC, false, HARDWARE);
        this->video_decoder_ = std::make_shared<OHCodecVideoDecoder>(capability);
    } else {
        // this->video_decoder_ = std::make_shared<FFmpegVideoDecoder>();
    }
    
    return RohierStatus::RohierStatus_Success;
}

RohierStatus RohierPlayer::release() {
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
    if (this->audio_decoder_input_thread_ && this->audio_decoder_input_thread_->joinable()) {
        this->audio_decoder_input_thread_->detach();
        this->audio_decoder_input_thread_.reset();
    }
    if (this->audio_decoder_output_thread_ && this->audio_decoder_output_thread_->joinable()) {
        this->audio_decoder_output_thread_->detach();
        this->audio_decoder_output_thread_.reset();
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
    std::unique_lock<std::mutex> lock(this->mutex_);
    if (this->started) {
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_AlreadyStarted;
    }
    if (!this->video_decoder_) {
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_DecoderNotFound;
    }
    if (this->video_decoder_->start() != RohierStatus::RohierStatus_Success) {
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_FailedToStartDecoder;
    }
    this->started = true;
    this->video_decode_input_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_video_decode_input, this);
    this->video_decode_output_thread_ = std::make_unique<std::thread>(&RohierPlayer::thread_video_decode_output, this);
    if (this->video_decode_input_thread_ == nullptr || this->video_decode_output_thread_ == nullptr) {
        lock.unlock();
        this->release();
        return RohierStatus::RohierStatus_FailedToCreateThread;
    }
    // TODO: audio decode
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
        
        this->demuxer_->read_sample(this->demuxer_->get_video_track_id(), reinterpret_cast<OH_AVBuffer*>(buffer.buffer), buffer.attr);
        
        if (this->video_decoder_->push_buffer(buffer) != RohierStatus::RohierStatus_Success) 
            break;
        
        if (buffer.attr.flags & AVCODEC_BUFFER_FLAGS_EOS)
            break;
    }
}

void RohierPlayer::thread_video_decode_output() {
    this->frameInterval = MICROSECOND / this->video_metadata_->averageFrameRate;
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
    
}

void RohierPlayer::thread_audio_decode_output() {
    
}


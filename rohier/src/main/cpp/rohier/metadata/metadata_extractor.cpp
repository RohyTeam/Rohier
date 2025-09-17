//
// Created on 2025/9/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "rohier/metadata/metadata_extractor.h"
#include "rohier/utils/pointer_management.h"

#include <cstdint>
#include <multimedia/player_framework/native_avdemuxer.h>
#include <multimedia/player_framework/native_avsource.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <multimedia/player_framework/native_avformat.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <fcntl.h>
#include <sys/stat.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

int find_cover_track(AVFormatContext* fmt_ctx) {
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            (stream->codecpar->codec_id == AV_CODEC_ID_MJPEG ||
            stream->codecpar->codec_id == AV_CODEC_ID_PNG ||
            stream->codecpar->codec_id == AV_CODEC_ID_BMP)) {
            return i;
        }
    }
    
    return -1;
}

    
std::vector<uint8_t> encode_frame_to_jpeg(const AVFrame* frame) {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!codec) {
        throw std::runtime_error("JPEG codec not found");
    }
    
    std::unique_ptr<AVCodecContext, AVCodecContextReleaser> codec_ctx(avcodec_alloc_context3(codec));
    if (!codec_ctx) {
        throw std::runtime_error("Could not allocate video codec context");
    }
    
    codec_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    codec_ctx->width = frame->width;
    codec_ctx->height = frame->height;
    codec_ctx->time_base = {1, 25};
    codec_ctx->color_range = AVCOL_RANGE_JPEG;
    codec_ctx->bit_rate = 400000;
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 0;
    codec_ctx->flags |= AV_CODEC_FLAG_QSCALE;
    codec_ctx->global_quality = 3;
    
    if (avcodec_open2(codec_ctx.get(), codec, nullptr) < 0) {
        throw std::runtime_error("Could not open JPEG codec");
    }
    
    std::unique_ptr<AVPacket, AVPacketReleaser> pkt(av_packet_alloc());
    if (!pkt) {
        throw std::runtime_error("Could not allocate packet");
    }
    
    if (avcodec_send_frame(codec_ctx.get(), frame) < 0) {
        throw std::runtime_error("Error sending frame to encoder");
    }
    
    int ret = avcodec_receive_packet(codec_ctx.get(), pkt.get());
    if (ret < 0) {
        throw std::runtime_error("Error encoding frame");
    }
    
    return {pkt->data, pkt->data + pkt->size};
}

std::vector<uint8_t> extract_cover_image(AVFormatContext* fmt_ctx, int video_stream_index) {
    AVStream* stream = fmt_ctx->streams[video_stream_index];
    AVCodecParameters* codec_params = stream->codecpar;
    
    const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        throw std::runtime_error("Unsupported codec for cover extraction");
    }
    
    std::unique_ptr<AVCodecContext, AVCodecContextReleaser> codec_ctx(avcodec_alloc_context3(codec));
    if (!codec_ctx) {
        throw std::runtime_error("Could not allocate codec context for cover extraction");
    }
    
    if (avcodec_parameters_to_context(codec_ctx.get(), codec_params) < 0) {
        throw std::runtime_error("Could not copy codec parameters for cover extraction");
    }
    
    if (avcodec_open2(codec_ctx.get(), codec, nullptr) < 0) {
        throw std::runtime_error("Could not open decoder for cover extraction");
    }
    
    std::unique_ptr<AVFrame, AVFrameReleaser> frame(av_frame_alloc());
    if (!frame) {
        throw std::runtime_error("Memory allocation failed for cover extraction");
    }
    
    std::unique_ptr<AVPacket, AVPacketReleaser> pkt(av_packet_alloc());
    if (!pkt) {
        throw std::runtime_error("Memory allocation failed for cover extraction");
    }
    
    if (av_seek_frame(fmt_ctx, video_stream_index, 0, AVSEEK_FLAG_FRAME) < 0) {
        throw std::runtime_error("Could not seek to first frame for cover extraction");
    }
    
    while (av_read_frame(fmt_ctx, pkt.get()) >= 0) {
        if (pkt->stream_index != video_stream_index) {
            av_packet_unref(pkt.get());
            continue;
        }
        
        if (avcodec_send_packet(codec_ctx.get(), pkt.get()) < 0) {
            av_packet_unref(pkt.get());
            continue;
        }
        
        if (avcodec_receive_frame(codec_ctx.get(), frame.get()) >= 0) {
            if (codec_params->codec_id == AV_CODEC_ID_MJPEG) {
                return {pkt->data, pkt->data + pkt->size};
            }
            
            std::unique_ptr<SwsContext, SwsContextReleaser> sws_ctx(sws_getContext(
                frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
                frame->width, frame->height, AV_PIX_FMT_YUVJ420P,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            ));
            
            if (!sws_ctx) {
                throw std::runtime_error("Could not create scale context for cover extraction");
            }
            
            std::unique_ptr<AVFrame, AVFrameReleaser> yuv_frame(av_frame_alloc());
            if (!yuv_frame) {
                throw std::runtime_error("Memory allocation failed for cover extraction");
            }
            
            int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, 
                                                      frame->width, frame->height, 1);
            std::vector<uint8_t> yuv_buffer(buffer_size);
            
            if (av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize,
                                    yuv_buffer.data(), AV_PIX_FMT_YUVJ420P,
                                    frame->width, frame->height, 1) < 0) {
                throw std::runtime_error("Failed to fill image arrays for cover extraction");
            }
            
            yuv_frame->width = frame->width;
            yuv_frame->height = frame->height;
            yuv_frame->format = AV_PIX_FMT_YUVJ420P;
            
            sws_scale(sws_ctx.get(),
                     frame->data,
                     frame->linesize,
                     0,
                     frame->height,
                     yuv_frame->data,
                     yuv_frame->linesize);
            
            return encode_frame_to_jpeg(yuv_frame.get());
        }
        
        av_packet_unref(pkt.get());
    }
    
    throw std::runtime_error("No frame found for cover extraction");
}

std::shared_ptr<VideoMetadata> MetadataExtractor::extractMetadata(VideoSource* source, bool cover) {
    std::shared_ptr<VideoMetadata> meta = std::make_shared<VideoMetadata>();
    try {
        avformat_network_init();
        
        OH_AVSource* oh_avSource;
        
        AVFormatContext* fmt_ctx = nullptr;
        AVDictionary* options = nullptr;
        
        if (source->isUrl) {
            av_dict_set(&options, "rw_timeout", "5000000", 0);
            av_dict_set(&options, "reconnect", "1", 0);
            av_dict_set(&options, "reconnect_at_eof", "1", 0);
            av_dict_set(&options, "reconnect_streamed", "1", 0);
            oh_avSource = OH_AVSource_CreateWithURI(const_cast<char*>(source->path.c_str()));
        } else {
            int fd = open(source->path.c_str(), O_RDONLY);
            struct stat fileStatus {};
            size_t fileSize = 0;
            if (stat(source->path.c_str(), &fileStatus) == 0) {
                fileSize = static_cast<size_t>(fileStatus.st_size);
                oh_avSource = OH_AVSource_CreateWithFD(fd, 0, fileSize);
            }
        }
        
        if (avformat_open_input(&fmt_ctx, source->path.c_str(), nullptr, &options) < 0) {
            return nullptr;
        }
        auto input_ctx_deleter = [&options](AVFormatContext* ctx) { 
            avformat_close_input(&ctx); 
            av_dict_free(&options);
        };
        std::unique_ptr<AVFormatContext, decltype(input_ctx_deleter)> fmt_ctx_ptr(fmt_ctx, input_ctx_deleter);
        
        if (avformat_find_stream_info(fmt_ctx_ptr.get(), nullptr) < 0) {
            return nullptr;
        }
        
        if (fmt_ctx_ptr->duration != AV_NOPTS_VALUE) {
            meta->duration = fmt_ctx_ptr->duration * 1000 / AV_TIME_BASE;
        }
        meta->bitrate = fmt_ctx_ptr->bit_rate;
        
        int video_stream_idx = av_find_best_stream(fmt_ctx_ptr.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (video_stream_idx < 0) {
            return nullptr;
        }
        
        if (fmt_ctx_ptr->nb_chapters > 0) {
            meta->chapters.reserve(fmt_ctx_ptr->nb_chapters);
            
            for (int i = 0; i < fmt_ctx_ptr->nb_chapters; i++) {
                AVChapter* chapter = fmt_ctx_ptr->chapters[i];
                ChapterMetadata cm;
                
                cm.start = time_base_to_ms(chapter->time_base, chapter->start);
                cm.end = time_base_to_ms(chapter->time_base, chapter->end);
                
                AVDictionaryEntry* tag = av_dict_get(chapter->metadata, "title", nullptr, 0);
                if (tag) cm.title = tag->value;
                
                meta->chapters.push_back(std::move(cm));
            }
        }
        
        meta->tracks.reserve(fmt_ctx_ptr->nb_streams);
        
        int cover_stream_index = -1;
        
        if (cover) {
            cover_stream_index = find_cover_track(fmt_ctx_ptr.get());
        }
        
        for (int i = 0; i < fmt_ctx_ptr->nb_streams; i++) {
            AVStream* stream = fmt_ctx_ptr->streams[i];
            
            OH_AVFormat* oh_avFormat;
            
            if (oh_avSource) {
                oh_avFormat = OH_AVSource_GetTrackFormat(oh_avSource, i);
            }
            
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
                        if (oh_avFormat) {
                            OH_AVFormat_GetIntValue(oh_avFormat, OH_MD_KEY_VIDEO_IS_HDR_VIVID, &isHDRVivid);                    
                        }
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
        
            std::unique_ptr<AVPacket, AVPacketReleaser> pkt(av_packet_alloc());
            
            if (cover) {
                if (av_read_frame(fmt_ctx_ptr.get(), pkt.get()) >= 0) {
                    if (cover_stream_index >= 0 && pkt->stream_index == cover_stream_index) {
                        meta->cover.assign(pkt->data, pkt->data + pkt->size);
                    }
                }
            }
            
            if (oh_avFormat) {
                OH_AVFormat_Destroy(oh_avFormat);
            }
            
            meta->tracks.push_back(std::move(track));
        }
        
        if (cover && meta->cover.empty()) {
            int video_stream_index = av_find_best_stream(
                fmt_ctx_ptr.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
                
            if (video_stream_index >= 0) {
                meta->cover = extract_cover_image(fmt_ctx_ptr.get(), video_stream_index);
            }
        }
        
        if (oh_avSource) {
            OH_AVSource_Destroy(oh_avSource);
        }
    } catch (const std::exception& e) {
        return nullptr;
    } catch (...) {
        return nullptr;
    }
    
    return meta;
}